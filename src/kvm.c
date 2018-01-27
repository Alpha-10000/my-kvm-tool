#include <err.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "disasm.h"
#include "io.h"
#include "kernel.h"
#include "kvm.h"

void kvm_add_region(struct vm_state *vm, uint32_t flags, uint64_t guest_phys,
		uint64_t mem_size, uint64_t user_addr) {
	struct kvm_userspace_memory_region region = {
		.slot = vm->slots++,
		.flags = flags,
		.guest_phys_addr = guest_phys,
		.memory_size = mem_size,
		.userspace_addr = user_addr,
	};
	ioctl(vm->fd_vm, KVM_SET_USER_MEMORY_REGION, &region);

}

void kvm_run(struct cmd_opts *opts)
{
	struct vm_state *vms = calloc(1, sizeof (struct vm_state));

	vms->fd_kvm = open("/dev/kvm", O_RDWR);
	if (vms->fd_kvm < 0)
		err(1, "unable to open /dev/kvm");

	vms->fd_vm = ioctl(vms->fd_kvm, KVM_CREATE_VM, 0);
	if (vms->fd_vm < 0)
		err(1, "unable to create vm");

	ioctl(vms->fd_vm, KVM_SET_TSS_ADDR, 0xffffd000);

	__u64 map_addr = 0xffffc000;
	ioctl(vms->fd_vm, KVM_SET_IDENTITY_MAP_ADDR, &map_addr);
	ioctl(vms->fd_vm, KVM_CREATE_IRQCHIP, 0);

	vms->fd_vcpu = ioctl(vms->fd_vm, KVM_CREATE_VCPU, 0);

	size_t mmap_sz = ioctl(vms->fd_kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
	vms->run = mmap(NULL, mmap_sz, PROT_READ | PROT_WRITE,
				MAP_SHARED, vms->fd_vcpu, 0);
	if (vms->run == MAP_FAILED)
		err(1, "unable to map kvm_run structure");

	struct kvm_sregs sregs;
	ioctl(vms->fd_vcpu, KVM_GET_SREGS, &sregs);
#define set_segment_selector(Seg, Base, Limit, Sel, Type)		\
	do {								\
		Seg.base = Base;					\
		Seg.limit = Limit;					\
		Seg.g = 1;						\
		Seg.selector = Sel;					\
		Seg.type = Type;					\
	} while (0)

	set_segment_selector(sregs.cs, 0, ~0, BOOT_CS, CS_TYPE);
	set_segment_selector(sregs.ds, 0, ~0, BOOT_DS, DS_TYPE);
	set_segment_selector(sregs.ss, 0, ~0, BOOT_DS, DS_TYPE);
	set_segment_selector(sregs.es, 0, ~0, BOOT_DS, DS_TYPE);
	set_segment_selector(sregs.fs, 0, ~0, BOOT_DS, DS_TYPE);
	set_segment_selector(sregs.gs, 0, ~0, BOOT_DS, DS_TYPE);

	sregs.cs.db = 1;
	sregs.ss.db = 1;
#undef set_segment_selector

	sregs.cr0 |= 1;
	ioctl(vms->fd_vcpu, KVM_SET_SREGS, &sregs);

	struct kvm_regs regs;
	ioctl(vms->fd_vcpu, KVM_GET_REGS, &regs);
	regs.rflags = 2;
	regs.rip = IMAGE_LOAD_ADDR;
	regs.rsi = SETUP_LOAD_ADDR;
	regs.rbp = 0;
	regs.rdi = 0;
	regs.rbx = 0;
	ioctl(vms->fd_vcpu, KVM_SET_REGS, &regs);

	load_kernel(vms, opts);
	kvm_dump_infos(vms);

	int stop = 0;
	struct kvm_guest_debug dbg;
	memset(&dbg, '\0', sizeof (struct kvm_guest_debug));
	dbg.control |= KVM_GUESTDBG_ENABLE | KVM_GUESTDBG_SINGLESTEP;

	while (!stop) {
		ioctl(vms->fd_vcpu, KVM_SET_GUEST_DEBUG, &dbg);

		int rc = ioctl(vms->fd_vcpu, KVM_RUN, 0);
		if (rc < 0)
			warn("KVM_RUN");

		switch (vms->run->exit_reason) {
		case KVM_EXIT_IO:
			io_emulate(vms->run);
			break;
		case KVM_EXIT_HLT:
			printf("KVM_EXIT_HLT\n");
			stop = 1;
			break;
		case KVM_EXIT_FAIL_ENTRY:
			errx(1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
				vms->run->fail_entry.hardware_entry_failure_reason);
			break;
		case KVM_EXIT_INTERNAL_ERROR:
			printf("KVM_EXIT_INTERNAL_ERROR\n");
			stop = 1;
			break;
		case KVM_EXIT_DEBUG:
			printf("DEBUG: 0x%llx\n", vms->run->debug.arch.pc);
			uint64_t code = vms->entry + (vms->run->debug.arch.pc - IMAGE_LOAD_ADDR);
			disasm(code, 0x15, vms->run->debug.arch.pc);
			//getchar();
			break;
		case KVM_EXIT_MMIO:
			printf("KVM_EXIT_MMIO\n");
			break;
		default:
			warnx("Unknown exit reason: 0x%x", vms->run->exit_reason);
			stop = 1;
			break;
		}
	}

	close(vms->fd_vcpu);
	close(vms->fd_vm);
	close(vms->fd_kvm);
	munmap(vms->run, mmap_sz);
	free(vms);
}

#define PR(REG) printf(#REG" : 0x%llx\n", regs.REG)
void kvm_dump_infos(struct vm_state *vms)
{
	struct kvm_regs regs;
	ioctl(vms->fd_vcpu, KVM_GET_REGS, &regs);
	PR(rip);
	PR(rsi);
	PR(rdi);
	PR(rsp);
	PR(rbp);
	PR(rax);
	PR(rbx);
	PR(rcx);
	PR(rdx);
	PR(r8);
	PR(r9);
	PR(r10);
	PR(r11);
	PR(r12);
	PR(r13);
	PR(r14);
	PR(r15);
	PR(rflags);
}
