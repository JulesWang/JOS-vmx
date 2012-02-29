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

#include <inc/hvm/vt_init.h>
#include <inc/hvm/vt.h>
#include <inc/hvm/vt_ept.h>

extern ept_control g_ept_ctl;

#define DEBUG_PRINT(x) cprintf(#x": %#08x\n", x);

bool
has_vmx(void)
{
	uint32_t a, b, c, d;
	uint64_t tmp;

	panic("has_vmx not implemented!");
	/* Ex1: Check whether CPU has vmx support */
	/* hint: Intel Manual 3B: 19.6 DISCOVERING SUPPORT FOR VMX */
	/* hint: cpuid() */

	/* 20.6.2 Processor-Based VM-Execution Controls */
	uint32_t low,high;
	asm_rdmsr32(MSR_IA32_VMX_PROCBASED_CTLS, &low, &high);

	if (high & VMCS_PROC_BASED_VMEXEC_CTL_ACTIVESECCTL_BIT) {
		cprintf("Processor-baesd Secondary Control is supported.\n");
		asm_rdmsr32(MSR_IA32_VMX_PROCBASED_CTLS2, &low, &high);

		if (high & SECONDARY_EXEC_ENABLE_EPT)  
			cprintf("EPT is supported.\n");
		else
			cprintf("EPT is NOT supported!\n");

		if (high & SECONDARY_EXEC_UNRESTRICTED_GUEST)  
			cprintf("Unrestricted Guest is supported.\n");
		else
			cprintf("Unrestricted Guest is NOT supported!\n");
	}
	else {
		cprintf("Processor-baesd Secondary Control is NOT supported!\n");
	}
	return true;
}

/*
 * Enable VMX and do VMXON
*/
static void
vmx_on (void)
{
	ulong cr0_0, cr0_1, cr4_0, cr4_1;
	ulong cr0, cr4;

	/* apply FIXED bits */
	asm_rdmsr(MSR_IA32_VMX_CR0_FIXED0, &cr0_0);
	asm_rdmsr(MSR_IA32_VMX_CR0_FIXED1, &cr0_1);
	asm_rdmsr(MSR_IA32_VMX_CR4_FIXED0, &cr4_0);
	asm_rdmsr(MSR_IA32_VMX_CR4_FIXED1, &cr4_1);

	asm_rdcr0 (&cr0);
	cr0 &= cr0_1;
	cr0 |= cr0_0;
	asm_wrcr0 (cr0);

	asm_rdcr4 (&cr4);
	cr4 &= cr4_1;
	cr4 |= cr4_0;
	asm_wrcr4 (cr4);

	panic("vmx_on not implemented");
	/* Ex2: set VMXE bit of CR4 to enable VMX */
	/* hint: CR4_VMXE_BIT */


	/* Ex2: alloc vmxon region */
	/* hint: 20.10.4 VMXON Region */
	/* hint: page_alloc */


	/* Ex2: write a VMCS revision identifier */
	/* hint: 20.10.4 VMXON Region */
	/* hint: asm_rdmsr32 */


	/* Ex2: execute vmxon */
	/* hint: asm_vmxon() */
}

