#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmd.h"

static char *get_cmd_arg(char *cmd_arg, const char *var)
{
	size_t n = strlen(var);
	if (!strncmp(cmd_arg, var, n) && cmd_arg[n] == '=')
		return cmd_arg + n + 1;
	return NULL;
}

static void dump_options(struct cmd_opts *opts)
{
	printf("\n");
	printf("--initrd Initrd: %s\n", opts->initrd);
	printf("-m       RAM memory: %d\n", opts->ram);
	printf("-h       help: %d\n", opts->help);
	printf("BzImage: %s\n", opts->img);
	printf("Root FS: %s\n", opts->root);
	printf("console: %s\n", opts->console);
	printf("\n");
}

void parse_command_line(int argc, char *argv[], struct cmd_opts *opts)
{
	memset(opts, '\0', sizeof(struct cmd_opts));
	for (;;) {
		int opt_ind = 0;
		static struct option long_options[] = {
			{"initrd",  required_argument, 0, 0 },
			{ NULL,     0,                 0, 0 }
		};

		int c = getopt_long(argc, argv, "m:h", long_options, &opt_ind);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			switch (opt_ind) {
			case 0:
				opts->initrd = optarg;
				break;
			default:
				printf("Error: option index %d\n", opt_ind);
				break;
			}
		case 'm':
			// TODO : improve
			opts->ram = atoi(optarg);
			break;
		case 'h':
			opts->help = 1;
			break;
		default:
			printf("Error: character code %d\n", c);
			break;
		}
	}

	while (optind < argc) {
		int image = 1;
#define FIND_ARG(ARG) do {						\
			if (get_cmd_arg(argv[optind], #ARG)) {		\
				opts->ARG = get_cmd_arg(argv[optind], #ARG); \
				image = 0;				\
			}						\
		} while (0)

		FIND_ARG(root);
		FIND_ARG(console);
#undef FIND_ARG
		if (image)
			opts->img = argv[optind];
		optind++;
	}
	if (opts->help)
		dump_options(opts);
}
