#include <linux/bpf.h>
#include <linux/bpf-cgroup.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/rculist.h>

struct bpf_cgroup_storage;
struct bpf_cgroup_storage_map;

static DEFINE_PER_CPU(struct bpf_prog *, bpf_cgroup_prog[BPF_CGROUP_MAX]);

int __cgroup_bpf_attach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type, u32 flags)
{
	return -EOPNOTSUPP;
}

int __cgroup_bpf_detach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type)
{
	return -EOPNOTSUPP;
}

void __cgroup_bpf_run_filter(struct sock *sk, struct sk_buff *skb,
			     enum bpf_attach_type type) {}

void __cgroup_bpf_run_filter_skb(struct sock *sk, struct sk_buff *skb,
				 enum bpf_attach_type type) {}

void __cgroup_bpf_run_filter_sk(struct sock *sk, enum bpf_attach_type type) {}