static void 
set_vmcs_ctl(void)
{
	u32 pinbased_ctls_or, pinbased_ctls_and;
	u32 procbased_ctls_or, procbased_ctls_and;
	u32 procbased_ctls2_or, procbased_ctls2_and;

	u32 procbased_ctls;
	u32 procbased_ctls2;

	u32 exit_ctls_or, exit_ctls_and;
	u32 entry_ctls_or, entry_ctls_and;

	/* get information from MSR */
	asm_rdmsr32 (MSR_IA32_VMX_PINBASED_CTLS,
		     &pinbased_ctls_or, &pinbased_ctls_and);
	asm_rdmsr32 (MSR_IA32_VMX_PROCBASED_CTLS,
		     &procbased_ctls_or, &procbased_ctls_and);
	asm_rdmsr32 (MSR_IA32_VMX_PROCBASED_CTLS2,
		     &procbased_ctls2_or, &procbased_ctls2_and);

	asm_rdmsr32 (MSR_IA32_VMX_EXIT_CTLS,
		     &exit_ctls_or, &exit_ctls_and);
	asm_rdmsr32 (MSR_IA32_VMX_ENTRY_CTLS,
		     &entry_ctls_or, &entry_ctls_and);


	panic("set_vmcs_ctl not implemented");
	/* Ex4: enable EPT and Unrestricted Guest */
	/* hint: 20.6.2 Processor-Based VM-Execution Controls */
	/* hint: procbased_ctls&procbased_ctls(just twe lines code) */

	/* bug fix */
	procbased_ctls_or &= ~(VMCS_PROC_BASED_VMEXEC_CTL_CR3LOADEXIT_BIT | 
			   VMCS_PROC_BASED_VMEXEC_CTL_CR3STOREXIT_BIT);


	/* 64-Bit Control Fields */
	asm_vmwrite (VMCS_ADDR_IOBMP_A, 0xFFFFFFFF);
	asm_vmwrite (VMCS_ADDR_IOBMP_A_HIGH, 0xFFFFFFFF);
	asm_vmwrite (VMCS_ADDR_IOBMP_B, 0xFFFFFFFF);
	asm_vmwrite (VMCS_ADDR_IOBMP_B_HIGH, 0xFFFFFFFF);
	asm_vmwrite (VMCS_VMEXIT_MSRSTORE_ADDR, 0);
	asm_vmwrite (VMCS_VMEXIT_MSRSTORE_ADDR_HIGH, 0);
	asm_vmwrite (VMCS_VMEXIT_MSRLOAD_ADDR, 0);
	asm_vmwrite (VMCS_VMEXIT_MSRLOAD_ADDR_HIGH, 0);
	asm_vmwrite (VMCS_VMENTRY_MSRLOAD_ADDR, 0);
	asm_vmwrite (VMCS_VMENTRY_MSRLOAD_ADDR_HIGH, 0);
	asm_vmwrite (VMCS_EXEC_VMCS_POINTER, 0);
	asm_vmwrite (VMCS_EXEC_VMCS_POINTER_HIGH, 0);
	asm_vmwrite (VMCS_TSC_OFFSET, 0);
	asm_vmwrite (VMCS_TSC_OFFSET_HIGH, 0);
	asm_vmwrite (VMCS_EPT_POINTER,      g_ept_ctl.eptp);
	asm_vmwrite (VMCS_EPT_POINTER_HIGH, g_ept_ctl.eptp >> 32);

	/* 32-Bit Control Fields */
	asm_vmwrite (VMCS_PIN_BASED_VMEXEC_CTL,
		    (pinbased_ctls_or) & pinbased_ctls_and);
	asm_vmwrite (VMCS_PROC_BASED_VMEXEC_CTL,
		    (procbased_ctls | procbased_ctls_or) & procbased_ctls_and);
	asm_vmwrite (VMCS_SECONDARY_VM_EXEC_CONTROL,
		    (procbased_ctls2 | procbased_ctls2_or) & procbased_ctls2_and);

	//asm_vmwrite (VMCS_EXCEPTION_BMP, 0xFFFFFFFF);
	asm_vmwrite (VMCS_EXCEPTION_BMP, 0x0);
	asm_vmwrite (VMCS_PAGEFAULT_ERRCODE_MASK, 0);
	asm_vmwrite (VMCS_PAGEFAULT_ERRCODE_MATCH, 0xFFFFFFFF);
	asm_vmwrite (VMCS_CR3_TARGET_COUNT, 1);

	asm_vmwrite (VMCS_VMEXIT_CTL, exit_ctls_or & exit_ctls_and);
	asm_vmwrite (VMCS_VMEXIT_MSR_STORE_COUNT, 0);
	asm_vmwrite (VMCS_VMEXIT_MSR_LOAD_COUNT, 0);
	asm_vmwrite (VMCS_VMENTRY_CTL, entry_ctls_or & entry_ctls_and);
	asm_vmwrite (VMCS_VMENTRY_MSR_LOAD_COUNT, 0);
	asm_vmwrite (VMCS_VMENTRY_INTR_INFO_FIELD, 0);
	asm_vmwrite (VMCS_VMENTRY_EXCEPTION_ERRCODE, 0);
	asm_vmwrite (VMCS_VMENTRY_INSTRUCTION_LEN, 0);
	asm_vmwrite (VMCS_TPR_THRESHOLD, 0);

	/* Natural-Width Control Fields */
	asm_vmwrite (VMCS_CR0_GUESTHOST_MASK, 0);
	asm_vmwrite (VMCS_CR4_GUESTHOST_MASK, 0);
	asm_vmwrite (VMCS_CR0_READ_SHADOW, 0);
	asm_vmwrite (VMCS_CR4_READ_SHADOW, 0);
	asm_vmwrite (VMCS_CR3_TARGET_VALUE_0, 0);
	asm_vmwrite (VMCS_CR3_TARGET_VALUE_1, 0);
	asm_vmwrite (VMCS_CR3_TARGET_VALUE_2, 0);
	asm_vmwrite (VMCS_CR3_TARGET_VALUE_3, 0);
}

