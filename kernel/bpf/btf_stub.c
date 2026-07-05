// SPDX-License-Identifier: GPL-2.0
/*
 * btf_stub.c - BTF is stripped from this backport. Provide minimal symbols so
 * the syscall BTF command paths link and cleanly reject with -EINVAL.
 */
#include <linux/bpf.h>
#include <linux/btf.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>

const struct file_operations btf_fops = { };

void btf_put(struct btf *btf) { }

int btf_new_fd(const union bpf_attr *attr) { return -EINVAL; }

struct btf *btf_get_by_fd(int fd) { return ERR_PTR(-EINVAL); }

int btf_get_info_by_fd(const struct btf *btf, const union bpf_attr *attr,
		       union bpf_attr __user *uattr) { return -EINVAL; }

int btf_get_fd_by_id(u32 id) { return -EINVAL; }

u32 btf_id(const struct btf *btf) { return 0; }
