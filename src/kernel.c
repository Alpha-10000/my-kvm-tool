#include <asm/bootparam.h>
#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "kernel.h"
#include "kvm.h"

void load_kernel(struct vm_state *vms, struct cmd_opts *opts)
{
	int kfd = open(opts->img, O_RDONLY);
	if (kfd < 0)
		err(1, "unable to open linux image");

	struct stat buf;
	if (fstat(kfd, &buf) < 0)
		err(1, "unable to stat linux image");

	void *img = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE,
			MAP_PRIVATE, kfd, 0);
	if (img == MAP_FAILED)
		err(1, "unable to mmap linux image");

	struct boot_params boot;
	memset(&boot, '\0', sizeof (struct boot_params));
	ssize_t setup_hdr_sz = 0x0202 + ((char*)img)[0x0201];
	memcpy(&boot.hdr, (char*)img + SETUP_HDR, setup_hdr_sz);

	if (memcmp(&boot.hdr.header, BZ_MAGIC, strlen(BZ_MAGIC)) != 0)
		err(1, "file is not a valid bzimage");
	if (boot.hdr.version < PROTOCOL_VERSION)
		err(1, "old kernel version");
	if (boot.hdr.setup_sects == 0)
		boot.hdr.setup_sects = SETUP_SECTS;
	boot.hdr.loadflags |= KEEP_SEGMENTS;

	ssize_t setup_size = (boot.hdr.setup_sects + 1) << 9;
	char *mem_setup = mmap(NULL, SETUP_LOAD_END - SETUP_LOAD_ADDR,
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem_setup == MAP_FAILED)
		err(1, "unable to map backing memory");
	memcpy(mem_setup, img, setup_size);
	memcpy(mem_setup, &boot, sizeof (struct boot_params));

	kvm_add_region(vms, 0, SETUP_LOAD_ADDR, SETUP_LOAD_END - SETUP_LOAD_ADDR,
		(uint64_t)mem_setup);

	void *vmlinux = (char*)img + setup_size;
	void *mem_kernel = mmap(NULL, opts->ram, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem_setup == MAP_FAILED)
		err(1, "unable to map backing memory");
	memcpy(mem_kernel, vmlinux, buf.st_size - setup_size);

	kvm_add_region(vms, 0, IMAGE_LOAD_ADDR, opts->ram, (uint64_t)mem_kernel);

	vms->entry = (uint64_t)mem_kernel;

	munmap(img, buf.st_size);
	close(kfd);
}
