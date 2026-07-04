#include <linux/syscalls.h>
#include <linux/shmem_fs.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define MFD_CLOEXEC       0x0001U
#define MFD_ALLOW_SEALING  0x0002U
#define MFD_HUGETLB       0x0004U
#define MFD_NOEXEC_SEAL   0x0008U
#define MFD_EXEC          0x0010U
#define MFD_ALL_FLAGS     (MFD_CLOEXEC | MFD_ALLOW_SEALING | MFD_HUGETLB | MFD_EXEC | MFD_NOEXEC_SEAL)
#define MFD_NAME_MAX_LEN  (NAME_MAX - 1)

SYSCALL_DEFINE2(memfd_create,
		const char __user *, uname,
		unsigned int, flags)
{
	struct file *file;
	int fd, error;
	char *name;
	long len;

	if (flags & ~(unsigned int)MFD_ALL_FLAGS)
		return -EINVAL;

	if ((flags & MFD_EXEC) && (flags & MFD_NOEXEC_SEAL))
		return -EINVAL;

	len = strnlen_user(uname, MFD_NAME_MAX_LEN + 1);
	if (len <= 0)
		return -EFAULT;
	if (len > MFD_NAME_MAX_LEN + 1)
		return -EINVAL;

	name = kmalloc(len + 1, GFP_KERNEL);
	if (!name)
		return -ENOMEM;

	if (copy_from_user(name, uname, len)) {
		error = -EFAULT;
		goto err_name;
	}
	name[len] = '\0';

	fd = get_unused_fd_flags((flags & MFD_CLOEXEC) ? O_CLOEXEC : 0);
	if (fd < 0) {
		error = fd;
		goto err_name;
	}

	file = shmem_file_setup(name, 0, VM_NORESERVE);
	if (IS_ERR(file)) {
		error = PTR_ERR(file);
		goto err_fd;
	}

	file->f_mode |= FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE;
	file->f_flags |= O_RDWR | O_LARGEFILE;

	if (flags & MFD_ALLOW_SEALING)
		file->f_inode->i_flags |= S_SEALABLE_FL;

	fd_install(fd, file);
	kfree(name);
	return fd;

err_fd:
	put_unused_fd(fd);
err_name:
	kfree(name);
	return error;
}
