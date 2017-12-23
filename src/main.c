#include "cmd.h"
#include "kvm.h"

int main(int argc, char **argv)
{
	struct cmd_opts opts;
	parse_command_line(argc, argv, &opts);
	kvm_run(&opts);
	return 0;
}
