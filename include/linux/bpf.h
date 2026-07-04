#ifndef _LINUX_BPF_H
#define _LINUX_BPF_H 1

#include <uapi/linux/bpf.h>
#include <linux/filter.h>

struct bpf_prog_aux;

struct bpf_prog {
    u16         pages;      /* Number of allocated pages */
    u16         jited:1,    /* Is our filter JIT'ed? */
                gpl_compatible:1, /* Is filter GPL compatible? */
                cb_access:1,    /* Is control block accessed? */
                dst_needed:1;   /* Do we need dst entry? */
    enum bpf_prog_type  type;       /* Type of BPF program */
    u32         len;        /* Number of filter blocks */
    struct bpf_prog_aux *aux;       /* Auxiliary fields */
    struct sock_fprog_kern  *orig_prog; /* Original BPF program */
    unsigned int        (*bpf_func)(const void *ctx,
                        const struct bpf_insn *insn);
    /* Instructions for interpreter */
    union {
        struct sock_filter  insns[0];
        struct bpf_insn     insnsi[0];
    };
};

#endif
void bpf_prog_unlock_free(struct bpf_prog *prog);
