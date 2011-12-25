/* See COPYRIGHT for copyright information. */

#ifndef JOS_INC_ENV_H
#define JOS_INC_ENV_H

#include <inc/types.h>
#include <inc/queue.h>
#include <inc/trap.h>
#include <inc/memlayout.h>

typedef int32_t envid_t;

// An environment ID 'envid_t' has three parts:
//
// +1+---------------21-----------------+--------10--------+
// |0|          Uniqueifier             |   Environment    |
// | |                                  |      Index       |
// +------------------------------------+------------------+
//                                       \--- ENVX(eid) --/
//
// The environment index ENVX(eid) equals the environment's offset in the
// 'envs[]' array.  The uniqueifier distinguishes environments that were
// created at different times, but share the same environment index.
//
// All real environments are greater than 0 (so the sign bit is zero).
// envid_ts less than 0 signify errors.  The envid_t == 0 is special, and
// stands for the current environment.

#define LOG2NENV		10
#define NENV			(1 << LOG2NENV)
#define ENVX(envid)		((envid) & (NENV - 1))

// Values of env_status in struct Env
#define ENV_FREE		0
#define ENV_RUNNABLE		1
#define ENV_NOT_RUNNABLE	2

struct Env {
	envid_t env_id;			// Unique environment identifier
	envid_t env_parent_id;		// env_id of this env's parent
	unsigned env_status;		// Status of the environment

        pde_t *env_pgdir;               // Address space page directory
                                        // (kernel virtual address)
	struct Trapframe env_tf;	// Saved registers
	uint32_t env_runs;		// Number of times environment has run
	
	LIST_ENTRY(Env) env_link;	// Free list link pointers

	// Exception handling
	void *env_pgfault_upcall;	// page fault upcall entry point

	// Lab 4 IPC
	bool env_ipc_recving;		// env is blocked receiving
	void *env_ipc_dstva;		// va at which to map received page
	uint32_t env_ipc_value;		// data value sent to us 
	envid_t env_ipc_from;		// envid of the sender	
	int env_ipc_perm;		// perm of page mapping received
};

#endif // !JOS_INC_ENV_H
