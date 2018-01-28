#ifndef KERNEL_H
# define KERNEL_H

#include "cmd.h"
#include "kvm.h"

#define BZ_MAGIC "HdrS"
#define PROTOCOL_VERSION 0x206
#define SETUP_HDR 0x01f1
#define SETUP_SECTS 4

#define SETUP_LOAD_ADDR 0x7000
#define SETUP_LOAD_END 0xA0000

#define CMD_OFFSET 0x10000

#define IMAGE_LOAD_ADDR 0x100000

#define BOOT_CS 0x10
#define BOOT_DS 0x18

#define CS_TYPE 0xA
#define DS_TYPE 0x2

void load_kernel(struct vm_state *vms, struct cmd_opts *opts);

#endif /* !KERNEL_H */
