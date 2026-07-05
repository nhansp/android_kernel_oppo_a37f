/* SPDX-License-Identifier: GPL-2.0 */
/*
 * bpf_compat_310.h - backport shims for the additive eBPF port on 3.10.
 * Provides the handful of helpers newer kernels expose that 3.10 lacks.
 * CONFIG_BPF_JIT stays OFF, so the JIT-only helpers are inert no-ops.
 */
#ifndef _LINUX_BPF_COMPAT_310_H
#define _LINUX_BPF_COMPAT_310_H

#include <linux/types.h>
#include <linux/math64.h>
#include <linux/random.h>
#include <linux/skbuff.h>
#include <linux/ktime.h>
#include <linux/time.h>

struct bpf_prog;

/* eBPF ALU64 MOD needs the remainder-returning divide; build over div64_u64. */
#ifndef div64_u64_rem
static inline u64 div64_u64_rem(u64 dividend, u64 divisor, u64 *remainder)
{
	u64 q = div64_u64(dividend, divisor);

	*remainder = dividend - q * divisor;
	return q;
}
#endif

/* JIT off: program pages are never remapped read-only. */
#ifndef CONFIG_BPF_JIT
static inline void bpf_prog_lock_ro(struct bpf_prog *fp) { }
#endif

/* 3.10 seeds prandom at boot; the per-cpu once-init is a no-op here. */
static inline void prandom_init_once(void *pcpu_state) { }

/* LD_ABS/LD_IND load helper over the classic skb accessor. */
extern void *bpf_internal_load_pointer_neg_helper(const struct sk_buff *skb,
						  int k, unsigned int size);
static inline void *bpf_load_pointer(const struct sk_buff *skb, int k,
				     unsigned int size, void *buffer)
{
	if (k >= 0)
		return skb_header_pointer(skb, k, size, buffer);
	return bpf_internal_load_pointer_neg_helper(skb, k, size);
}

/* ktime fast-ns accessors (4.x): map onto 3.10 monotonic/boottime clocks. */
static inline u64 ktime_get_mono_fast_ns(void)
{
	return ktime_to_ns(ktime_get());
}
static inline u64 ktime_get_boot_fast_ns(void)
{
	struct timespec ts;

	get_monotonic_boottime(&ts);
	return timespec_to_ns(&ts);
}

/* JIT off: prog kallsyms tracking is inert. */
#ifndef CONFIG_BPF_JIT
static inline void bpf_prog_kallsyms_add(struct bpf_prog *fp) { }
static inline void bpf_prog_kallsyms_del(struct bpf_prog *fp) { }
#endif

#endif /* _LINUX_BPF_COMPAT_310_H */
