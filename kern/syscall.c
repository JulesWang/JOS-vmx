/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>
#include <kern/programs.h>


/**********************************
 * SYSTEM CALLS FOR LAB 3, PART 2 *
 *                                *
 **********************************/

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.
	
	// LAB 3: Your code here. (Exercise 7)

	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console.
// Returns the character.
static int
sys_cgetc(void)
{
	int c;

	// The cons_getc() primitive doesn't wait for a character,
	// but the sys_cgetc() system call does.
	while ((c = cons_getc()) == 0)
		/* do nothing */;

	return c;
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	if (e == curenv)
		cprintf("[%08x] exiting gracefully\n", curenv->env_id);
	else
		cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}



/********************************************************
 * SYSTEM CALLS FOR LAB 3, PART 3                       *
 *                                                      *
 * No need to implement these until you get to Part 3.  *
 *                                                      *
 ********************************************************/

// Deschedule current environment and pick a different one to run.
// The system call returns 0.
static void
sys_yield(void)
{
	sched_yield();
}

// Lab 3 Exercise 10:
// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.
	//
	// Hint: Your code in env_run() shows how to copy a register set.
	
	// LAB 3: Your code here.

	panic("sys_exofork not implemented");
}

// Lab 3 Exercise 10:
// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
  	// Hint: Use the 'envid2env' function from kern/env.c to translate an
  	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.
	
	// LAB 3: Your code here.
	panic("sys_env_set_status not implemented");
}

// Lab 3 Exercise 10:
// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	// LAB 3: Your code here.
	panic("sys_page_alloc not implemented");
}

// Lab 3 Exercise 10:
// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
	     envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 3: Your code here.
	panic("sys_page_map not implemented");
}

// Lab 3 Exercise 10:
// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().
	
	// LAB 3: Your code here.
	panic("sys_page_unmap not implemented");
}


/**********************************
 * SYSTEM CALLS FOR LAB 4, PART 1 *
 *                                *
 **********************************/

// Lab 4 Exercise 1:
// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	// LAB 4: Your code here.
	panic("sys_env_set_pgfault_upcall not implemented");
}


/**********************************
 * SYSTEM CALLS FOR LAB 4, PART 2 *
 *                                *
 **********************************/

// Lab 4 Exercise 8:
// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	// LAB 4: Your code here.
	panic("sys_ipc_recv not implemented");
	return 0;
}

// Lab 4 Exercise 8:
// Try to send 'value' to the target env 'envid'.
// If va != 0, then also send page currently mapped at 'va',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target has not requested IPC with sys_ipc_recv.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again.
//
// If the sender sends a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc doesn't happen unless no errors occur.
//
// Returns 0 on success where no page mapping occurs,
// 1 on success where a page mapping occurs, and < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	// LAB 4: Your code here.
	panic("sys_ipc_try_send not implemented");
}


/**********************************
 * SYSTEM CALLS FOR LAB 4, PART 3 *
 *                                *
 **********************************/

// Look up a program from the kernel's program data collection.
// Return the ID for the program named 'name' (length of name is 'len').
// If no such program exists, returns -E_INVAL.
// All valid program IDs are large positive numbers
// greater than or equal to PROGRAM_OFFSET.
static int
sys_program_lookup(const char *name, size_t len)
{
	int i;
	
	user_mem_assert(curenv, name, len, PTE_U);
	
	for (i = 0; i < nprograms; i++)
		if (strncmp(programs[i].name, name, len) == 0)
			return PROGRAM_OFFSET + i;

	return -E_INVAL;
}

// Return the size of the ELF binary for program 'programid',
// or -E_INVAL if there is no such program.
static ssize_t
sys_program_size(int programid)
{
	programid -= PROGRAM_OFFSET;
	if (programid < 0 || programid >= nprograms)
		return -E_INVAL;
	else
		return programs[programid].size;
}

// Map a page of data from the ELF binary for program 'programid'
// into the address space of environment 'envid'.
// The page is mapped at 'va' with permission 'perm'.
// Its contents equal the program's ELF binary
// starting at binary offset 'offset'.
// Thus, multiple calls to sys_program_page_map can map an entire ELF binary.
// Any bytes past the length of the program's ELF binary are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
// The page may not be mapped as writable: 'perm' must not contain PTE_W.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see sys_page_map)
//	        or contains PTE_W.
//	-E_INVAL if programid does not name a valid kernel binary.
//	-E_INVAL if offset is not page-aligned.
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static ssize_t
sys_program_page_map(envid_t envid, int programid, uint32_t offset,
		     void *va, int perm)
{
	struct Env *env;
	struct Page *pp;
	int retval;
	int pgoff;
	size_t size;
	
	if ((retval = envid2env(envid, &env, 1)) < 0)
		return retval;

	// Check the programid
	programid -= PROGRAM_OFFSET;
	if (programid < 0 || programid >= nprograms)
		return -E_INVAL;

	// Check the offset and destination VA
	if ((offset & (PGSIZE - 1))
	    || PGOFF(va)
	    || va >= (void*) UTOP
	    || (~perm & (PTE_P | PTE_U))
	    || (perm & ~(PTE_P | PTE_U | PTE_AVAIL)))
		return -E_INVAL;

	// Check the page cache
	pgoff = offset >> PGSHIFT;
	if (pgoff < PROGRAM_MAXPAGES && (pp = programs[programid].pages[pgoff])) {
		if ((retval = page_insert(env->env_pgdir, pp, va, perm)) < 0)
			return retval;
		return 0;
	}

	// Allocate a page and map it at va
	if ((retval = page_alloc(&pp)) < 0)
		return retval;
	if ((retval = page_insert(env->env_pgdir, pp, va, perm)) < 0) {
		page_free(pp);
		return retval;
	}
	
	// Copy the relevant portion of the binary
	if (offset < programs[programid].size) {
		size = PGSIZE;
		if (offset + size > programs[programid].size)
			size = programs[programid].size - offset;
		memcpy(page2kva(pp), programs[programid].data + offset, size);
	} else
		size = 0;

	// Erase remaining portion of page
	memset((uint8_t*) page2kva(pp) + size, 0, PGSIZE - size);

	// Cache the page (and don't lose the reference)
	if (pgoff < PROGRAM_MAXPAGES) {
		pp->pp_ref++;
		programs[programid].pages[pgoff] = pp;
	}
	
	// Return the binary size
	return 0;
}


// Set envid's trap frame to 'tf'.
// tf is modified to make sure that user environments always run at code
// protection level 3 (CPL 3) with interrupts enabled.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_trapframe(envid_t envid, struct Trapframe* tf)
{
	int retval;
	struct Env *e;
	struct Trapframe ltf;

	user_mem_assert(curenv, tf, sizeof(struct Trapframe), PTE_U);
	ltf = *tf;
	ltf.tf_eflags |= FL_IF;
	ltf.tf_cs |= 3;

	if ((retval = envid2env(envid, &e, 1)) < 0)
		return retval;
	e->env_tf = ltf;
	return 0;
}


// Dispatches to the correct kernel function, passing the arguments.
uint32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.

	panic("syscall not implemented");
}

