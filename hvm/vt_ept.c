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

#include <inc/hvm/vt_ept.h>
#include <inc/hvm/vt.h>

ept_control g_ept_ctl;

static void ept_create_table(struct Page* ept_pml4_page);
static int ept_update_identity_table ( void* pt, u8 level, gfn_t gfn, u32 p2m_type, u32 op_type);

void ept_setup(void)
{
	u32 error = 0;
	int i = 0;

	/* allocate ept PML4 page */
	struct Page *ept_pml4_page;

	error = page_alloc(&ept_pml4_page);
	assert(error==0);
	memset(page2kva(ept_pml4_page), 0, PAGESIZE);
	
	/* set ept */
	g_ept_ctl.ept_mt = EPT_DEFAULT_MT;
	g_ept_ctl.ept_wl = EPT_DEFAULT_WL;
	g_ept_ctl.asr = GFN(page2pa(ept_pml4_page));

	ept_create_table(ept_pml4_page);
}
static
void ept_create_table(struct Page *ept_pml4_page)
{
	gfn_t gfn;
	virt_t* ept_pml4_page_virt = page2kva(ept_pml4_page);

	//0xFFFFFFFF = 4GB; quick and dirty
	for(gfn = 0; gfn<GFN(0xFFFFFFFF); gfn += EPT_EACHTABLE_ENTRIES)
	{
		if(!ept_update_identity_table(ept_pml4_page_virt, 
					      EPT_DEFAULT_WL, 
					      gfn, 
					      P2M_FULL_ACCESS, 
					      P2M_UPDATE_ALL)
		  )
	   	{
			panic("In ept_create_table gfn:%#08x",gfn);
		}
	}
}
static int 
ept_update_identity_table (
	void* pt,
	u8 level,
	gfn_t gfn,
	u32 p2m_type,
	u32 op_type
)
{
	u64 offset = 0;
	ept_entry_t ept_entry;	
	u64 mask = 0;
	u32 error = 0;

	assert((gfn & (EPT_EACHTABLE_ENTRIES - 1))==0); 

	mask = (1ULL << ((level+1) * EPT_TABLE_ORDER)) - 1;
	offset = (gfn & mask) >> (level * EPT_TABLE_ORDER);

	if (level == 0)
	{
		u32 i = 0;
		for(i = 0; i < EPT_EACHTABLE_ENTRIES; i++)
		{
			panic("ept_update_identity_table not implemented");
			/*Lab3: set the mapping and attributes for all entries*/
			/*NOTE: set MTRR_TYPE_UNCACHE to the emt of read only memory address*/
			/*NOTE: the memory address of JOS is above 0x1000 0000(kern/Makefrag:108)
			 *      To make sure that VM will not overwrite this memory region,
			 *      we shall give a offset of the mapping of memory address 
			 *      above 0x1000 00000.
			 *      --------------       ------------
			 *      |            |    ___|          |
			 *      --------------   /   ------------
			 *      |            |__/    |JOS memory|
			 *      --------------       ----------- 0x1000 0000
			 *      |            |_______|          |
			 *      --------------       -----------
			 */
					
		}
		return true;
  	}
        
	ept_entry.epte =  ((u64 *)pt)[offset];

	phys_t sub_pt_phys;
	virt_t *sub_pt_virt;

	if (ept_entry.epte == 0 ) 
	{
		panic("ept_update_identity_table not implemented");
		/* Lab3: alloc page as sub page table and set the entry */
		/* hint: page_alloc */
		/* hint: ept_entry.mfn = GFN(sub_pt_phys); */
  	} 
	else 
	{
		sub_pt_phys = (phys_t)(ept_entry.mfn << PAGESIZE_SHIFT);
		sub_pt_virt = (virt_t *)KADDR(sub_pt_phys);
	}

	return ept_update_identity_table ((void *)sub_pt_virt, level-1, gfn, p2m_type, op_type);
}
