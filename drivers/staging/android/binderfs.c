// SPDX-License-Identifier: GPL-2.0
/*
 * binderfs.c - binder filesystem for dynamic binder device provisioning.
 *
 * Backported additively onto the 3.10 legacy binder (drivers/staging/android).
 * Adapted from the Android Common Kernel 5.15 binderfs to the 3.10 VFS:
 *   - old .mount/mount_nodev API (no fs_context/fs_parser)
 *   - atomic_t instead of refcount_t
 *   - CURRENT_TIME, ida_simple_*, i_mutex, ->d_inode
 *   - dropped s_user_ns/s_iflags, binder_features, binder_logs (need 5.x binder)
 * Enough for Android init to `mount binder /dev/binderfs` and get
 * binder/hwbinder/vndbinder + binder-control.
 */
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fsnotify.h>
#include <linux/gfp.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/namei.h>
#include <linux/magic.h>
#include <linux/major.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/mount.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <uapi/linux/android/binderfs.h>

#include "binder_internal.h"

#define FIRST_INODE 1
#define SECOND_INODE 2
#define INODE_OFFSET 3
#define BINDERFS_MAX_MINOR (1U << MINORBITS)

static dev_t binderfs_dev;
static DEFINE_MUTEX(binderfs_minors_mutex);
static DEFINE_IDA(binderfs_minors);

struct binderfs_info {
	struct dentry *control_dentry;
	kuid_t root_uid;
	kgid_t root_gid;
	int device_count;
};

static inline struct binderfs_info *BINDERFS_SB(const struct super_block *sb)
{
	return sb->s_fs_info;
}

bool is_binderfs_device(const struct inode *inode)
{
	return inode->i_sb->s_magic == BINDERFS_SUPER_MAGIC;
}

static int binderfs_binder_device_create(struct inode *ref_inode,
					 struct binderfs_device __user *userp,
					 struct binderfs_device *req)
{
	int minor, ret;
	struct dentry *dentry, *root;
	struct binder_device *device;
	char *name = NULL;
	size_t name_len;
	struct inode *inode = NULL;
	struct super_block *sb = ref_inode->i_sb;
	struct binderfs_info *info = sb->s_fs_info;

	mutex_lock(&binderfs_minors_mutex);
	minor = ida_simple_get(&binderfs_minors, 0, BINDERFS_MAX_MINOR, GFP_KERNEL);
	mutex_unlock(&binderfs_minors_mutex);
	if (minor < 0)
		return minor;

	ret = -ENOMEM;
	device = kzalloc(sizeof(*device), GFP_KERNEL);
	if (!device)
		goto err;

	inode = new_inode(sb);
	if (!inode)
		goto err;

	inode->i_ino = minor + INODE_OFFSET;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	init_special_inode(inode, S_IFCHR | 0600,
			   MKDEV(MAJOR(binderfs_dev), minor));
	inode->i_fop = &binder_fops;
	inode->i_uid = info->root_uid;
	inode->i_gid = info->root_gid;

	req->name[BINDERFS_MAX_NAME] = '\0';
	name_len = strlen(req->name);
	name = kmemdup(req->name, name_len + 1, GFP_KERNEL);
	if (!name)
		goto err;

	atomic_set(&device->ref, 1);
	device->binderfs_inode = inode;
	device->context.binder_context_mgr_uid = INVALID_UID;
	device->context.name = name;
	device->miscdev.name = name;
	device->miscdev.minor = minor;

	req->major = MAJOR(binderfs_dev);
	req->minor = minor;

	if (userp && copy_to_user(userp, req, sizeof(*req))) {
		ret = -EFAULT;
		goto err;
	}

	root = sb->s_root;
	mutex_lock(&root->d_inode->i_mutex);
	dentry = lookup_one_len(name, root, name_len);
	if (IS_ERR(dentry)) {
		mutex_unlock(&root->d_inode->i_mutex);
		ret = PTR_ERR(dentry);
		goto err;
	}
	if (dentry->d_inode) {
		dput(dentry);
		mutex_unlock(&root->d_inode->i_mutex);
		ret = -EEXIST;
		goto err;
	}

	inode->i_private = device;
	d_instantiate(dentry, inode);
	fsnotify_create(root->d_inode, dentry);
	mutex_unlock(&root->d_inode->i_mutex);
	return 0;

err:
	kfree(name);
	kfree(device);
	mutex_lock(&binderfs_minors_mutex);
	ida_simple_remove(&binderfs_minors, minor);
	mutex_unlock(&binderfs_minors_mutex);
	iput(inode);
	return ret;
}

static long binder_ctl_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	int ret = -EINVAL;
	struct inode *inode = file->f_path.dentry->d_inode;
	struct binderfs_device __user *device = (struct binderfs_device __user *)arg;
	struct binderfs_device device_req;

	switch (cmd) {
	case BINDER_CTL_ADD:
		ret = copy_from_user(&device_req, device, sizeof(device_req));
		if (ret) {
			ret = -EFAULT;
			break;
		}
		ret = binderfs_binder_device_create(inode, device, &device_req);
		break;
	default:
		break;
	}
	return ret;
}

static void binderfs_evict_inode(struct inode *inode)
{
	struct binder_device *device = inode->i_private;

	clear_inode(inode);
	if (!S_ISCHR(inode->i_mode) || !device)
		return;

	mutex_lock(&binderfs_minors_mutex);
	ida_simple_remove(&binderfs_minors, device->miscdev.minor);
	mutex_unlock(&binderfs_minors_mutex);

	if (atomic_dec_and_test(&device->ref)) {
		kfree(device->context.name);
		kfree(device);
	}
}

