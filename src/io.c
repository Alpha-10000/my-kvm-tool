#include <err.h>
#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>

#include "io.h"

void io_emulate(struct kvm_run *run)
{
	switch (run->io.direction) {
	case KVM_EXIT_IO_IN:
		break;
	case KVM_EXIT_IO_OUT:
		write(STDOUT_FILENO, (char*)run + run->io.data_offset, run->io.size);
		break;
	default:
		warn("Unknown IO direction");
		break;
	};
}
