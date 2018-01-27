#ifndef DISASM_H
# define DISASM_H

#include <stdint.h>

void disasm(const uint64_t code, size_t size, uint64_t start_addr);

#endif /* !DISASM_H */
