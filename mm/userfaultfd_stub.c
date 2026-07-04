#include <linux/syscalls.h>
SYSCALL_DEFINE6(userfaultfd, int, flags, unsigned long, addr, unsigned long, len,
		int __user *, mode, int __user *, features)
{
	return -ENOSYS;
}
