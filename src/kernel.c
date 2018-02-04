#include <asm/bootparam.h>
#include <asm/e820.h>
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

void load_initrd(char *mem, struct cmd_opts *opts, struct setup_header *hdr)
{
	int ifd = open(opts->initrd, O_RDONLY);
	if (ifd < 0)
		err(1, "unable to open initramfs file");

	struct stat buf;
	if (fstat(ifd, &buf) < 0)
		err(1, "unable to stat initrd file");

	hdr->type_of_loader = 0xff;

	unsigned int size = buf.st_size;

	unsigned long addr = hdr->initrd_addr_max;
	for (;;) {
		if (addr < IMAGE_LOAD_ADDR)
			err(1, "Not enough ram for initrd");
		if (addr < IMAGE_LOAD_ADDR + opts->ram - size)
			break;
		addr -= IMAGE_LOAD_ADDR;
	}

	char *img = mmap(NULL, size, PROT_READ, MAP_PRIVATE, ifd, 0);
	if (img == MAP_FAILED)
		err(1, "unable to mmap initramfs");

	hdr->ramdisk_image = addr;
	hdr->ramdisk_size = size;

	memcpy(mem + (addr - IMAGE_LOAD_ADDR), img, size);

	munmap(img, size);
	close(ifd);
}

void load_kernel(struct vm_state *vms, struct cmd_opts *opts)
{
	int kfd = open(opts->img, O_RDONLY);
	if (kfd < 0)
		err(1, "unable to open linux image");

	struct stat buf;
	if (fstat(kfd, &buf) < 0)
		err(1, "unable to stat linux image");

	char *img = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE,
			MAP_PRIVATE, kfd, 0);
	if (img == MAP_FAILED)
		err(1, "unable to mmap linux image");

	char *mem_setup = mmap(NULL, SETUP_LOAD_END - SETUP_LOAD_ADDR,
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem_setup == MAP_FAILED)
		err(1, "unable to map backing memory");

	struct boot_params boot;
	memset(&boot, 0, sizeof (struct boot_params));
	ssize_t setup_hdr_sz = 0x0202 + img[0x0201];
	memcpy(&boot.hdr, img + SETUP_HDR, setup_hdr_sz);

	if (memcmp(&boot.hdr.header, BZ_MAGIC, strlen(BZ_MAGIC)) != 0)
		err(1, "file is not a valid bzimage");
	if (boot.hdr.version < PROTOCOL_VERSION)
		err(1, "old kernel version");
	if (boot.hdr.setup_sects == 0)
		boot.hdr.setup_sects = SETUP_SECTS;

	boot.hdr.loadflags |= KEEP_SEGMENTS;

	boot.hdr.cmd_line_ptr = SETUP_LOAD_ADDR + CMD_OFFSET;
	boot.hdr.cmdline_size = opts->kcmd_sz;
	memcpy(mem_setup + CMD_OFFSET, opts->kcmd, opts->kcmd_sz);

	boot.e820_entries = E820_NUM;

	boot.e820_table[0].addr = SETUP_LOAD_ADDR;
	boot.e820_table[0].size = SETUP_LOAD_END - SETUP_LOAD_ADDR;
	boot.e820_table[0].type = E820_RAM;

	boot.e820_table[1].addr = IMAGE_LOAD_ADDR;
	boot.e820_table[1].size = opts->ram;
	boot.e820_table[1].type = E820_RAM;

	ssize_t setup_size = (boot.hdr.setup_sects + 1) << 9;
	memcpy(mem_setup, img, setup_size);
	memcpy(mem_setup, &boot, sizeof (struct boot_params));

	kvm_add_region(vms, 0, SETUP_LOAD_ADDR, SETUP_LOAD_END - SETUP_LOAD_ADDR,
		(uint64_t)mem_setup);

	char *vmlinux = img + setup_size;
	void *mem_kernel = mmap(NULL, opts->ram, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem_setup == MAP_FAILED)
		err(1, "unable to map backing memory");
	memcpy(mem_kernel, vmlinux, buf.st_size - setup_size);

	load_initrd(mem_kernel, opts, &boot.hdr);

	kvm_add_region(vms, 0, IMAGE_LOAD_ADDR, opts->ram, (uint64_t)mem_kernel);

	vms->entry = (uint64_t)mem_kernel;

	munmap(img, buf.st_size);
	close(kfd);
}
