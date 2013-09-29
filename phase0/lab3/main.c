//******************************************************************************
// main.c, Timer Lab
//******************************************************************************

#include <spede/stdio.h>
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/proc_reg.h>
#include <spede/machine/seg.h>
#include <spede/machine/pic.h>

#include "entry.h" // needs TIMER_INTR defined (0x20) & TimerEntry addr

typedef void (* func_ptr_t)();

struct i386_gate *idt_table;

void SetIDTEntry(int entry_num, func_ptr_t entry_addr)
{
	struct i386_gate *gateptr = &idt_table[entry_num];
	fill_gate(gateptr, (int)entry_addr, get_cs(), ACC_INTR_GATE, 0);
}

main()
{
	int i;

	idt_table = get_idt_base(); // get where IDT is in RAM

	cons_printf("IDT is at memory location %u.\n", idt_table);

	SetIDTEntry(32, TimerEntry); // prime IDT with an entry

	outportb(0x21, ~0x01); // 0x21 PIC mask register, ~0x01 mask value

	EI(); // exec CPU inst "sti" to enable intr bit in CPU EFLAGS reg

	while(1) // forever loop
	{
		for (i=0; i<1666000; i++) IO_DELAY(); // delay CPU loop for 1 sec
		cons_putchar('.'); // display an 'X'

		if (cons_kbhit()) break; // polling keyboard, if pressed, break loop
	}
}
