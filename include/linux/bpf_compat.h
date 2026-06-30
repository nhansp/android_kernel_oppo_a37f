#ifndef _LINUX_BPF_COMPAT_H
#define _LINUX_BPF_COMPAT_H
/* eBPF backport shims: 3.10 -> 4.x APIs needed by acro bpf core (arm64) */
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/percpu.h>
#include <linux/ktime.h>
#include <linux/dcache.h>
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/uaccess.h>

#ifndef __alloc_percpu_gfp
#define __alloc_percpu_gfp(size, align, gfp)	__alloc_percpu((size), (align))
#endif

static inline void *kvmalloc(size_t size, gfp_t flags)
{
	void *ret = kmalloc(size, flags | __GFP_NOWARN);
	if (ret)
		return ret;
	return vmalloc(size);
}
static inline void *kvzalloc(size_t size, gfp_t flags)
{
	return kvmalloc(size, flags | __GFP_ZERO);
}
static inline void *kvmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;
	return kvmalloc(n * size, flags);
}
static inline void *kvcalloc(size_t n, size_t size, gfp_t flags)
{
	return kvmalloc_array(n, size, flags | __GFP_ZERO);
}

#ifndef u64_to_user_ptr
#define u64_to_user_ptr(x) ((void __user *)(unsigned long)(x))
#endif

#ifndef PAGE_ALIGNED
#define PAGE_ALIGNED(addr)	IS_ALIGNED((unsigned long)(addr), PAGE_SIZE)
#endif

static inline struct inode *d_inode(const struct dentry *dentry)
{
	return dentry->d_inode;
}
static inline struct inode *d_backing_inode(const struct dentry *upper)
{
	return upper->d_inode;
}

static inline u64 ktime_get_mono_fast_ns(void)
{
	return ktime_to_ns(ktime_get());
}
static inline u64 ktime_get_boot_fast_ns(void)
{
	return ktime_to_ns(ktime_get_boottime());
}

/* vmalloc flags variant */
#ifndef vmalloc_user_node_flags
#define vmalloc_user_node_flags(size, node, flags)	vmalloc_user(size)
#endif

/* perf/cgroup symbols backed by weak stubs in kernel/bpf/compat_stubs.c
 * (real cgroup_* provided by cgroup-v2 core in stage B). */
struct perf_event;
struct cgroup;
struct file *perf_event_get(unsigned int fd);
int perf_event_read_local(struct perf_event *event, u64 *value);
struct cgroup *cgroup_get_from_fd(int fd);
void cgroup_put(struct cgroup *cgrp);

#endif /* _LINUX_BPF_COMPAT_H */
