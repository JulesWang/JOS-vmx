// System call stubs.

#include <inc/syscall.h>
#include <inc/lib.h>

static inline uint32_t
syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	uint32_t ret;

	// Generic system call: pass system call number in AX,
	// up to five parameters in DX, CX, BX, DI, SI.
	// Interrupt kernel with T_SYSCALL.
	//
	// The "volatile" tells the assembler not to optimize
	// this instruction away just because we don't use the
	// return value.
	// 
	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations.

	asm volatile("int %1\n"
		: "=a" (ret)
		: "i" (T_SYSCALL),
		  "a" (num),
		  "d" (a1),
		  "c" (a2),
		  "b" (a3),
		  "D" (a4),
		  "S" (a5)
		: "cc", "memory");
	
	return ret;
}

void
sys_cputs(const char *s, size_t len)
{
	syscall(SYS_cputs, (uint32_t) s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0);
}

envid_t
sys_getenvid(void)
{
	 return syscall(SYS_getenvid, 0, 0, 0, 0, 0);
}

int
sys_env_destroy(envid_t envid)
{
	return syscall(SYS_env_destroy, envid, 0, 0, 0, 0);
}

int
sys_env_set_status(envid_t envid, int status)
{
	return syscall(SYS_env_set_status, envid, status, 0, 0, 0);
}

void
sys_yield(void)
{
	syscall(SYS_yield, 0, 0, 0, 0, 0);
}

int
sys_page_alloc(envid_t envid, void *pg, int perm)
{
	return syscall(SYS_page_alloc, envid, (uintptr_t) pg, perm, 0, 0);
}

int
sys_page_map(envid_t srcenv, void *srcpg, envid_t dstenv, void *dstpg, int perm)
{
	return syscall(SYS_page_map, srcenv, (uintptr_t) srcpg,
		       dstenv, (uintptr_t) dstpg, perm);
}

int
sys_page_unmap(envid_t envid, void *pg)
{
	return syscall(SYS_page_unmap, envid, (uintptr_t) pg, 0, 0, 0);
}

// sys_exofork is inlined in lib.h

int
sys_env_set_pgfault_upcall(envid_t envid, void *upcall)
{
	return syscall(SYS_env_set_pgfault_upcall, envid, (uintptr_t) upcall, 0, 0, 0);
}

int
sys_ipc_recv(void *dstva)
{
	return syscall(SYS_ipc_recv, (uintptr_t) dstva, 0, 0, 0, 0);
}

int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, int perm)
{
	return syscall(SYS_ipc_try_send, envid, value, (uintptr_t) srcva, perm, 0);
}

int
sys_program_lookup(const char *name, size_t len)
{
	return syscall(SYS_program_lookup, (uintptr_t) name, len, 0, 0, 0);
}

ssize_t
sys_program_size(int programid)
{
	return syscall(SYS_program_lookup, programid, 0, 0, 0, 0);
}

int
sys_program_page_map(envid_t dst_env, int programid, size_t offset,
		     void *pg, int pg_perm)
{
	return syscall(SYS_program_page_map, dst_env, programid, offset, (uintptr_t) pg, pg_perm);
}

int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	return syscall(SYS_env_set_trapframe, envid, (uintptr_t) tf, 0, 0, 0);
}

