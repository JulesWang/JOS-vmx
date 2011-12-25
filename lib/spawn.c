#include <inc/lib.h>
#include <inc/elf.h>

#define UTEMP2USTACK(addr)	((void*) (addr) + (USTACKTOP - PGSIZE) - UTEMP)
#define UTEMP2			(UTEMP + PGSIZE)
#define UTEMP3			(UTEMP2 + PGSIZE)

// Helper functions for spawn.
static int init_stack(envid_t child, const char **argv, uintptr_t *init_esp);
static int load_segment(envid_t child, int id,
			struct Proghdr *ph, struct Elf *elf, size_t elf_size);

// Load a page from a program binary into an environment 'dst_env'
// at virtual address 'pg', and with permissions 'pg_perm'.
// Note that 'pg_perm' will never grant write permission (no PTE_W).
// In Lab 4, the program binary is always located in the kernel,
// and its data is accessed using 'sys_program_page_map'.
// In Lab 5, you will extend this function to interact with the file system
// when 'id' is a file descriptor rather than a program ID.
int
map_page(envid_t dst_env, int id, size_t offset, void *pg, unsigned pg_perm)
{
	if (id >= PROGRAM_OFFSET)
		return sys_program_page_map(dst_env, id, offset, pg, pg_perm);

	panic("load_page not implemented for file descriptors!");
}


// Spawn a child process from a program image.
// In Lab 4, the image is loaded from the kernel.
// In Lab 5, you will allow images to be loaded from the file system.
// prog: the name of the program to run.
// argv: pointer to null-terminated array of pointers to strings,
// 	 which will be passed to the child as its command-line arguments.
// Returns child envid on success, < 0 on failure.
envid_t
spawn(const char* progname, const char** argv)
{
	uint8_t elf_header_buf[512];
	ssize_t elf_size;
	struct Trapframe child_tf; 	/* Hint!! */

	// Insert your code, following approximately this procedure:
	//
	//   - Look up the program using sys_program_lookup.
	//     Return an error code if no such program exists.
	//
	//   - Set 'elf_size' to the program's ELF binary size using
	//     sys_program_size.
	//
	//   - Map the program's first page at UTEMP using the map_page helper.
	//
	//   - Copy the 512-byte ELF header from UTEMP into elf_header_buf.
	//
	//   - Read the ELF header, as you have before, and sanity check its
	//     magic number.  (Check out your load_icode for hints!)
	//
	//   - Use sys_exofork() to create a new environment.
	//
	//   - Set child_tf to an initial struct Trapframe for the child.
	//     Hint: The sys_exofork() system call has already created
	//     a good starting point.  It is accessible at
	//     envs[ENVX(child)].env_tf.
	//     Hint: You must do something with the program's entry point.
	//     What?  (See load_icode!)
	//
	//   - Call the init_stack() function to set up the initial stack
	//     page for the child environment.
	//
	//   - Map all of the program's segments that are of p_type
	//     ELF_PROG_LOAD into the new environment's address space.
	//     Use the load_segment() helper function below.
	//     All the 'struct Proghdr' structures will be accessible
	//     within the first 512 bytes of the ELF.
	//
	//   - Call sys_env_set_trapframe(child, &child_tf) to set up the
	//     correct initial eip and esp values in the child.
	//
	//   - Start the child process running with sys_env_set_status().
	//
	panic("spawn unimplemented!");
}

// Spawn, taking command-line arguments array directly on the stack.
envid_t
spawnl(const char *prog, const char *args, ...)
{
	return spawn(prog, &args);
}


