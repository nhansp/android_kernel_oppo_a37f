#include <linux/bpf.h>
#include <linux/bpf-cgroup.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/cgroup.h>
#include <linux/filter.h>

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
	struct cgroup_bpf_prog *entry, *old = NULL;

	if (!prog || type >= MAX_BPF_ATTACH_TYPE)
		return -EINVAL;

	spin_lock(&cgroup_bpf_lock);
	list_for_each_entry(entry, &cgroup_bpf_progs, node) {
		if (entry->type == type) {
			old = entry;
			break;
		}
	}
	spin_unlock(&cgroup_bpf_lock);

	if (old) {
		bpf_prog_put(old->prog);
		spin_lock(&cgroup_bpf_lock);
		list_del_rcu(&old->node);
		spin_unlock(&cgroup_bpf_lock);
		synchronize_rcu();
		kfree(old);
	}

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
			enum bpf_attach_type type, u32 flags)
{
	struct cgroup_bpf_prog *entry;

	spin_lock(&cgroup_bpf_lock);
	list_for_each_entry(entry, &cgroup_bpf_progs, node) {
		if (entry->type == type) {
			list_del_rcu(&entry->node);
			spin_unlock(&cgroup_bpf_lock);
			synchronize_rcu();
			bpf_prog_put(entry->prog);
			kfree(entry);
			return 0;
		}
	}
	spin_unlock(&cgroup_bpf_lock);
	return -ENOENT;
}

static int cgroup_bpf_run(int type, void *ctx)
{
	struct cgroup_bpf_prog *entry;
	int ret = 0;

	rcu_read_lock();
	list_for_each_entry_rcu(entry, &cgroup_bpf_progs, node) {
		if (entry->type == type && entry->prog) {
			ret = BPF_PROG_RUN(entry->prog, ctx);
			if (ret)
				break;
		}
	}
	rcu_read_unlock();
	return ret;
}

int __cgroup_bpf_run_filter_skb(struct sock *sk, struct sk_buff *skb,
				enum bpf_attach_type type)
{ return cgroup_bpf_run(type, skb); }

int __cgroup_bpf_run_filter_sk(struct sock *sk, enum bpf_attach_type type)
{ return cgroup_bpf_run(type, sk); }

int __cgroup_bpf_run_filter_sock_addr(struct sock *sk, struct sockaddr *uaddr,
		enum bpf_attach_type type, void *ignored) { return 0; }
int __cgroup_bpf_run_filter_setsockopt(struct sock *sk, int *level, int *optname,
		char __user *optval, int *optlen, char **kernel_optval) { return 0; }
int __cgroup_bpf_run_filter_getsockopt(struct sock *sk, int level, int optname,
		char __user *optval, int __user *optlen, int retval, int max_optlen) { return retval; }
