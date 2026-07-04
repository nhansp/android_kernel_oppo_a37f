#ifndef _BPF_CGROUP_H
#define _BPF_CGROUP_H

#include <linux/bpf.h>
#include <linux/jump_label.h>
#include <uapi/linux/bpf.h>

struct sock;
struct sockaddr;
struct cgroup;
struct sk_buff;

enum bpf_attach_type {
	BPF_CGROUP_INET_INGRESS,
	BPF_CGROUP_INET_EGRESS,
	BPF_CGROUP_INET_SOCK_CREATE,
	BPF_CGROUP_INET4_BIND,
	BPF_CGROUP_INET6_BIND,
	BPF_CGROUP_INET4_CONNECT,
	BPF_CGROUP_INET6_CONNECT,
	BPF_CGROUP_UDP4_SENDMSG,
	BPF_CGROUP_UDP6_SENDMSG,
	BPF_CGROUP_SOCK_OPS,
	BPF_CGROUP_DEVICE,
	BPF_CGROUP_INET4_GETSOCKOPT,
	BPF_CGROUP_INET4_SETSOCKOPT,
	BPF_CGROUP_MAX,
};

int __cgroup_bpf_attach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type, u32 flags);
int __cgroup_bpf_detach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type);
void __cgroup_bpf_run_filter(struct sock *sk, struct sk_buff *skb,
			     enum bpf_attach_type type);
void __cgroup_bpf_run_filter_skb(struct sock *sk, struct sk_buff *skb,
				 enum bpf_attach_type type);
void __cgroup_bpf_run_filter_sk(struct sock *sk, enum bpf_attach_type type);

static inline void cgroup_bpf_get(struct cgroup *cgrp) {}
static inline void cgroup_bpf_put(struct cgroup *cgrp) {}

#endif /* _BPF_CGROUP_H */
