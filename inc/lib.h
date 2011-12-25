// Main public header file for our user-land support library,
// whose code lives in the lib directory.
// This library is roughly our OS's version of a standard C library,
// and is intended to be linked into all user-mode applications
// (NOT the kernel or boot loader).

#ifndef JOS_INC_LIB_H
#define JOS_INC_LIB_H 1

#include <inc/types.h>
#include <inc/stdio.h>
#include <inc/stdarg.h>
#include <inc/string.h>
#include <inc/error.h>
#include <inc/assert.h>
#include <inc/env.h>
#include <inc/memlayout.h>
#include <inc/syscall.h>
#include <inc/trap.h>
#include <inc/fs.h>
#include <inc/fd.h>
#include <inc/args.h>

#define USED(x)		(void)(x)

// libos.c or entry.S
extern char *binaryname;
extern volatile struct Env *env;
extern volatile struct Env envs[NENV];
extern volatile struct Page pages[];
void	exit(void);

// pgfault.c
void	set_pgfault_handler(void (*handler)(struct UTrapframe *utf));

// readline.c
char*	readline(const char *buf);

// syscall.c
void	sys_cputs(const char *string, size_t len);
int	sys_cgetc(void);
envid_t	sys_getenvid(void);
int	sys_env_destroy(envid_t);
void	sys_yield(void);
static envid_t sys_exofork(void);
int	sys_env_set_status(envid_t env, int status);
int	sys_page_alloc(envid_t env, void *pg, int perm);
int	sys_page_map(envid_t src_env, void *src_pg,
		     envid_t dst_env, void *dst_pg, int perm);
int	sys_page_unmap(envid_t env, void *pg);

// This must be inlined.  Exercise for reader: why?
static __inline envid_t sys_exofork(void) __attribute__((always_inline));
static __inline envid_t
sys_exofork(void)
{
	envid_t ret;
	__asm __volatile("int %2"
		: "=a" (ret)
		: "a" (SYS_exofork),
		  "i" (T_SYSCALL)
	);
	return ret;
}

int	sys_env_set_pgfault_upcall(envid_t env, void *upcall);
int	sys_ipc_try_send(envid_t to_env, uint32_t value, void *pg, int perm);
int	sys_ipc_recv(void *rcv_pg);
int	sys_program_lookup(const char *name, size_t len);
ssize_t	sys_program_size(int programid);
int	sys_program_page_map(envid_t dst_env, int programid,
			     size_t offset, void *pg, int pg_perm);
int	sys_env_set_trapframe(envid_t env, struct Trapframe *tf);

// ipc.c
void	ipc_send(envid_t to_env, uint32_t value, void *pg, int perm);
uint32_t ipc_recv(envid_t *from_env, void *pg, int *perm);

// fork.c
#define	PTE_SHARE	0x400
envid_t	fork(void);
envid_t	sfork(void);	// Challenge!

// spawn.c
envid_t	spawn(const char *program, const char **argv);
envid_t	spawnl(const char *program, const char *arg0, ...);

// fd.c
int	close(int fd);
ssize_t	read(int fd, void *buf, size_t nbytes);
ssize_t	write(int fd, const void *buf, size_t nbytes);
int	seek(int fd, off_t offset);
void	close_all(void);
ssize_t	readn(int fd, void *buf, size_t nbytes);
int	dup(int oldfd, int newfd);
int	fstat(int fd, struct Stat *statbuf);
int	stat(const char *path, struct Stat *statbuf);

// file.c
int	open(const char *path, int mode);
int	read_map(int fd, off_t offset, void **blk);
int	ftruncate(int fd, off_t size);
int	remove(const char *path);
int	sync(void);

// fsipc.c
int	fsipc_open(const char *path, int omode, struct Fd *fd);
int	fsipc_map(int fileid, off_t offset, void *dst_va);
int	fsipc_set_size(int fileid, off_t size);
int	fsipc_close(int fileid);
int	fsipc_dirty(int fileid, off_t offset);
int	fsipc_remove(const char *path);
int	fsipc_sync(void);

// pageref.c
int	pageref(void *addr);

// console.c
void	cputchar(int c);
int	getchar(void);
int	iscons(int fd);
int	opencons(void);

// pipe.c
int	pipe(int pipefds[2]);
int	pipeisclosed(int pipefd);

// wait.c
void	wait(envid_t env);

/* File open modes */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

#define	O_CREAT		0x0100		/* create if nonexistent */
#define	O_TRUNC		0x0200		/* truncate to zero length */
#define	O_EXCL		0x0400		/* error if already exists */
#define O_MKDIR		0x0800		/* create directory, not regular file */

#endif /* !JOS_INC_LIB_H */
