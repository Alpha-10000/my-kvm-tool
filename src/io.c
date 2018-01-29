#include <err.h>
#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>

#include "io.h"

void io_emulate(struct kvm_run *run)
{
	switch (run->io.direction) {
	case KVM_EXIT_IO_IN:
		if (run->io.port == SERIAL_PORT + 5)
			*((char*)run + run->io.data_offset) = IO_POLL;
                /*else
		warn("IN: unhandled IO port: 0x%x", run->io.port);*/
		break;
	case KVM_EXIT_IO_OUT:
		if (run->io.port == SERIAL_PORT)
			write(STDOUT_FILENO, (char*)run + run->io.data_offset,
			run->io.size);
                /*else
		warn("OUT: unhandled IO port: 0x%x", run->io.port);*/
		break;
	default:
		warn("Unknown IO direction");
		break;
	};
}