// Set up the initial stack page for the new child process with envid 'child'
// using the arguments array pointed to by 'argv',
// which is a null-terminated array of pointers to '\0'-terminated strings.
//
// On success, returns 0 and sets *init_esp
// to the initial stack pointer with which the child should start.
// Returns < 0 on failure.
static int
init_stack(envid_t child, const char **argv, uintptr_t *init_esp)
{
	size_t string_size;
	int argc, i, r;
	char *string_store;
	uintptr_t *argv_store;

	// Count the number of arguments (argc)
	// and the total amount of space needed for strings (string_size).
	string_size = 0;
	for (argc = 0; argv[argc] != 0; argc++)
		string_size += strlen(argv[argc]) + 1;

	// Determine where to place the strings and the argv array.
	// We set up the 'string_store' and 'argv_store' pointers to point
	// into the temporary page at UTEMP.
	// Later, we'll remap that page into the child environment
	// at (USTACKTOP - PGSIZE).
	
	// strings is the topmost thing on the stack.
	string_store = (char*) UTEMP + PGSIZE - string_size;
	
	// argv is below that.  There's one argument pointer per argument, plus
	// a null pointer.
	argv_store = (uintptr_t*) (ROUNDDOWN(string_store, 4) - 4 * (argc + 1));
	
	// Make sure that argv, strings, and the 2 words that hold 'argc'
	// and 'argv' themselves will all fit in a single stack page.
	if ((void*) (argv_store - 2) < (void*) UTEMP)
		return -E_NO_MEM;

	// Allocate a page at UTEMP.
	if ((r = sys_page_alloc(0, (void*) UTEMP, PTE_P|PTE_U|PTE_W)) < 0)
		return r;

	// Replace this with your code to:
	//
	//	* Initialize 'argv_store[i]' to point to argument string i,
	//	  for all 0 <= i < argc.
	//	  Also, copy the argument strings from 'argv' into the
	//	  newly-allocated stack page.
	//	  Hint: Copy the argument strings into string_store.
	//	  Hint: Make sure that argv_store uses addresses valid in the
	//	  CHILD'S environment!  The string_store variable itself
	//	  points into page UTEMP, but the child environment will have
	//	  this page mapped at USTACKTOP - PGSIZE.  Check out the
	//	  UTEMP2USTACK macro defined above.
	//
	//	* Set 'argv_store[argc]' to 0 to null-terminate the args array.
	//
	//	* Push two more words onto the child's stack below 'args',
	//	  containing the argc and argv parameters to be passed
	//	  to the child's umain() function.
	//	  argv should be below argc on the stack.
	//	  (Again, argv should use an address valid in the child's
	//	  environment.)
	//
	//	* Set *init_esp to the initial stack pointer for the child,
	//	  (Again, use an address valid in the child's environment.)
	//
	// LAB 4: Your code here.
	*init_esp = USTACKTOP;	// Change this!

	// After completing the stack, map it into the child's address space
	// and unmap it from ours!
	if ((r = sys_page_map(0, (void*) UTEMP, child, (void*) (USTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
		goto error;
	if ((r = sys_page_unmap(0, (void*) UTEMP)) < 0)
		goto error;

	return 0;

error:
	sys_page_unmap(0, (void*) UTEMP);
	return r;
}

// Load the 'ph' program segment of program 'id' into 'child's address
// space at the proper place.
//
// Returns 0 on success, < 0 on error.  It is also OK panic on error.
// There's no need to clean up page mappings after error; the caller will call
// sys_env_destroy(), which cleans up most mappings for us.
static int
load_segment(envid_t child, int id, struct Proghdr* ph,
	     struct Elf* elf, size_t elf_size)
{
	// Use the p_flags field in the Proghdr for each segment
	// to determine how to map the segment:
	//
	//    * If the ELF flags do not include ELF_PROG_FLAG_WRITE,
	//	then the segment contains text and read-only data.
	//	Use map_page() to map the contents of this segment
	//	directly into the child, so that multiple instances of
	//	the same program will share the same copy of the program text.
	//      Be sure to map the program text read-only in the child.
	//
	//    * If the ELF segment flags DO include ELF_PROG_FLAG_WRITE,
	//	then the segment contains read/write data and bss.
	//	As with load_icode(), such an ELF segment
	//	occupies p_memsz bytes in memory, but only the first
	//	p_filesz bytes of the segment are actually loaded
	//	from the executable file -- you must clear the rest to zero.
	//      For each page to be mapped for a read/write segment,
	//	allocate a page of memory at UTEMP in the current
	//	environment.  Then use map_page() to map that portion of
	//	the program at UTEMP2.  Then copy the data from UTEMP2 into
	//	the page at UTEMP, and/or use memset() to zero any non-loaded
	//	portions of the page.  (You can avoid calling memset(),
	//	if you like, because sys_page_alloc() returns zeroed pages
	//	already.)  Finally, insert the correct page mapping into
	//	the child, and unmap the page at UTEMP2.
	//	Look at load_icode() and fork() for inspiration.
	//
	// Note: All of the segment addresses or lengths above
	// might be non-page-aligned, so you must deal with
	// these non-page-aligned values appropriately.
	// The ELF linker does, however, guarantee that no two segments
	// will overlap on the same page; and it guarantees that
	// PGOFF(ph->p_offset) == PGOFF(ph->p_va).
	
	// LAB 4: Your code here.
	panic("load_segment not completed!\n");
	return -1;
}

