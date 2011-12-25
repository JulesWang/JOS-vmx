/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_PROGRAMS_H
#define JOS_KERN_PROGRAMS_H
#include <inc/types.h>
#include <inc/memlayout.h>

#define PROGRAM_MAXPAGES 32

struct Program {
	const char *name;
	const uint8_t *data;
	size_t size;
	struct Page *pages[PROGRAM_MAXPAGES];
};

extern struct Program programs[];
extern int nprograms;

#endif	// !JOS_KERN_PROGRAMS_H
