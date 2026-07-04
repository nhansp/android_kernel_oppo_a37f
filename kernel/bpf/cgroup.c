#include <linux/bpf.h>
#include <linux/bpf-cgroup.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>

/* Root-only single-prog cgroup BPF storage (no cgroup v2 hierarchy needed) */
struct cgroup_bpf_prog {
	struct bpf_prog *prog;
	enum bpf_attach_type type;
	struct list_head node;
};

static LIST_HEAD(cgroup_bpf_progs);
static DEFINE_SPINLOCK(cgroup_bpf_lock);

int __cgroup_bpf_attach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type, u32 flags)
{
	struct cgroup_bpf_prog *entry;

	if (!prog || type >= BPF_CGROUP_MAX)
		return -EINVAL;

	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	bpf_prog_inc(prog);
	entry->prog = prog;
	entry->type = type;

	spin_lock(&cgroup_bpf_lock);
	list_add_rcu(&entry->node, &cgroup_bpf_progs);
	spin_unlock(&cgroup_bpf_lock);
	return 0;
}

int __cgroup_bpf_detach(struct cgroup *cgrp, struct bpf_prog *prog,
			enum bpf_attach_type type)
{
	struct cgroup_bpf_prog *entry, *tmp;

	spin_lock(&cgroup_bpf_lock);
	list_for_each_entry_safe(entry, tmp, &cgroup_bpf_progs, node) {
		if (entry->prog == prog && entry->type == type) {
			list_del_rcu(&entry->node);
			spin_unlock(&cgroup_bpf_lock);
			synchronize_rcu();
			bpf_prog_put(prog);
			kfree(entry);
			return 0;
		}
	}
	spin_unlock(&cgroup_bpf_lock);
	return -ENOENT;
}

/* Run filter on attach type - iterate global list */
static int cgroup_bpf_run(int type, void *ctx)
{
	struct cgroup_bpf_prog *entry;
	int ret = 0;

	rcu_read_lock();
	list_for_each_entry_rcu(entry, &cgroup_bpf_progs, node) {
		if (entry->type == type && entry->prog) {
			ret = bpf_prog_run(entry->prog, ctx);
			if (ret)
				break;
		}
	}
	rcu_read_unlock();
	return ret;
}

#define BPF_RUN_FILTER(type, ctx, ret)		\
	do {					\
		rcu_read_lock();		\
		ret = cgroup_bpf_run(type, ctx);\
		rcu_read_unlock();		\
	} while (0)

int __cgroup_bpf_run_filter_skb(struct sock *sk, struct sk_buff *skb,
				enum bpf_attach_type type)
{
	int ret = 0;
	BPF_RUN_FILTER(type, skb, ret);
	return ret;
}

int __cgroup_bpf_run_filter_sk(struct sock *sk, enum bpf_attach_type type)
{
	int ret = 0;
	BPF_RUN_FILTER(type, sk, ret);
	return ret;
}

int __cgroup_bpf_run_filter_sock_addr(struct sock *sk,
				      struct sockaddr *uaddr,
				      enum bpf_attach_type type, void *ignored)
{
	/* Stub for socket address hooks - requires sock struct access */
	return 0;
}

int __cgroup_bpf_run_filter_setsockopt(struct sock *sk, int *level,
				       int *optname, char __user *optval,
				       int *optlen, char **kernel_optval)
{
	return 0;
}

int __cgroup_bpf_run_filter_getsockopt(struct sock *sk, int level,
				       int optname, char __user *optval,
				       int __user *optlen, int retval,
				       int max_optlen)
{
	return retval;
}
