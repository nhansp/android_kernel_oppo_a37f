#include <linux/syscalls.h>
#include <linux/pid.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/anon_inodes.h>

SYSCALL_DEFINE2(pidfd_open, pid_t, pid, unsigned int, flags)
{
	return -ENOSYS;
}

SYSCALL_DEFINE4(pidfd_send_signal, int, pidfd, int, sig,
		siginfo_t __user *, info, unsigned int, flags)
{
	return -ENOSYS;
}
