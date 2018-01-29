#ifndef IO_H
# define IO_H

#include <linux/kvm.h>

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;

#define SERIAL_PORT 0x3f8
#define IO_POLL 0x20

void io_emulate(struct kvm_run *run);

#endif /* !IO_H */
