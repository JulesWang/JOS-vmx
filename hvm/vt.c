/*
 * Copyright (c) 2007, 2008 University of Tsukuba
 * Copyright (c) 2012 Shanghai JiaoTong University, School of Software, TC group
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Tsukuba nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <inc/hvm/vt.h>
#include <inc/x86.h>
#include <kern/console.h>

struct vt_vmentry_regs vr;

enum vt_status {
	VT_VMENTRY_SUCCESS,
	VT_VMENTRY_FAILED,
	VT_VMEXIT,
};

static void
do_ept_violation (void)
{
	ulong error_code;
	ulong guest_linear_addr;
	ulong guest_physical_addr;
	ulong guest_physical_addr_high;
	asm_vmread(VMCS_EXIT_QUALIFICATION, &error_code);
	asm_vmread(VMCS_GUEST_LINEAR_ADDR, &guest_linear_addr);
	asm_vmread(VMCS_GUEST_PHYSICAL_ADDR, &guest_physical_addr);
	asm_vmread(VMCS_GUEST_PHYSICAL_ADDR_HIGH, &guest_physical_addr_high);
	cprintf("EPT_Violation Error code: %#08lx\n", error_code);
	cprintf("EPT_Violation guest_linear_addr: %#08lx\n", guest_linear_addr);
	cprintf("EPT_Violation guest_physical_addr: %#08lx\n", guest_physical_addr);
	cprintf("EPT_Violation guest_physical_addr_high: %#08lx\n", guest_physical_addr_high);
	panic("EPT_Violation");
}

static void
do_ept_misconfig (void)
{
	ulong error_code;
	ulong guest_linear_addr;
	ulong guest_physical_addr;
	asm_vmread(VMCS_EXIT_QUALIFICATION, &error_code);
	asm_vmread(VMCS_GUEST_LINEAR_ADDR, &guest_linear_addr);
	asm_vmread(VMCS_GUEST_PHYSICAL_ADDR, &guest_physical_addr);
	cprintf("EPT_Misconfig Error code: %#08lx\n", error_code);
	cprintf("EPT_Misconfig guest_linear_addr: %#08lx\n", guest_linear_addr);
	cprintf("EPT_Misconfig guest_physical_addr: %#08lx\n", guest_physical_addr);
	panic("EPT_Misconfig");
}

static void
add_ip (void)
{
	ulong ip, len;

	asm_vmread (VMCS_GUEST_RIP, &ip);
	asm_vmread (VMCS_VMEXIT_INSTRUCTION_LEN, &len);
	ip += len;
	asm_vmwrite (VMCS_GUEST_RIP, ip);
}

static void
do_cpuid (void)
{
	u32 ia, oa, ob, oc, od;
	ulong la;

	vt_read_general_reg(GENERAL_REG_RAX, &la);

	/* Ex5: The CODE */
	/* Assign a speical 'code' to RAX, like 0xDeadBeef. */
	/* So when you execute cpuid with the value of RAX  */
	/* equals to '0xFeedCafe', you will get the respond.*/
	/* The respond indicates that you are in the Matrix.*/
	/* Hint: vt_write_general_reg(), add_ip()           */
	if (la == 0xFeedCafe) {
		/* Add your code here. */
	}

	ia = la;
	cpuid(ia, &oa, &ob, &oc, &od);

	/* remove vmx support in Matrix */
	if (ia == CPUID_1)
		oc &= ~CPUID_1_ECX_VMX_BIT;

	vt_write_general_reg(GENERAL_REG_RAX, oa);
	vt_write_general_reg(GENERAL_REG_RBX, ob);
	vt_write_general_reg(GENERAL_REG_RCX, oc);
	vt_write_general_reg(GENERAL_REG_RDX, od);

	add_ip();
}

static bool
vt_exit_reason (void)
{
	ulong exit_reason = 0;
	ulong ip = 0;

	asm_vmread (VMCS_EXIT_REASON, &exit_reason);
	asm_vmread (VMCS_GUEST_RIP, &ip);


	if (exit_reason & EXIT_REASON_VMENTRY_FAILURE_BIT) {
		ulong error_code;
		asm_vmread(VMCS_EXIT_QUALIFICATION, &error_code);
		panic("VMEntry failure(EXIT_REASON, EXIT_QUALIFICATION): %lx, %lx\n", exit_reason,error_code);
		return false;
	}

	switch (exit_reason & EXIT_REASON_MASK) {
		case EXIT_REASON_CPUID:
			do_cpuid();
			break;
		case EXIT_REASON_EPT_VIOLATION:
			do_ept_violation();
			break;
		case EXIT_REASON_EPT_MISCONFIG:
			do_ept_misconfig();
			break;
		case EXIT_REASON_INIT_SIGNAL:
			return false;
		default:
			asm_vmread (VMCS_GUEST_RIP, &ip);
			panic("Fatal error: handler not implemented. code:%ld, ip:%#lx",
				   exit_reason, ip);
	}
	return true;
}

enum vt_status
vt_vmlaunch (void)
{
	if (asm_vmlaunch_regs(&vr))
		return VT_VMENTRY_FAILED;
	return VT_VMEXIT;
}

static enum vt_status
vt_vmresume (void)
{
	if (asm_vmresume_regs (&vr))
		return VT_VMENTRY_FAILED;
	return VT_VMEXIT;
}

static void
vt_first_run(void)
{
	enum vt_status status;
	ulong errcode;

	status = vt_vmlaunch ();
	if (status != VT_VMEXIT) {
		if (status == VT_VMENTRY_FAILED)
		{
			asm_vmread(VMCS_VM_INSTRUCTION_ERR, &errcode);
			panic ("Fatal error: VM launch failed. Error Code: %ld", errcode);
		}
		else
			panic ("Fatal error: Strange status.");
	}
}

static void
vt_run (void)
{
	enum vt_status status;

	status = vt_vmresume();
	if (status != VT_VMEXIT) {
		if (status == VT_VMENTRY_FAILED)
			panic ("Fatal error: VM resume failed.");
		else
			panic ("Fatal error: Strange status.");
	}
}

void
vt_main(void)
{
	vt_init();
	cprintf("Start VM...\n");

	vt_first_run();
	cprintf("VM launch Success\n");

	while(vt_exit_reason())
	{
		vt_run ();
	}
	get_cursor_loc();
	cprintf("VM Stopped\n");
}
