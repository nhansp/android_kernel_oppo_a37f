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
	atomic_inc(&prog->aux->refcnt);
	return prog;
}
EXPORT_SYMBOL_GPL(bpf_prog_inc);

void bpf_prog_put(struct bpf_prog *prog)
{
	if (atomic_dec_and_test(&prog->aux->refcnt))
		kfree(prog);
}
EXPORT_SYMBOL_GPL(bpf_prog_put);

struct bpf_prog *bpf_prog_alloc(unsigned int size, gfp_t flags)
{
	struct bpf_prog *prog = kzalloc(size, flags);
	if (prog)
		atomic_set(&prog->aux->refcnt, 1);
	return prog;
}

int __bpf_prog_charge(struct user_struct *user, u32 pages) { return 0; }
void __bpf_prog_uncharge(struct user_struct *user, u32 pages) {}
void bpf_prog_free(struct bpf_prog *prog) { kfree(prog->aux); kfree(prog); }
void bpf_prog_kallsyms_add(struct bpf_prog *prog) {}
void bpf_prog_kallsyms_del(struct bpf_prog *prog) {}
struct bpf_prog *bpf_prog_select_runtime(struct bpf_prog *prog, int *err) { return prog; }
void bpf_prog_alloc_id(struct bpf_prog *prog) {}
void bpf_prog_free_id(struct bpf_prog *prog) {}
