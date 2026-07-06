/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BINDER_INTERNAL_H
#define _BINDER_INTERNAL_H

#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/uidgid.h>

struct binder_node;

struct binder_context {
	struct binder_node *binder_context_mgr_node;
	kuid_t binder_context_mgr_uid;
	const char *name;
};

struct binder_device {
	struct hlist_node hlist;
	struct miscdevice miscdev;
	struct binder_context context;
	struct inode *binderfs_inode;	/* CONFIG_ANDROID_BINDERFS */
	atomic_t ref;			/* 3.10 predates refcount_t */
};

extern const struct file_operations binder_fops;
extern char *binder_devices_param;

#ifdef CONFIG_ANDROID_BINDERFS
int init_binderfs(void);
bool is_binderfs_device(const struct inode *inode);
#else
static inline int init_binderfs(void) { return 0; }
#endif

#endif /* _BINDER_INTERNAL_H */
