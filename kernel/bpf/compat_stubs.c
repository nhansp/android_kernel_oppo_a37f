// SPDX-License-Identifier: GPL-2.0
/* eBPF backport weak stubs for perf-array and cgroup-array map deps on 3.10.
 * perf_event_* : perf backport not done (BPF_MAP_TYPE_PERF_EVENT_ARRAY inert).
 * cgroup_*     : overridden by real cgroup-v2 core in stage B (strong wins). */
#include <linux/types.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/bpf.h>

struct perf_event;
struct cgroup;

struct file *__weak perf_event_get(unsigned int fd)
{
	return ERR_PTR(-EOPNOTSUPP);
}

int __weak perf_event_read_local(struct perf_event *event, u64 *value)
{
	return -EOPNOTSUPP;
}

struct cgroup *__weak cgroup_get_from_fd(int fd)
{
	return ERR_PTR(-EOPNOTSUPP);
}

void __weak cgroup_put(struct cgroup *cgrp)
{
}
