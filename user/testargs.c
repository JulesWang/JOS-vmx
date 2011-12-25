#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int i;
	struct Argstate args;

	argstart(&argc, argv, &args);
	while ((i = argnext(&args)) >= 0)
		switch (i) {
		case 'r':
		case 'x':
			cprintf("'-%c' flag\n", i);
			break;
		case 'f':
			cprintf("'-f %s' flag\n", argvalue(&args));
			break;
		default:
			cprintf("unknown flag\n");
		}

	for (i = 1; i < argc; i++)
		cprintf("argument '%s'\n", argv[i]);
}
