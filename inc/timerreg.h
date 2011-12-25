/*	$NetBSD: timerreg.h,v 1.4 1994/10/27 04:18:17 cgd Exp $	*/

/*
 * Register definitions for the Intel
 * 8253/8254/82C54 Programmable Interval Timer (PIT).
 *
 * This chip has three independent 16-bit down counters that can be
 * read on the fly.  There are three mode registers and three countdown
 * registers.  The countdown registers are addressed directly, via the
 * first three I/O ports.  The three mode registers are accessed via
 * the fourth I/O port, with two bits in the mode byte indicating the
 * register.  (Why are hardware interfaces always so braindead?).
 *
 * To write a value into the countdown register, the mode register
 * is first programmed with a command indicating the which byte of
 * the two byte register is to be modified.  The three possibilities
 * are load msb(TMR_MR_MSB), load lsb(TMR_MR_LSB), or load lsb then
 * msb(TMR_MR_BOTH).
 *
 * To read the current value("on the fly") from the countdown register,
 * you write a "latch" command into the mode register, then read the stable
 * value from the corresponding I/O port.  For example, you write
 * TMR_MR_LATCH into the corresponding mode register.  Presumably,
 * after doing this, a write operation to the I/O port would result
 * in undefined behavior(but hopefully not fry the chip).
 * Reading in this manner has no side effects.
 *
 * The outputs of the three timers are connected as follows:
 *
 *	 timer 0 -> irq 0
 *	 timer 1 -> dma chan 0 (for dram refresh)
 * 	 timer 2 -> speaker(via keyboard controller)
 *
 * Timer 0 is used to call hardclock.
 * Timer 2 is used to generate console beeps.
 */

/*
 * Frequency of all three count-down timers; (TIMER_FREQ/freq) is the
 * appropriate count to generate a frequency of freq hz.
 */
#define	TIMER_FREQ	1193182
#define TIMER_DIV(x)	((TIMER_FREQ+(x)/2)/(x))

/*
 * Macros for specifying values to be written into a mode register.
 */
#define	TIMER_CNTR0	(IO_TIMER1 + 0)	/* timer 0 counter port */
#define	TIMER_CNTR1	(IO_TIMER1 + 1)	/* timer 1 counter port */
#define	TIMER_CNTR2	(IO_TIMER1 + 2)	/* timer 2 counter port */
#define	TIMER_MODE	(IO_TIMER1 + 3)	/* timer mode port */
#define		TIMER_SEL0	0x00	/* select counter 0 */
#define		TIMER_SEL1	0x40	/* select counter 1 */
#define		TIMER_SEL2	0x80	/* select counter 2 */
#define		TIMER_INTTC	0x00	/* mode 0, intr on terminal cnt */
#define		TIMER_ONESHOT	0x02	/* mode 1, one shot */
#define		TIMER_RATEGEN	0x04	/* mode 2, rate generator */
#define		TIMER_SQWAVE	0x06	/* mode 3, square wave */
#define		TIMER_SWSTROBE	0x08	/* mode 4, s/w triggered strobe */
#define		TIMER_HWSTROBE	0x0a	/* mode 5, h/w triggered strobe */
#define		TIMER_LATCH	0x00	/* latch counter for reading */
#define		TIMER_LSB	0x10	/* r/w counter LSB */
#define		TIMER_MSB	0x20	/* r/w counter MSB */
#define		TIMER_16BIT	0x30	/* r/w counter 16 bits, LSB first */
#define		TIMER_BCD	0x01	/* count in BCD */

