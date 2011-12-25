/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_PMAP_H
#define JOS_KERN_PMAP_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif
#include <inc/memlayout.h>
#include <inc/assert.h>
struct Env;

// Takes a kernel virtual address 'kva' -- an address that points above
// KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
// and returns the corresponding physical address.  Panics if 'kva' is a
// non-kernel virtual address.
#define PADDR(kva)						\
({								\
	physaddr_t __m_kva = (physaddr_t) (kva);		\
	if (__m_kva < KERNBASE)					\
		panic("PADDR called with invalid kva %08lx", __m_kva);\
	__m_kva - KERNBASE;					\
})

// Takes a physical address 'pa' and returns the corresponding kernel virtual
// address.  Panics if 'pa' is an invalid physical address.
#define KADDR(pa)						\
({								\
	physaddr_t __m_pa = (pa);				\
	uint32_t __m_ppn = PGNUM(__m_pa);			\
	if (__m_ppn >= npages)					\
		panic("KADDR called with invalid pa %08lx", __m_pa);\
	(void*) (__m_pa + KERNBASE);				\
})


// Page structures.
// The pages[] array keeps track of the state of physical memory.
// Entry pages[N] holds information about physical page #N.
// The machine has 'npages' pages of physical memory space.
extern struct Page *pages;
extern size_t npages;

// Returns the physical page number corresponding to an element of pages[].
// If pages[N] == pp, then page2ppn(pp) == N.
static inline ppn_t
page2ppn(struct Page *pp)
{
	return pp - pages;
}

// Returns the starting physical address corresponding to an element of
// pages[].  If pages[N] == pp, then page2pa(pp) == N*PGSIZE.
static inline physaddr_t
page2pa(struct Page *pp)
{
	return page2ppn(pp) << PGSHIFT;
}

// Returns the element of pages[] that contains physical address 'pa'.
// That is, returns &pages[PGNUM(pa)].
static inline struct Page *
pa2page(physaddr_t pa)
{
	if (PGNUM(pa) >= npages)
		panic("pa2page called with invalid pa");
	return &pages[PGNUM(pa)];
}

// Returns the starting kernel virtual address corresponding to an element of
// pages[].  If pages[N] == pp, then page2kva(pp) == KADDR(N*PGSIZE) ==
// N*PGSIZE + KERNBASE.
static inline void*
page2kva(struct Page *pp)
{
	return KADDR(page2pa(pp));
}


extern pde_t *kern_pgdir;

extern struct Segdesc gdt[];
extern struct Pseudodesc gdt_pd;

void	boot_mem_init(void);

void	page_init(void);
void	page_check(void);
int	page_alloc(struct Page **pp_store);
void	page_free(struct Page *pp);
int	page_insert(pde_t *pgdir, struct Page *pp, void *va, int perm);
void	page_remove(pde_t *pgdir, void *va);
struct Page *page_lookup(pde_t *pgdir, void *va, pte_t **pte_store);
void	page_decref(struct Page *pp);
pte_t *	pgdir_walk(pde_t *pgdir, const void *va, int create);

void	tlb_invalidate(pde_t *pgdir, void *va);

int	user_mem_check(struct Env *env, const void *va, size_t len, int perm);
void	user_mem_assert(struct Env *env, const void *va, size_t len, int perm);

#endif /* !JOS_KERN_PMAP_H */
