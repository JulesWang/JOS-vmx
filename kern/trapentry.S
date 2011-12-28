/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>


###################################################################
# exceptions/interrupts
###################################################################

/* The TRAPHANDLER macro defines a globally-visible function for handling
 * a trap.  It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 */ 
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text
TRAPHANDLER_NOEC(handler0,0);
TRAPHANDLER_NOEC(handler1,1);
TRAPHANDLER_NOEC(handler2,2);
TRAPHANDLER_NOEC(handler3,3);
TRAPHANDLER_NOEC(handler4,4);
TRAPHANDLER_NOEC(handler5,5);
TRAPHANDLER_NOEC(handler6,6);
TRAPHANDLER_NOEC(handler7,7);
TRAPHANDLER(handler8,8);
TRAPHANDLER_NOEC(handler9,9);
TRAPHANDLER(handler10,10);
TRAPHANDLER(handler11,11);
TRAPHANDLER(handler12,12);
TRAPHANDLER(handler13,13);
TRAPHANDLER(handler14,14);
TRAPHANDLER(handler15,15);
TRAPHANDLER_NOEC(handler16,16);
TRAPHANDLER_NOEC(handler17,17);
TRAPHANDLER_NOEC(handler18,18);
TRAPHANDLER_NOEC(handler19,19);

TRAPHANDLER_NOEC(handler48,T_SYSCALL);

/* 
struct Trapframe { 
    struct Registers tf_regs; 
    uint16_t tf_es; 
    uint16_t tf_padding1; 
    uint16_t tf_ds; 
    uint16_t tf_padding2; 
    uint32_t tf_trapno; 
    // below here defined by x86 hardware 
    // and pushed by the processor when an exception is taken 
    ..... 
} 
 */ 
_alltraps: 
    pushl %ds 
    pushl %es 
    pushal 
 
    movl $GD_KD , %eax 
    movw %ax ,%ds 
    movw %ax ,%es 
 
    pushl %esp 
 
    call trap 
 
    pop %esp 
    popal 
    pop %es 
    pop %ds 
    addl $0x8 , %esp //ignore parameters--num and trapno 
 
    iret
