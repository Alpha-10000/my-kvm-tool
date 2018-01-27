#include <err.h>
#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>

#include "io.h"

void io_emulate(struct kvm_run *run)
{
	/*if (ioperm(SERIAL_PORT, 1, 1) == -1)
	  err(1, "unable to access serial port");*/

	switch (run->io.direction) {
	case KVM_EXIT_IO_IN:
/*#define IN(SZ, STR) do {						\
			if (run->io.size == SZ / 8) {			\
				u##SZ val = 0;				\
				val = in##STR(run->io.port);		\
				*((u##SZ*)((char*)run)			\
					+ run->io.data_offset) = val;	\
			}						\
		} while (0)

		IN(8, b);
		IN(16, w);
		IN(32, l);
#undef IN*/
		break;
	case KVM_EXIT_IO_OUT:
/*#define OUT(SZ, STR) do {						\
			if (run->io.size == SZ / 8) {			\
				u##SZ val = *((u##SZ*)((char*)run)	\
					+ run->io.data_offset);		\
				out##STR(val, run->io.port);		\
			}						\
		} while (0)

		OUT(8, b);
		OUT(16, w);
		OUT(32, l);
#undef OUT*/
		write(1, (char*)run + run->io.data_offset, run->io.size);
		break;
	default:
		warn("Unknown IO direction");
		break;
	};

/*	if (ioperm(SERIAL_PORT, 1, 0) == -1)
	err(1, "unable to drop port privilege");*/
}
