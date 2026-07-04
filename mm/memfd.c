#include <linux/syscalls.h>
SYSCALL_DEFINE2(memfd_create, const char __user *, name, unsigned int, flags)
{
	return -ENOSYS;
}
