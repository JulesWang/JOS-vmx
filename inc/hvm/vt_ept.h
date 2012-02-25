/*
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

#ifndef JOS_VT_EPT_H
#define JOS_vT_EPT_H

#include <inc/types.h>
#include <inc/hvm/constants.h>

/*
 * xen-4.0.1/xen/include/asm-x86/hvm/vmx/vmx.h
 * xen-4.0.1/xen/include/asm-x86/hvm/vmx/vmcs.h
 * xen-4.0.1/xen/arch/x86/mm/hap/p2m-ept.c
 */
typedef u32 gfn_t;

typedef union {
    struct {
        u64 r       :   1,
        w           :   1,
        x           :   1,
        emt         :   3, /* EPT Memory type */
        ipat        :   1, /* Ignore PAT memory type */
        sp          :   1, /* Is this a superpage? */
        avail1      :   4,
        mfn         :   40,
        avail2      :   12;
    };
    u64 epte;
} ept_entry_t;

typedef union {
    struct {
    u64 ept_mt :3,
        ept_wl :3,
        rsvd   :6,
        asr    :52;
    };
    u64 eptp;
} ept_control;

#define GFN(addr) (gfn_t)((addr)>>PAGESIZE_SHIFT)

#define P2M_READABLE 0x01
#define P2M_WRITABLE 0x02
#define P2M_EXECUTABLE 0x04
#define P2M_FULL_ACCESS (P2M_READABLE | P2M_WRITABLE | P2M_EXECUTABLE)

#define P2M_UPDATE_MFN 0x01
#define P2M_UPDATE_REMAININGS 0x02
#define P2M_UPDATE_MT 0x04
#define P2M_UPDATE_ALL 0x07

#define MTRR_TYPE_WRBACK        6
#define MTRR_TYPE_UNCACHE        0
#define EPT_DEFAULT_MT          MTRR_TYPE_WRBACK
#define EPT_DEFAULT_WL          3

#define EPT_TABLE_ORDER         9
#define EPTE_SUPER_PAGE_MASK    0x80
#define EPTE_MFN_MASK           0xffffffffff000ULL
#define EPTE_AVAIL1_MASK        0xF00
#define EPTE_EMT_MASK           0x38
#define EPTE_IGMT_MASK          0x40
#define EPTE_AVAIL1_SHIFT       8
#define EPTE_EMT_SHIFT          3
#define EPTE_IGMT_SHIFT         6

#define EPT_EACHTABLE_ENTRIES       512

void ept_setup();

#endif
