#include <linux/filter.h>
#include <linux/bpf.h>
#include <linux/export.h>
#include <linux/slab.h>

unsigned int __bpf_prog_run(void *ctx, const struct bpf_insn *insn)
{
	return 0;
}
EXPORT_SYMBOL_GPL(__bpf_prog_run);

struct bpf_prog *bpf_prog_inc(struct bpf_prog *prog)
{
	return prog;
}
EXPORT_SYMBOL_GPL(bpf_prog_inc);

void bpf_prog_put(struct bpf_prog *prog)
{
}
EXPORT_SYMBOL_GPL(bpf_prog_put);

int __bpf_prog_charge(struct user_struct *user, u32 pages)
{
	return 0;
}

void __bpf_prog_uncharge(struct user_struct *user, u32 pages)
{
}
