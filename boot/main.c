#include <inc/x86.h>
#include <inc/elf.h>

/**********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program (boot.S and main.c) is the bootloader.  It should
 *    be stored in the disk's sector 0 (the first sector).
 * 
 *  * The 2nd sector onward holds the kernel image.
 *	
 *  * The kernel image must be in ELF executable format.
 *
 * BOOT UP STEPS	
 *  * When the CPU boots it loads the BIOS into memory and executes it.
 *
 *  * The BIOS intializes devices, sets up the interrupt routines, and
 *    reads the first sector of the boot device (e.g., hard-drive) 
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over.
 *
 *  * Control starts in boot.S, which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain().
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 *
 **********************************************************************/

#define SECTSIZE	512
#define PAGESIZE	4096
#define ELFHDR		((struct Elf *) 0x10000) // scratch space

void readsect(void *addr, uint32_t sectornum);
void readseg(uint32_t va, uint32_t filesz, uint32_t memsz, uint32_t offset);

void
bootmain(void)
{
	struct Proghdr *ph, *eph;

	// read 1st page off disk
	readseg((uint32_t) ELFHDR, PAGESIZE, PAGESIZE, 0);

	// is this a valid ELF?
	if (ELFHDR->e_magic != ELF_MAGIC)
		goto bad;

	// load each program segment (ignores ph flags)
	ph = (struct Proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
	eph = ph + ELFHDR->e_phnum;
	for (; ph < eph; ph++)
		readseg(ph->p_va, ph->p_filesz, ph->p_memsz, ph->p_offset);

	// call the entry point from the ELF header
	// note: does not return!
	((void (*)(void)) (ELFHDR->e_entry & 0xFFFFFF))();

bad:
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8E00);
	/* boot.S will spin for us */
}

// Read 'filesz' bytes at 'offset' from kernel into virtual address 'va',
// then clear the memory from 'va+filesz' up to 'va+memsz' (set it to 0).
void
readseg(uint32_t va, uint32_t filesz, uint32_t memsz, uint32_t offset)
{
	uint32_t end_va;

	va &= 0xFFFFFF;
	end_va = va + filesz;
	memsz += va;
	
	// round down to sector boundary
	va &= ~(SECTSIZE - 1);

	// translate from bytes to sectors, and kernel starts at sector 1
	offset = (offset / SECTSIZE) + 1;

	// read sectors
	while (va < end_va) {
		readsect((uint8_t*) va, offset);
		va += SECTSIZE;
		offset++;
	}

	// clear bss segment
	while (end_va < memsz)
		*((uint8_t*) end_va++) = 0;
}

void
waitdisk(void)
{
	// wait for disk reaady
	while ((inb(0x1F7) & 0xC0) != 0x40)
		/* do nothing */;
}

void
readsect(void *dst, uint32_t offset)
{
	// wait for disk to be ready
	waitdisk();

	outb(0x1F2, 1);		// count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);	// cmd 0x20 - read sectors

	// wait for disk to be ready
	waitdisk();

	// read a sector
	insl(0x1F0, dst, SECTSIZE/4);
}

