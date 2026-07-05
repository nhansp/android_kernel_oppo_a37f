#include <linux/filter.h>
#include <linux/bpf.h>
#include <linux/export.h>
#include <linux/slab.h>

unsigned int __bpf_prog_run(void *ctx, const struct bpf_insn *insn)
{
	return 0;
}
EXPORT_SYMBOL_GPL(__bpf_prog_run);

void bpf_prog_kallsyms_add(struct bpf_prog *prog) {}
void bpf_prog_kallsyms_del(struct bpf_prog *prog) {}
u64 __bpf_call_base;
