obj-$(CONFIG_BPF) += core.o
obj-$(CONFIG_CGROUP_BPF) += cgroup.o
obj-$(CONFIG_BPF_SYSCALL) += syscall.o arraymap.o hashtab.o
CFLAGS_syscall.o := -Wno-declaration-after-statement
obj-$(CONFIG_BPF_SYSCALL) += bpf_lru_list.o
obj-$(CONFIG_BPF_SYSCALL) += percpu_freelist.o
obj-$(CONFIG_BPF_SYSCALL) += map_in_map.o
