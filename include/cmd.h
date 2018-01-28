#ifndef CMD_H
# define CMD_H

#include <stddef.h>

struct cmd_opts {
	char *img;
	char *initrd;
	int ram;
	int help;
	char *kcmd;
	size_t kcmd_sz;
};

void parse_command_line(int argc, char *argv[], struct cmd_opts *opts);

#endif /* !CMD_H */
