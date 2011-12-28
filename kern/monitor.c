// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display back trace information of function", mon_backtrace },
    { "exit", "Exit from trap monitor", mon_exit },
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))



/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start %08x (virt)  %08x (phys)\n", _start, _start - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		(end-_start+1023)/1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
    // LAB1: Your code here.
    uintptr_t *ebp = 0;
    uintptr_t *eip = 0;

    struct Eipdebuginfo info;

    int i = 0;
    int index = 0;

    ebp = (uintptr_t *)read_ebp();

    /* =stack=
       |ebp|
       |eip|
       |param|
       |param|
       |param|
       |param|
       |param|
       |param|
       |ebp|
       ....
     */
    cprintf("Stack backtrace:\n");

    while (ebp != NULL) {
        eip = (uintptr_t *)*(ebp + 1);

        debuginfo_eip((uintptr_t)eip, &info);

        cprintf("%d: ebp %08x  eip %08x  args %08x %08x %08x %08x\n",
                index++, ebp, eip, *(ebp+2), *(ebp+3), *(ebp+4), *(ebp+5));

        cprintf("%s:%d: " , info.eip_file , info.eip_line);

        for( i=0 ; i<info.eip_fn_namelen ; i++)
            cprintf("%c" , info.eip_fn_name[i]);

        cprintf("+%x ", (uintptr_t)(eip) - info.eip_fn_addr);
        cprintf("(%d arg)\n", info.eip_fn_narg);

        ebp = (uintptr_t *)*(ebp); 
    }

    return 0;
}

int
mon_exit(int argc, char **argv, struct Trapframe *tf)
{
    if (tf == NULL)
        return 0;

    if (tf->tf_trapno == T_BRKPT)
        return -1;

    return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
