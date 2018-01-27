#include <capstone/capstone.h>
#include <err.h>
#include <inttypes.h>
#include <stdio.h>

#include "disasm.h"

void disasm(const uint64_t code, size_t size, uint64_t start_addr)
{
	csh handle;
	if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
		warn("unable to init capstone");
		return;
	}
	cs_insn *insn;
	size_t count = cs_disasm(handle, (uint8_t*)code, size, start_addr, 0, &insn);
	if (count >0) {
		for (size_t j = 0; j < count; j++) {
			printf("0x%"PRIx64":\t%s\t\t%s\n", insn[j].address,
				insn[j].mnemonic, insn[j].op_str);
		}
		cs_free(insn, count);
	} else {
		warn("unable to disassemble code");
	}
	cs_close(&handle);
}
