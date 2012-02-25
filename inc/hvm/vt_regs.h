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

#ifndef JOS_VT_REGS_H
#define JOS_VT_REGS_H

#include <inc/hvm/vt.h>

enum general_reg {
	GENERAL_REG_RAX = 0,
	GENERAL_REG_RCX = 1,
	GENERAL_REG_RDX = 2,
	GENERAL_REG_RBX = 3,
	GENERAL_REG_RSP = 4,
	GENERAL_REG_RBP = 5,
	GENERAL_REG_RSI = 6,
	GENERAL_REG_RDI = 7,
	GENERAL_REG_R8 = 8,
	GENERAL_REG_R9 = 9,
	GENERAL_REG_R10 = 10,
	GENERAL_REG_R11 = 11,
	GENERAL_REG_R12 = 12,
	GENERAL_REG_R13 = 13,
	GENERAL_REG_R14 = 14,
	GENERAL_REG_R15 = 15,
};

struct regs_in_vmcs_sreg {
	u16 sel;
	ulong limit;
	ulong acr;
	ulong base;
};

struct regs_in_vmcs_desc {
	ulong base;
	ulong limit;
};

struct regs_in_vmcs {
	struct regs_in_vmcs_sreg es, cs, ss, ds, fs, gs, ldtr, tr;
	struct regs_in_vmcs_desc gdtr, idtr;
	ulong cr0, cr3, cr4;
	ulong dr7, rflags;
};

void vt_get_current_regs_host(struct regs_in_vmcs *p);
void vt_read_general_reg(enum general_reg reg, ulong *val);
void vt_write_general_reg(enum general_reg reg, ulong val);

#endif
