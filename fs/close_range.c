#include <linux/syscalls.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/close_range.h>
#include <linux/fs.h>

SYSCALL_DEFINE3(close_range, unsigned int, fd, unsigned int, max_fd,
		unsigned int, flags)
{
	return __close_range(fd, max_fd, flags);
}
