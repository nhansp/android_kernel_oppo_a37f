#ifndef _LINUX_REFCOUNT_H
#define _LINUX_REFCOUNT_H
#include <linux/atomic.h>
#include <linux/bug.h>

typedef atomic_t refcount_t;
#define REFCOUNT_INIT(n) ATOMIC_INIT(n)
static inline void refcount_set(refcount_t *r, unsigned int n) { atomic_set(r, n); }
static inline unsigned int refcount_read(const refcount_t *r) { return atomic_read(r); }
static inline void refcount_inc(refcount_t *r) { atomic_inc(r); }
static inline bool refcount_inc_not_zero(refcount_t *r) { return atomic_inc_not_zero(r); }
static inline bool refcount_sub_and_test(unsigned int i, refcount_t *r) { return atomic_sub_and_test(i, r); }
static inline bool refcount_dec_and_test(refcount_t *r) { return atomic_sub_and_test(1, r); }
static inline bool refcount_dec(refcount_t *r) { atomic_dec(r); return atomic_read(r) == 0; }
#endif
