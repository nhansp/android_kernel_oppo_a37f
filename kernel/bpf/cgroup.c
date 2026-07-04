/* WARNING: cgroup_bpf uses isolated TU. No <linux/cgroup.h> include.
   If cgroup struct layout changes, verify offsets manually. */

#include <linux/bpf.h>
#include <linux/bpf-cgroup.h>

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

int __cgroup_bpf_run_filter_skb(struct sock *sk, struct sk_buff *skb,
				 enum bpf_attach_type type)
{
	return 0;
}
