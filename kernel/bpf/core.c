#include <linux/filter.h>
#include <linux/bpf.h>
#include <linux/export.h>

unsigned int __bpf_prog_run(void *ctx, const struct bpf_insn *insn)
{
	return 0;
}
EXPORT_SYMBOL_GPL(__bpf_prog_run);
