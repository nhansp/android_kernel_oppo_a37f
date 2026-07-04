#include <linux/filter.h>
#include <linux/bpf.h>
#include <linux/moduleloader.h>

void bpf_jit_compile(struct sk_filter *fp)
{
}

void bpf_jit_free(struct sk_filter *fp)
{
}

void bpf_prog_unlock_free(struct bpf_prog *prog)
{
}
int bpf_jit_enable __read_mostly;
EXPORT_SYMBOL(bpf_jit_enable);
