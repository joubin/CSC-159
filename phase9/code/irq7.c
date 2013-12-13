// irq7.c  Phase 5 of CpE/CSc 159
//
// the code needed for printing via parallel port

#include <spede/machine/parallel.h> // flag constants used below
#include "externs.h"   // has printing semaphore ID needed below
#include "sys_calls.h" // prototypes of sys calls
#include "isr.h"       // prototypes of ISR calls
#include "irq7.h"      // internal dependecies in this .c
#include "spede.h"

int io_base = 0x378;   // io_base of parallel port 1 (LPT1)

// a driver process usually runs a service loop
void PrintDriver() {
	int result;
	msg_t msg;

	// PrintInit() is optional for busy-poll printing (if printer's been OK)
	PrintInit();  // get print_sid, enable IRQ7 on printer port

	while(1)      // loop for print service
	{
		MsgRcv(&msg); // receive print request
		result = PrintStr(msg.bytes); // msg.byte is str to print
		if(result == -1)  // something timed out from PrintStr()
			cons_printf("\n >>> PrintDriver: print error!\n");
	}
}

// initialize data structures and the printer. Since there is only one
// printer device, we can use global variables. Resets the printer by
// setting control register without PC_INIT, then putting it back in.
void PrintInit() {
	int status, print_result;

	// sem is for non-polling, interrupt-driver I/O, printing
	print_sid = SemInit(-1); // why -1, not 0? what happens if 0?

	// PrintInit() is optional for busy-poll printing (if printer's been OK)
	// this resets the printer port, the connected printer will jitter
	// outportb PC_SLCTIN to io_base + LPT_CONTROL
	outportb(io_base + LPT_CONTROL, PC_SLCTIN);
	// read status from io_base + LPT_STATUS
	status = inportb(io_base + LPT_STATUS);
	// outportb or-ed flags: PC_INIT | PC_SLCTIN | PC_IRQEN to
	// io_base + LPT_CONTROL
	outportb(io_base + LPT_CONTROL, PC_INIT | PC_SLCTIN | PC_IRQEN);

	Sleep(1); // need time for printer to go thru its init stage

	// print test message:
	print_result = PrintStr("Testing, 1-2-3-1-2-3...\n\n\0");
	// if print_result is -1, show "PrintInit: test error!" on the PC
	if (print_result == -1)
	{
		cons_printf("PrintInit: test error!");
	}
}

// loop for each character in str (until null-terminated),
// calling PrintChar(). If a char in str is '\n' then also PrintChar('\r')
int PrintStr(char *str) {
	int result;

	int TIME_OUT = 3 * 1666000;           // 3-sec time-out period
	int i, the_code;

	for(i = 0; i < TIME_OUT; i++) {
		the_code = inportb(io_base + LPT_STATUS) & PS_BUSY;
		if(the_code != 0) break;        // not busy, start print str!
		IO_DELAY();                     // .65 us
	}
	if(i == TIME_OUT) {  // timed out, return -1
		cons_printf(">>> PrintStr: printer timed out (power/online)?\n");
		return -1;
	}

	// for each ch of str: (convert this line to code, can use str++)
	while (*str != '\0') {
		//    send it to PrintChar()
		result = PrintChar(*str);
		//    if PrintChar() returns -1, return that (failed to print ch)
		if (result == -1) {
			return -1;
		}
		//    if ch is '\n' (manual treatment of ASCII 10, add ASCII 13),
		//    also prints char '\r' which is printer's newline char
		if (*str == '\n') {
			result = PrintChar('\r');
		}
		//    if returned -1 return that
		if (result == -1) {
			return -1;
		}
		// end loop
		str++;
	}
	return 0;
}

// Each character is put to print. First it is the busy-poll method:
// send a char, a control signal (print), then repeatedly poll the
// printer port for an ACK signal until timed out.
int PrintChar(char ch) { // in irq7.c
	//int TIME_OUT = 3*1666000;   // about 3 sec then time out
	int i, the_code;

	outportb(io_base + LPT_DATA, ch);          // send char as data
	the_code = inportb(io_base + LPT_CONTROL); // read control
	outportb(io_base + LPT_CONTROL, the_code | PC_STROBE); // add strobe

	for(i=0; i<9; i++) IO_DELAY();      // Epson LP571 needs 9 for delay

	outportb(io_base + LPT_CONTROL, the_code); // return original control
	// above outportb() causes an IRQ7 signal which is OK to still

	SemWait(print_sid);

	return 0; // didn't time out, return 0
}
