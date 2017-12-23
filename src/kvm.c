#include <err.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "io.h"
#include "kvm.h"

unsigned char out_o[] = {
	0xb0, 0x61, 0x66, 0xba, 0xf8, 0x03, 0xee, 0xeb, 0xf7
};
unsigned int out_o_len = sizeof(out_o) / sizeof(unsigned char);

void kvm_run(struct cmd_opts *opts)
{
	int fd_kvm = open("/dev/kvm", O_RDWR);
	if (fd_kvm < 0)
		err(1, "unable to open /dev/kvm");

	int fd_vm = ioctl(fd_kvm, KVM_CREATE_VM, 0);
	if (fd_vm < 0)
		err(1, "unable to create vm");

	ioctl(fd_vm, KVM_SET_TSS_ADDR, 0xffffd000);
	__u32 map_addr = 0xffffc000;
	ioctl(fd_vm, KVM_SET_IDENTITY_MAP_ADDR, &map_addr);
	ioctl(fd_vm, KVM_CREATE_IRQCHIP, 0);

	void *mem_addr = mmap(NULL, 1 << 12, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem_addr == MAP_FAILED)
		err(1, "unable to map backing memory");
	memcpy(mem_addr, out_o, out_o_len);

	struct kvm_userspace_memory_region region = {
		.slot = 0,
		.flags = 0,
		.guest_phys_addr = 0x100000,
		.memory_size = 1 << 12,
		.userspace_addr = (__u64)mem_addr,
	};
	ioctl(fd_vm, KVM_SET_USER_MEMORY_REGION, &region);

	int fd_vcpu = ioctl(fd_vm, KVM_CREATE_VCPU, 0);

	size_t mmap_sz = ioctl(fd_kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
	struct kvm_run *run = mmap(NULL, mmap_sz, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd_vcpu, 0);
	if (run == MAP_FAILED)
		err(1, "unable to map kvm_run structure");

	struct kvm_sregs sregs;
	ioctl(fd_vcpu, KVM_GET_SREGS, &sregs);
#define set_segment_selector(Seg, Base, Limit, G)	\
	do {						\
		Seg.base = Base;			\
		Seg.limit = Limit;			\
		Seg.g = G;				\
	} while (0)

	set_segment_selector(sregs.cs, 0, ~0, 1);
	set_segment_selector(sregs.ds, 0, ~0, 1);
	set_segment_selector(sregs.ss, 0, ~0, 1);

	sregs.cs.db = 1;
	sregs.ss.db = 1;
#undef set_segment_selector

	sregs.cr0 |= 1;
	ioctl(fd_vcpu, KVM_SET_SREGS, &sregs);

	struct kvm_regs regs;
	ioctl(fd_vcpu, KVM_GET_REGS, &regs);
	regs.rflags = 2;
	regs.rip = 0x100000;
	ioctl(fd_vcpu, KVM_SET_REGS, &regs);

	for (;;) {
		int rc = ioctl(fd_vcpu, KVM_RUN, 0);
		if (rc < 0)
			warn("KVM_RUN");

		switch (run->exit_reason) {
		case KVM_EXIT_IO:
			io_emulate(run);
			break;
		case KVM_EXIT_HLT:
			printf("KVM_EXIT_HLT\n");
			break;
		case KVM_EXIT_FAIL_ENTRY:
			printf("KVM_EXIT_FAIL_ENTRY\n");
			break;
		case KVM_EXIT_INTERNAL_ERROR:
			printf("KVM_EXIT_INTERNAL_ERROR\n");
			break;
		default:
			warnx("Unknown exit reason: 0x%x", run->exit_reason);
			break;

		}
	}
	(void) opts;
}