static void 
set_vmcs_host_state(void)
{
	struct regs_in_vmcs host_riv;

	vt_get_current_regs_host (&host_riv);

	/* 16-Bit Host-State Fields */
	asm_vmwrite (VMCS_HOST_ES_SEL, host_riv.es.sel);
	asm_vmwrite (VMCS_HOST_CS_SEL, host_riv.cs.sel);
	asm_vmwrite (VMCS_HOST_SS_SEL, host_riv.ss.sel);
	asm_vmwrite (VMCS_HOST_DS_SEL, host_riv.ds.sel);
	asm_vmwrite (VMCS_HOST_FS_SEL, host_riv.fs.sel);
	asm_vmwrite (VMCS_HOST_GS_SEL, host_riv.gs.sel);
	asm_vmwrite (VMCS_HOST_TR_SEL, host_riv.tr.sel);

	/* 64-Bit Host-State Fields */

	/* 32-Bit Host-State Field */

	/* Natural-Width Host-State Fields */
	asm_vmwrite (VMCS_HOST_CR0, host_riv.cr0);
	asm_vmwrite (VMCS_HOST_CR3, host_riv.cr3);
	asm_vmwrite (VMCS_HOST_CR4, host_riv.cr4);
	asm_vmwrite (VMCS_HOST_FS_BASE, host_riv.fs.base);
	asm_vmwrite (VMCS_HOST_GS_BASE, host_riv.gs.base);
	asm_vmwrite (VMCS_HOST_TR_BASE, host_riv.tr.base);
	asm_vmwrite (VMCS_HOST_GDTR_BASE, host_riv.gdtr.base);
	asm_vmwrite (VMCS_HOST_IDTR_BASE, host_riv.idtr.base);

	asm_vmwrite (VMCS_HOST_RSP, 0xDEADBEEF);
	asm_vmwrite (VMCS_HOST_RIP, 0xDEADBEEF);
}

