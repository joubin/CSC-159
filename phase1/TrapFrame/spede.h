// spede.h, CpE/CSc159
// We need these SPEDE library headers.
// All include files are in ~spede/Target-i386/Common/include/spede/

#ifndef _SPEDE_H_
#define _SPEDE_H_

// <spede/flames.h> does a lot:
// splx_t[1] type, splhigh(), splx(), DI(), EI(), PTR2INT(), MK4CODE(),
// IO_DELAY(), breakpoint(), cons_putchar(), cons_getchar(), cons_kbhit(),
// cons_clear, cons_gotoRC(), cons_println(), cons_putstr(), QBIT_ON(), etc.
#include <spede/flames.h>

#include <spede/sys/cdefs.h> // __BEGIN_DECLS, __END_DECLS, etc.
#include <spede/stdio.h> // printf(), sprintf(), vprintf(), sscanf(), etc.
#include <spede/stdlib.h> // TRUE, FALSE, NULL, etc.
#include <spede/string.h> // memset(), strcpy(), strtok(), etc.
#include <spede/time.h> // CLK_TCK, time_t, clock(), time(), etc.
#include <spede/assert.h> // assert()

#include <spede/machine/io.h> // inportb(), outportb(), inportw(), etc.
#include <spede/machine/proc_reg.h> // get_idt_base(), get_cs(), set_cr3(), etc
#include <spede/machine/seg.h> // struct i386_gate, fill_gate(), etc.
#include <spede/machine/asmacros.h> // ASM(), CNAME(), ENTRY(), etc.

// IRQ_VECTOR(), IRQ_TIMER, IRQ_COM1, ICU0_IOBASE, SPECIFIC_EOI(), etc.
#include <spede/machine/pic.h>

#endif

/*  I/O port 0x80 is reserved and will never be used.  If you
 *  access this port, you create a delay of three cycles at 8.33MHz,
 *  plus the I/O access, which takes two clock cycles (total of 5).
 *  An unmapped I/O port goes out to the ISA bus, which runs with
 *  a 8.33MHz clock.  Especially true in PCI systems.
 *  Two of these in a row give a 1.2 microsecond delay.
 *       Attributed to James van Artsdalen.
 */
// #define  IO_DELAY()  ASM ("inb $0x80,%%al" : : : "eax")
