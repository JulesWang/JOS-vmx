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

#include <inc/hvm/vt_regs.h>

extern struct vt_vmentry_regs vr;

void
get_seg_base (ulong gdtbase, u16 ldtr, u16 sel, ulong *segbase)
{
	ulong ldtbase;
	struct segdesc *p;

	if (sel == 0) {
		*segbase = 0;
	} else if (sel & SEL_LDT_BIT) {
		p = (struct segdesc *)(gdtbase + (ldtr & SEL_MASK));
		if (!p->p)
			goto err;
		ldtbase = SEGDESC_BASE (*p);
		p = (struct segdesc *)(ldtbase + (sel & SEL_MASK));
		if (!p->p)
			goto err;
		*segbase = SEGDESC_BASE (*p);
	} else {
		p = (struct segdesc *)(gdtbase + (sel & SEL_MASK));
		if (!p->p)
			goto err;
		*segbase = SEGDESC_BASE (*p);
	}
	return;
err:
	cprintf ("ERROR: get_seg_base failed!\n");
	*segbase = 0;
}

u32
get_seg_access_rights (u16 sel)
{
	ulong tmp, ret;

	if (sel) {
		asm_lar (sel, &tmp);
		ret = (tmp >> 8) & ACCESS_RIGHTS_MASK;
	} else {
		ret = ACCESS_RIGHTS_UNUSABLE_BIT;
	}
	return ret;
}

void
vt_get_current_regs_host (struct regs_in_vmcs *p)
{
	asm_rdes (&p->es.sel);
	asm_rdcs (&p->cs.sel);
	asm_rdss (&p->ss.sel);
	asm_rdds (&p->ds.sel);
	asm_rdfs (&p->fs.sel);
	asm_rdgs (&p->gs.sel);
	asm_rdldtr (&p->ldtr.sel);
	asm_rdtr (&p->tr.sel);
	asm_rdcr0 (&p->cr0);
	asm_rdcr3 (&p->cr3);
	asm_rdcr4 (&p->cr4);
	asm_lsl (p->es.sel, &p->es.limit);
	asm_lsl (p->cs.sel, &p->cs.limit);
	asm_lsl (p->ss.sel, &p->ss.limit);
	asm_lsl (p->ds.sel, &p->ds.limit);
	asm_lsl (p->fs.sel, &p->fs.limit);
	asm_lsl (p->gs.sel, &p->gs.limit);
	asm_lsl (p->ldtr.sel, &p->ldtr.limit);
	asm_lsl (p->tr.sel, &p->tr.limit);
	p->es.acr = get_seg_access_rights (p->es.sel);
	p->cs.acr = get_seg_access_rights (p->cs.sel);
	p->ss.acr = get_seg_access_rights (p->ss.sel);
	p->ds.acr = get_seg_access_rights (p->ds.sel);
	p->fs.acr = get_seg_access_rights (p->fs.sel);
	p->gs.acr = get_seg_access_rights (p->gs.sel);
	p->ldtr.acr = get_seg_access_rights (p->ldtr.sel);
	p->tr.acr = get_seg_access_rights (p->tr.sel);
	asm_rdgdtr (&p->gdtr.base, &p->gdtr.limit);
	asm_rdidtr (&p->idtr.base, &p->idtr.limit);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->es.sel, &p->es.base);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->cs.sel, &p->cs.base);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->ss.sel, &p->ss.base);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->ds.sel, &p->ds.base);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->fs.sel, &p->fs.base);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->gs.sel, &p->gs.base);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->ldtr.sel, &p->ldtr.base);
	get_seg_base (p->gdtr.base, p->ldtr.sel, p->tr.sel, &p->tr.base);
	asm_rddr7 (&p->dr7);
	asm_rdrflags (&p->rflags);
}

void
vt_read_general_reg (enum general_reg reg, ulong *val)
{
	switch(reg) {
	case GENERAL_REG_RAX:
		*val = vr.rax;
		break;
	case GENERAL_REG_RCX:
		*val = vr.rcx;
		break;
	case GENERAL_REG_RDX:
		*val = vr.rdx;
		break;
	case GENERAL_REG_RBX:
		*val = vr.rbx;
		break;
	case GENERAL_REG_RSP:
		asm_vmread (VMCS_GUEST_RSP, val);
		break;
	case GENERAL_REG_RBP:
		*val = vr.rbp;
		break;
	case GENERAL_REG_RSI:
		*val = vr.rsi;
		break;
	case GENERAL_REG_RDI:
		*val = vr.rdi;
		break;
	default:
		panic ("Fatal error: unknown register.");
	}
}

void
vt_write_general_reg (enum general_reg reg, ulong val)
{
	switch(reg) {
	case GENERAL_REG_RAX:
		vr.rax = val;
		break;
	case GENERAL_REG_RCX:
		vr.rcx = val;
		break;
	case GENERAL_REG_RDX:
		vr.rdx = val;
		break;
	case GENERAL_REG_RBX:
		vr.rbx = val;
		break;
	case GENERAL_REG_RSP:
		asm_vmwrite (VMCS_GUEST_RSP, val);
		break;
	case GENERAL_REG_RBP:
		vr.rbp = val;
		break;
	case GENERAL_REG_RSI:
		vr.rsi = val;
		break;
	case GENERAL_REG_RDI:
		vr.rdi = val;
		break;
	default:
		panic ("Fatal error: unknown register.");
	}
}