static void 
set_vmcs_guest_state()
{
	/* 16-Bit Guest-State Fields */
	asm_vmwrite (VMCS_GUEST_ES_SEL, 0);
	asm_vmwrite (VMCS_GUEST_CS_SEL, 0);
	asm_vmwrite (VMCS_GUEST_SS_SEL, 0);
	asm_vmwrite (VMCS_GUEST_DS_SEL, 0);
	asm_vmwrite (VMCS_GUEST_FS_SEL, 0);
	asm_vmwrite (VMCS_GUEST_GS_SEL, 0);
	asm_vmwrite (VMCS_GUEST_LDTR_SEL, 0);
	asm_vmwrite (VMCS_GUEST_TR_SEL, 0);

	/* 64-Bit Guest-State Fields */
	asm_vmwrite (VMCS_VMCS_LINK_POINTER, 0xFFFFFFFF);
	asm_vmwrite (VMCS_VMCS_LINK_POINTER_HIGH, 0xFFFFFFFF);
	asm_vmwrite (VMCS_GUEST_IA32_DEBUGCTL, 0);
	asm_vmwrite (VMCS_GUEST_IA32_DEBUGCTL_HIGH, 0);

	/* 32-Bit Guest-State Fields */
	asm_vmwrite (VMCS_GUEST_ES_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_CS_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_SS_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_DS_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_FS_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_GS_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_LDTR_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_TR_LIMIT, 0x0000ffff);
	asm_vmwrite (VMCS_GUEST_GDTR_LIMIT, 0x30);
	asm_vmwrite (VMCS_GUEST_IDTR_LIMIT, 0x3ff);
	asm_vmwrite (VMCS_GUEST_ES_ACCESS_RIGHTS, 0x93);
	asm_vmwrite (VMCS_GUEST_CS_ACCESS_RIGHTS, 0x93);
	asm_vmwrite (VMCS_GUEST_SS_ACCESS_RIGHTS, 0x93);
	asm_vmwrite (VMCS_GUEST_DS_ACCESS_RIGHTS, 0x93);
	asm_vmwrite (VMCS_GUEST_FS_ACCESS_RIGHTS, 0x93);
	asm_vmwrite (VMCS_GUEST_GS_ACCESS_RIGHTS, 0x93);
	asm_vmwrite (VMCS_GUEST_LDTR_ACCESS_RIGHTS, 0x82);
	asm_vmwrite (VMCS_GUEST_TR_ACCESS_RIGHTS, 0x8b);
	asm_vmwrite (VMCS_GUEST_INTERRUPTIBILITY_STATE, 0);
	asm_vmwrite (VMCS_GUEST_ACTIVITY_STATE, 0);

	/* Natural-Width Guest-State Fields */
	asm_vmwrite (VMCS_GUEST_CR0, CR0_NE_BIT); 
	asm_vmwrite (VMCS_GUEST_CR3, 0);
	asm_vmwrite (VMCS_GUEST_CR4, CR4_VMXE_BIT); /* clear this bit will result in vm entry failure. */
	asm_vmwrite (VMCS_GUEST_ES_BASE, 0);
	asm_vmwrite (VMCS_GUEST_CS_BASE, 0);
	asm_vmwrite (VMCS_GUEST_SS_BASE, 0);
	asm_vmwrite (VMCS_GUEST_DS_BASE, 0);
	asm_vmwrite (VMCS_GUEST_FS_BASE, 0);
	asm_vmwrite (VMCS_GUEST_GS_BASE, 0);
	asm_vmwrite (VMCS_GUEST_LDTR_BASE, 0);
	asm_vmwrite (VMCS_GUEST_TR_BASE, 0);
	asm_vmwrite (VMCS_GUEST_GDTR_BASE, 0);
	asm_vmwrite (VMCS_GUEST_IDTR_BASE, 0);
	asm_vmwrite (VMCS_GUEST_DR7, 0);
	asm_vmwrite (VMCS_GUEST_RSP, 0);
	asm_vmwrite (VMCS_GUEST_RIP, 0x7c00);
	asm_vmwrite (VMCS_GUEST_RFLAGS, RFLAGS_ALWAYS1_BIT);
	asm_vmwrite (VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, 0);
}

static void
vmcs_setup(void)
{
	panic("vmcs_setup not implemented");
	/* Ex4: alloc VMCS region */
	/* hint: refer to vmx_on() */

	/* Ex4: write a VMCS revision identifier */
	/* hint: 20.2 FORMAT OF THE VMCS REGION */
	/* hint: refer to vmx_on() */


	/* Ex4: Load VMCS pointer */
	/* hint: asm_vmclear() and asm_vmptrld() */


	/* initialize VMCS fields */
	set_vmcs_ctl();
	set_vmcs_host_state();
	set_vmcs_guest_state(); 
}

void
vt_init (void)
{
	vmx_on();
	ept_setup();
	vmcs_setup();
}