static void binderfs_put_super(struct super_block *sb)
{
	struct binderfs_info *info = sb->s_fs_info;

	kfree(info);
	sb->s_fs_info = NULL;
}

static const struct super_operations binderfs_super_ops = {
	.evict_inode	= binderfs_evict_inode,
	.statfs		= simple_statfs,
	.put_super	= binderfs_put_super,
};

static inline bool is_binderfs_control_device(const struct dentry *dentry)
{
	struct binderfs_info *info = dentry->d_sb->s_fs_info;

	return info->control_dentry == dentry;
}

static int binderfs_rename(struct inode *old_dir, struct dentry *old_dentry,
			   struct inode *new_dir, struct dentry *new_dentry)
{
	if (is_binderfs_control_device(old_dentry) ||
	    is_binderfs_control_device(new_dentry))
		return -EPERM;

	return simple_rename(old_dir, old_dentry, new_dir, new_dentry);
}

static int binderfs_unlink(struct inode *dir, struct dentry *dentry)
{
	if (is_binderfs_control_device(dentry))
		return -EPERM;

	return simple_unlink(dir, dentry);
}

static const struct file_operations binder_ctl_fops = {
	.owner		= THIS_MODULE,
	.open		= nonseekable_open,
	.unlocked_ioctl	= binder_ctl_ioctl,
	.compat_ioctl	= binder_ctl_ioctl,
	.llseek		= noop_llseek,
};

static int binderfs_binder_ctl_create(struct super_block *sb)
{
	int minor, ret;
	struct dentry *dentry;
	struct binder_device *device;
	struct inode *inode = NULL;
	struct dentry *root = sb->s_root;
	struct binderfs_info *info = sb->s_fs_info;

	device = kzalloc(sizeof(*device), GFP_KERNEL);
	if (!device)
		return -ENOMEM;

	if (info->control_dentry) {
		ret = 0;
		goto out;
	}

	ret = -ENOMEM;
	inode = new_inode(sb);
	if (!inode)
		goto out;

	mutex_lock(&binderfs_minors_mutex);
	minor = ida_simple_get(&binderfs_minors, 0, BINDERFS_MAX_MINOR, GFP_KERNEL);
	mutex_unlock(&binderfs_minors_mutex);
	if (minor < 0) {
		ret = minor;
		goto out;
	}

	inode->i_ino = SECOND_INODE;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	init_special_inode(inode, S_IFCHR | 0600,
			   MKDEV(MAJOR(binderfs_dev), minor));
	inode->i_fop = &binder_ctl_fops;
	inode->i_uid = info->root_uid;
	inode->i_gid = info->root_gid;

	atomic_set(&device->ref, 1);
	device->binderfs_inode = inode;
	device->miscdev.minor = minor;

	dentry = d_alloc_name(root, "binder-control");
	if (!dentry)
		goto out;

	inode->i_private = device;
	info->control_dentry = dentry;
	d_add(dentry, inode);
	return 0;

out:
	kfree(device);
	iput(inode);
	return ret;
}

static const struct inode_operations binderfs_dir_inode_operations = {
	.lookup	= simple_lookup,
	.rename	= binderfs_rename,
	.unlink	= binderfs_unlink,
};

static int binderfs_fill_super(struct super_block *sb, void *data, int silent)
{
	int ret;
	struct binderfs_info *info;
	struct inode *inode = NULL;
	struct binderfs_device device_info;
	const char *name;
	size_t len;

	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = BINDERFS_SUPER_MAGIC;
	sb->s_op = &binderfs_super_ops;
	sb->s_time_gran = 1;

	info = kzalloc(sizeof(struct binderfs_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	sb->s_fs_info = info;

	info->root_gid = GLOBAL_ROOT_GID;
	info->root_uid = GLOBAL_ROOT_UID;

	inode = new_inode(sb);
	if (!inode)
		return -ENOMEM;

	inode->i_ino = FIRST_INODE;
	inode->i_fop = &simple_dir_operations;
	inode->i_mode = S_IFDIR | 0755;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_op = &binderfs_dir_inode_operations;
	set_nlink(inode, 2);

	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	ret = binderfs_binder_ctl_create(sb);
	if (ret)
		return ret;

	memset(&device_info, 0, sizeof(device_info));
	name = binder_devices_param;
	for (len = strcspn(name, ","); len > 0; len = strcspn(name, ",")) {
		strlcpy(device_info.name, name, len + 1);
		ret = binderfs_binder_device_create(sb->s_root->d_inode, NULL,
						    &device_info);
		if (ret)
			return ret;
		name += len;
		if (*name == ',')
			name++;
	}

	return 0;
}

static struct dentry *binderfs_mount(struct file_system_type *fs_type,
				     int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, binderfs_fill_super);
}

static struct file_system_type binder_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "binder",
	.mount		= binderfs_mount,
	.kill_sb	= kill_litter_super,
	.fs_flags	= FS_USERNS_MOUNT,
};

int __init init_binderfs(void)
{
	int ret;
	const char *name;
	size_t len;

	name = binder_devices_param;
	for (len = strcspn(name, ","); len > 0; len = strcspn(name, ",")) {
		if (len > BINDERFS_MAX_NAME)
			return -E2BIG;
		name += len;
		if (*name == ',')
			name++;
	}

	ret = alloc_chrdev_region(&binderfs_dev, 0, BINDERFS_MAX_MINOR, "binder");
	if (ret)
		return ret;

	ret = register_filesystem(&binder_fs_type);
	if (ret) {
		unregister_chrdev_region(binderfs_dev, BINDERFS_MAX_MINOR);
		return ret;
	}

	return ret;
}
