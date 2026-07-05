# PHASE 0 stub checklist — build the CGROUP_SKB control plane on 3.10 (NO WALL, 18 bounded stubs)

From workflow `wik7315p5` (verified against the tree). **Every stub is bounded — zero net-stack ABI
risk.** The only edit to a pre-existing struct is `user_struct.locked_vm` (a guard-widen, no layout
change; `user_struct` is not a WALL struct). The verifier stays WALL-free by construction: all
`sk_buff`/ctx layout knowledge lives behind `bpf_verifier_ops` fn-pointers, never in the verifier.

## ⚠️ ITEM #1 — the root cause of the `core.c` incomplete-type errors — AND the wall trap
The branch `filter.h` is still the **154-line classic** header (no `struct bpf_prog`), so `core.c`
fails with "dereferencing pointer to incomplete type." **BUT DO NOT install the ref's 952-line
`filter.h`** — verified: it has `sk_run_filter` = **0** (DELETED) → that is the exact D11/D12
net-stack wall. Instead, **ADD the eBPF type block to the classic 154-line `filter.h` ADDITIVELY**,
keeping `sk_run_filter` + the classic `struct sk_filter` interpreter intact:
- Add: `struct bpf_prog`, `struct bpf_prog_aux` (`atomic_t refcnt; user; u32 id; used_maps; work`),
  `struct bpf_insn`, `BPF_CALL_0..5`, `BPF_PROG_RUN`, `MAX_BPF_STACK`, `MAX_BPF_REG`, `BPF_REG_*`,
  `bpf_load_pointer` inline (over existing `skb_header_pointer`).
- Keep: `sk_run_filter`, classic `struct sk_filter`. cBPF + eBPF coexist (mainline did 3.15→4.x).
**This gates everything — do it first, additively.**

## Already on the branch (verify only, no new work)
`include/linux/bpf.h` (533L: `bpf_map`/`bpf_array`/`bpf_map_ops`/`enum bpf_reg_type`/
`bpf_verifier_ops`/`bpf_long_memcpy`/`bpf_map_area_alloc`), `include/linux/bpf_verifier.h` (env/
state/reg_state/insn_aux_data/log), `pcpu_freelist.{c,h}` (0 4.x symbols), `bpf_map_inc/put/get`,
`bpf_patch_insn_single`. (`bpf.h`: add forward-decls for `struct btf` / `struct vm_area_struct`.)

## Trivial cluster — one-liners (new macro/inline, additive)
2. `READ_ONCE`/`WRITE_ONCE` → `#define READ_ONCE(x) ACCESS_ONCE(x)` etc. (base has only ACCESS_ONCE). core.c.
3. `INIT_LIST_HEAD_RCU` → `#define … INIT_LIST_HEAD(l)` in rculist.h. core.c (inert with JIT off).
4. `div64_u64_rem` → inline over existing `div64_u64` in math64.h. core.c ALU64 MOD.
5. `u64_to_user_ptr` → `#define … ((void __user*)(uintptr_t)(x))` in kernel.h. syscall.c.
6. `STACK_FRAME_NON_STANDARD(func)` → **new** `include/linux/frame.h`, empty macro (no objtool). core.c.
7. empty `include/linux/btf.h` (forward-decl `struct btf`) so residual `#include` resolves after BTF strip.
8. `hlist_nulls_for_each_entry_safe` + `hlist_nulls_entry_safe` → canonical upstream defs in
   rculist_nulls.h (only `_rcu` exists in base). hashtab.c free + GET_NEXT_KEY paths.
9. `PAGE_ALIGNED` → `IS_ALIGNED(...)` — but DROPPED by strip (mmapable branch removed). Likely 0 uses.

## Shims / config
10. `security_bpf`/`_map`/`_prog`/`_map_alloc`/`_free`/`_prog_alloc`/`_free` (7 LSM hooks) → additive
    static-inline no-ops (return 0/void). **No LSM struct modified.** syscall.c (9 sites).
11. `user_struct.locked_vm` — field EXISTS (sched.h:767) under `#ifdef CONFIG_PERF_EVENTS`; widen to
    `#if defined(CONFIG_PERF_EVENTS)||defined(CONFIG_BPF_SYSCALL)`. No new field, no layout change.
12. Kconfig: add `CONFIG_BPF`/`BPF_SYSCALL`/`CGROUP_BPF`. **Leave `CONFIG_BPF_JIT` OFF** (interpreter-only)
    → drops the whole `rbtree_latch.h`/`latch_tree`/`module_alloc`/kallsyms block in core.c.
13. `include/uapi/linux/bpf.h` (install ref's, it's uapi-only): full `enum bpf_cmd`, `enum bpf_func_id`
    + large `__BPF_FUNC_MAX_ID`, `enum bpf_prog_type` incl `CGROUP_SKB=8`, `struct __sk_buff`,
    `union bpf_attr`, `BPF_MAP_TYPE_ARRAY/PERCPU_ARRAY/HASH/PERCPU_HASH`. (`BPF_FS_MAGIC` → magic.h.)
14. `kernel/bpf/Makefile`: keep `core.o cgroup.o syscall.o arraymap.o hashtab.o helpers.o
    percpu_freelist.o`; **strip** `bpf_lru_list.o stackmap.o devmap.o lpm_trie.o map_in_map.o ringbuf.o`.

## STRIP (removes deps entirely — don't port these)
mmapable map branch (→ no `vmalloc_user_node_flags`), all BTF paths (34 refs), tail_call/PROG_ARRAY,
LD_ABS/LD_IND, the entire `CONFIG_BPF_JIT` block in core.c, `BPF_OBJ_GET_INFO`/prog_info/map_info
(→ removes most `u64_to_user_ptr`), `prandom_init_once` (drop if `bpf_user_rnd_u32` unregistered),
LRU_HASH/stackmap/devmap/lpm_trie/map_in_map/ringbuf.

## Incremental link order
- **(a) ARRAY-map + trivial PROG_LOAD:** items #1, bpf.h(present), uapi #13(partial: ARRAY+attr+cmd),
  #6 frame.h, #7 btf.h, trivials #2/3/4/5, security shim #10, locked_vm #11, Kconfig BPF+BPF_SYSCALL,
  Makefile #14 → links `core.o(interpreter)+arraymap.o+syscall.o`.
- **(b) hashtab:** add only the 2 hlist_nulls macros (#8). percpu_freelist compiles as-is.
- **(c) CGROUP_SKB prog-type + verifier gates:** bpf_verifier.h(present) + `enum bpf_func_id`/
  `__BPF_FUNC_MAX_ID` + `bpf_prog_type CGROUP_SKB=8` + the CGROUP_SKB `bpf_verifier_ops`
  (is_valid_access/convert_ctx_access — supplied by cgroup.c/net-filter, keeps the verifier WALL-free)
  + CONFIG_CGROUP_BPF + cgroup.o.

**Gate each phase on: `nm vmlinux | grep -w sys_bpf` = `T` (not `W`) after (a); no `sk_run_filter`
deletions in `git diff -- include/linux/filter.h`; ABI stop-conditions (`mm_types/sched/skbuff/sock`)
stay empty.**
