#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>

#include "io.h"

void io_emulate(struct kvm_run *run)
{
	static uint8_t code = 0;
	switch (run->io.direction) {
	case KVM_EXIT_IO_IN:
		if (run->io.port == SERIAL_PORT + 1)
			*((uint8_t*)run + run->io.data_offset) = code;
		if (run->io.port == SERIAL_PORT + 5)
			*((uint8_t*)run + run->io.data_offset) = IO_POLL;
		break;
	case KVM_EXIT_IO_OUT:
		if (run->io.port == SERIAL_PORT)
			write(STDOUT_FILENO, (uint8_t*)run + run->io.data_offset,
				run->io.size);
		if (run->io.port == SERIAL_PORT + 1)
			code = *((uint8_t*)run + run->io.data_offset);
		break;
	default:
		warn("Unknown IO direction");
		break;
	};
}
