#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmd.h"

static int is_cmd(const char *arg)
{
	for (size_t i = 0; arg[i] != '\0'; i++) {
		if (arg[i] == '=')
			return 1;
	}
	return 0;
}

static void add_kmd(struct cmd_opts *opts, const char *cmd_arg)
{
	int pad = opts->kcmd_sz > 0 ? 2 : 0;
	size_t arg_sz = strlen(cmd_arg);
	size_t nsize = opts->kcmd_sz + arg_sz + pad;
	opts->kcmd = realloc(opts->kcmd, nsize * sizeof (char));
	if (opts->kcmd_sz > 0) {
		opts->kcmd[opts->kcmd_sz] = ' ';
		opts->kcmd[opts->kcmd_sz + 1] = '\0';
	}
	opts->kcmd = strncat(opts->kcmd, cmd_arg, arg_sz);
	opts->kcmd_sz = nsize;
}

static void dump_options(struct cmd_opts *opts)
{
	printf("\n");
	printf("--initrd Initrd: %s\n", opts->initrd);
	printf("-m       RAM memory: %d\n", opts->ram);
	printf("-h       help: %d\n", opts->help);
	printf("BzImage: %s\n", opts->img);
	printf("Command line: %s\n", opts->kcmd);
	printf("\n");
}

void parse_command_line(int argc, char *argv[], struct cmd_opts *opts)
{
	memset(opts, '\0', sizeof(struct cmd_opts));
	opts->kcmd_sz = 0;
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
				err(1, "Option index %d\n", opt_ind);
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
			err(1, "Character code %d\n", c);
			break;
		}
	}

	while (optind < argc) {
		int image = 1;
		if (is_cmd(argv[optind])) {
			add_kmd(opts, argv[optind]);
			image = 0;
		}

		if (image)
			opts->img = argv[optind];
		optind++;
	}
	if (opts->help)
		dump_options(opts);
}
