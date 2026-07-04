#ifndef _UAPI__LINUX_BPF_H__
#define _UAPI__LINUX_BPF_H__

#include <linux/types.h>

/* Extended instruction set based on top of classic BPF */

/* instruction classes */
#define BPF_ALU64   0x07    /* alu mode in double word width */
#define BPF_JMP32   0x06    /* jmp mode in word width */

/* alu/jmp fields */
#define BPF_DW      0x18    /* double word (64-bit) */
#define BPF_XADD    0xc0    /* exclusive add */
#define BPF_MOV     0xb0    /* mov reg to reg */
#define BPF_ARSH    0xc0    /* sign extending arithmetic shift right */

/* Endianness conversion, instruction classes */
#define BPF_END     0xd0    /* endianness conversion */
#define BPF_TO_LE   0x00    /* convert to little-endian */
#define BPF_TO_BE   0x08    /* convert to big-endian */

/* jmp encodings */
#define BPF_JNE     0x50    /* jump != */
#define BPF_JLT     0xa0    /* LT is unsigned, '<' */
#define BPF_JLE     0xb0    /* LE is unsigned, '<=' */
#define BPF_JSGT    0x60    /* SGT is signed '>', GT in x86 */
#define BPF_JSGE    0x70    /* SGE is signed '>=', GE in x86 */
#define BPF_JSLT    0xc0    /* SLT is signed, '<' */
#define BPF_JSLE    0xd0    /* SLE is signed, '<=' */
#define BPF_CALL    0x80    /* function call */
#define BPF_EXIT    0x90    /* function return */

/* Register numbers */
enum {
    BPF_REG_0 = 0,
    BPF_REG_1,
    BPF_REG_2,
    BPF_REG_3,
    BPF_REG_4,
    BPF_REG_5,
    BPF_REG_6,
    BPF_REG_7,
    BPF_REG_8,
    BPF_REG_9,
    BPF_REG_10,
    __MAX_BPF_REG,
};

struct bpf_insn {
    __u8    code;       /* opcode */
    __u8    dst_reg:4;  /* dest register */
    __u8    src_reg:4;  /* source register */
    __s16   off;        /* signed offset */
    __s32   imm;        /* signed immediate constant */
};

/* BPF program types */
enum bpf_prog_type {
    BPF_PROG_TYPE_UNSPEC,
    BPF_PROG_TYPE_SOCKET_FILTER,
    BPF_PROG_TYPE_KPROBE,
    BPF_PROG_TYPE_SCHED_CLS,
    BPF_PROG_TYPE_SCHED_ACT,
};


enum bpf_map_type {
    BPF_MAP_TYPE_UNSPEC,
    BPF_MAP_TYPE_HASH,
    BPF_MAP_TYPE_ARRAY,
    BPF_MAP_TYPE_PROG_ARRAY,
    BPF_MAP_TYPE_PERF_EVENT_ARRAY,
    BPF_MAP_TYPE_PERCPU_HASH,
    BPF_MAP_TYPE_PERCPU_ARRAY,
    BPF_MAP_TYPE_STACK_TRACE,
    BPF_MAP_TYPE_CGROUP_ARRAY,
    BPF_MAP_TYPE_LRU_HASH,
    BPF_MAP_TYPE_LRU_PERCPU_HASH,
    BPF_MAP_TYPE_LPM_TRIE,
    BPF_MAP_TYPE_ARRAY_OF_MAPS,
    BPF_MAP_TYPE_HASH_OF_MAPS,
    BPF_MAP_TYPE_DEVMAP,
    BPF_MAP_TYPE_SK_STORAGE = 24,
    BPF_MAP_TYPE_RINGBUF = 27,
};

enum bpf_func_id {
    BPF_FUNC_unspec,
    __BPF_FUNC_MAX_ID,
};

enum bpf_attach_type {
    BPF_CGROUP_INET_INGRESS,
    BPF_CGROUP_INET_EGRESS,
    BPF_CGROUP_INET_SOCK_CREATE,
    BPF_CGROUP_INET4_BIND,
    BPF_CGROUP_INET6_BIND,
    BPF_CGROUP_INET4_CONNECT,
    BPF_CGROUP_INET6_CONNECT,
    BPF_CGROUP_UDP4_SENDMSG,
    BPF_CGROUP_UDP6_SENDMSG,
    BPF_CGROUP_SOCK_OPS,
    BPF_CGROUP_DEVICE,
    BPF_CGROUP_INET4_GETSOCKOPT,
    BPF_CGROUP_INET4_SETSOCKOPT,
    BPF_CGROUP_MAX,
};
#define MAX_BPF_ATTACH_TYPE BPF_CGROUP_MAX
#define BPF_OBJ_NAME_LEN 16U
union bpf_attr {
	struct {
		__u32	map_type;	__u32	key_size;	__u32	value_size;
		__u32	max_entries;	__u32	map_flags;	__u32	inner_map_fd;
	};
	struct {	__u32		map_fd;	__aligned_u64	key;
		__aligned_u64	value;	__aligned_u64	next_key;	__u64	flags;
	};
	struct {	__u32	prog_type;	__u32	insn_cnt;
		__aligned_u64	insns;	__aligned_u64	license;
		__u32	log_level;	__u32	log_size;	__aligned_u64	log_buf;
		__u32	kern_version;	__u32	prog_flags;
	};
	struct {	__u32	target_fd;	__u32	attach_bpf_fd;
		__u32	attach_type;	__u32	attach_flags;
	};
} __attribute__((aligned(8)));
#endif /* _UAPI__LINUX_BPF_H__ */
