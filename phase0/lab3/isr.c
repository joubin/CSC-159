//******************************************************************************
// isr.c, Timer Lab
//******************************************************************************

#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

// 2-byte ptr points to PC VGA display memory
unsigned short *vidmem = (unsigned short *) 0xB8000 + 12*80 + 40;

#define ASCII_SPACE (0xf00 + ' ') // space 0xf00 is VGA attr W on B

void TimerISR() // every 1/2 second display a different ASCII char on PC
{
	static int ascii_char = ASCII_SPACE; // start with the space char
	static int tick_count = 0; // count IRQ 0 occurrences

	// dismiss IRQ 0, otherwise new one won't edge-trigger intr to CPU
	outportb(0x20, 0x60); // 0x20 is PIC control, 0x60 dismisses IRQ 0

	if (++tick_count % 50 == 0)
	{
		*vidmem = ascii_char++;

		if (ascii_char == ASCII_SPACE + 95)
			ascii_char = ASCII_SPACE;
	}
}
