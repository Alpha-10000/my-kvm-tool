#ifndef CMD_H
# define CMD_H

struct cmd_opts {
	char *img;
	char *initrd;
	int ram;
	int help;
	char *root;
	char *console;
};

void parse_command_line(int argc, char *argv[], struct cmd_opts *opts);

#endif /* !CMD_H */
