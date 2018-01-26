#include <err.h>
#include "cmd.h"
#include "kvm.h"

int main(int argc, char **argv)
{
	struct cmd_opts opts;
	parse_command_line(argc, argv, &opts);
	if (!opts.img)
		err(1, "No kernel image");
	if (!opts.ram)
		err(1, "Please specify ram");
	kvm_run(&opts);
	return 0;
}
