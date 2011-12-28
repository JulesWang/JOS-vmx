#include <inc/stdio.h>
#include <inc/error.h>
#include <inc/string.h>

#define BUFLEN 1024
static char buf[BUFLEN];

char readline_hack;

char *
readline(const char *prompt)
{
	int i, c, echoing;

	if (readline_hack) {
		strcpy(buf, "exit");
		return buf;
	}

	if (prompt != NULL)
		cprintf("%s", prompt);

	i = 0;
	echoing = iscons(0);
	while (1) {
		c = getchar();
		if (c < 0) {
			cprintf("read error: %e\n", c);
			return NULL;
		} else if (c >= ' ' && i < BUFLEN-1) {
			if (echoing)
				cputchar(c);
			buf[i++] = c;
		} else if (c == '\b' && i > 0) {
			if (echoing)
				cputchar(c);
			i--;
		} else if (c == '\n' || c == '\r') {
			if (echoing)
				cputchar(c);
			buf[i] = 0;
			return buf;
		}
	}
}

