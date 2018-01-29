#ifndef KVM_H
# define KVM_H

#include <linux/kvm.h>
#include <stdint.h>

#include "cmd.h"

#define CPUID_NUM 100

struct vm_state
{
	int fd_kvm;
	int fd_vm;
	int fd_vcpu;
	unsigned int slots;
	struct kvm_run *run;
	uint64_t entry;
};

void kvm_add_region(struct vm_state *vm, uint32_t flags, uint64_t guest_phys,
		uint64_t mem_size, uint64_t user_addr);
void kvm_run(struct cmd_opts *opts);
void kvm_dump_infos(struct vm_state *vm);

#endif /* !KVM_H */
