// irq34.c, 159 phase 6, serial port device driver

// irq34.h defines constants including NUM_TERM, and type terminal_t
#include "irq34.h"
#include "spede.h"
#include "q_mgmt.h"

#include "sys_calls.h" // SemWait, PrintChar() calls via TerminalInit
#include "isr.h"       // SemPostISR(int) from IRQ34ISR

// set the first 2 fields of structure, the rest is initialized by
// C-compiler as null values.
// COM1~8 io_base: 0x3f8 0x2f8 0x3e8 0x2e8 0x2f0 0x3e0 0x2e0 0x260
// COM1~8 irq: 4 3 4 3 4 3 4 3
terminal_t terminals[NUM_TERM]= {
	{COM2_IOBASE, 3}, // IO port base and IRQ #
	{COM3_IOBASE, 4},
	{COM4_IOBASE, 3},
};

// setup data structures and UART of a terminal. Clear char queues,
// and preset the bounded buffer semaphore. Put UART to interrupt mode:
// clear IER, enable TXRDY & RXRDY for interrupts.
void TerminalInit(int term_num) {
	int BAUD_RATE = 9600;
	int divisor = 115200 / BAUD_RATE;

	// first setup our vars
	terminals[term_num].echo_on = TRUE;
	terminals[term_num].missed_intr = TRUE;

	// Use a pair of sems. One limits available space in the output queue
	// (terminal display), the other limits chars that are typed from the
	// terminal. As part of initialization, the count of the output queue
	// is set to the capacity of the char queue
	terminals[term_num].incoming_sid = SemInit(0);           // nothing in
	terminals[term_num].outgoing_sid = SemInit(CHAR_Q_SIZE); // all avail

	CharInitQ(&terminals[term_num].incoming_q);   // initially empty
	CharInitQ(&terminals[term_num].outgoing_q);   // initially empty
	CharInitQ(&terminals[term_num].echo_q);       // initially empty

	// then setup the terminal for 7-E-1 at 9600 baud
	// abbrevs:
	// CFCR Char Format Control Reg, MSR Modem Status Reg, IIR Intr Indicator Reg
	// MCR Modem Control Reg, IER Intr Enable Reg, LSR Line Status Reg
	// ERXRDY Enable Recv Ready, ETXRDY Enable Xmit Ready
	// LSR_TSRE Line Status Reg Xmit+Shift Regs Empty

	outportb(terminals[term_num].io_base+CFCR, CFCR_DLAB); // CFCR_DLAB is 0x80
	outportb(terminals[term_num].io_base+BAUDLO, LOBYTE(divisor));
	outportb(terminals[term_num].io_base+BAUDHI, HIBYTE(divisor));

	outportb(terminals[term_num].io_base+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
	outportb(terminals[term_num].io_base+IER, 0);

	// raise DTR & RTS of the serial port to start read/write
	outportb(terminals[term_num].io_base+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
	IO_DELAY();
	outportb(terminals[term_num].io_base+IER, IER_ERXRDY|IER_ETXRDY);
	IO_DELAY();

	// A null-terminated test message is sent to display to the terminal.
	// "ESC *" will clear screen on TVI 910, but we use newlines for portability
	StdoutString(term_num, "\n\n\nFollow the pink bunny on pogostick.\n\n\n\0");
}

//*********************************************************************
// this serves as a shell's stdout, the terminal output/display process
//*********************************************************************
void Stdout() {
	msg_t msg;
	int term_num, shell_pid, sender;

	MsgRcv(&msg); // msg provided by Init() for this process to know:

	term_num = msg.numbers[1];  // need info which terminal structure to use
	shell_pid = msg.sender;  // need to know mbox ID of shell servicing

	while(1) { // service loop, servicing shell_pid only actually
		MsgRcv(&msg);
        sender = msg.sender;
		StdoutString(term_num, msg.bytes); // bytes is str to display
		MsgSnd(sender, &msg);                // completion msg, content not important
	}
}

void StdoutString(int term_num, char *str) {
	char *p;

	for(p = str; *p; p++) { // str must be a null-terminated string
		// in <spede/sys/ascii.h> CH_LF is \n (^J), CH_CR is \r (^M)
		if(*p == CH_LF) StdoutChar(term_num, CH_CR); // add a CR if LF
		StdoutChar(term_num, *p);
	}
}

void StdoutChar(int term_num, char ch) {

	SemWait(terminals[term_num].outgoing_sid);

	CharEnQ(ch, &terminals[term_num].outgoing_q);

	// check if UART xmitted interrupt (a space ready has already happened)
	if(terminals[term_num].missed_intr) IRQ34ISROutChar(term_num);
}

//*********************************************************************
// This serves as a shell's stdin, the terminal input/keyboard process
//*********************************************************************
void Stdin() {
	msg_t msg;
	int term_num, shell_pid;

	MsgRcv(&msg); // sent from Init() for info below:

	term_num = msg.numbers[1];
	shell_pid = msg.sender;

	while(1) // loop of service, servicing shell_pid only actually
	{
		MsgRcv(&msg); // request from shell to read terminal keyboard input

		MyBZero(msg.bytes, NUM_BYTE);     // filled with zeroes (nulls)
		StdinString(term_num, msg.bytes); // read into string msg.bytes

		MsgSnd(shell_pid, &msg); // serve user shell
	}
}

// Fill a string buffer with char data. Depending on echo_on, can read
// up to capacity of str, or till NEWLINE char (null termination added)
void StdinString(int term_num, char *str) {
	int ch, num_ch_read;

	num_ch_read = 0;

	do {
		SemWait(terminals[term_num].incoming_sid); // wait terminal key-in to SemPostISR()

		// Access the circular queue. Disable interrupts to protect against
		// concurrent write (race). Since we wait, guaranteed there's a char.

		ch = CharDeQ(&terminals[term_num].incoming_q); // get a char from in queue

		if(ch < 0) { // incoming_q is empty, something's wrong with UARTdevice?
			cons_printf("Terminal %d: read char, none there, abort!\n", term_num);
			abort(); // abort() is an ugly way to exit...
		}
		if(ch == CH_BS) { // got a char, if backspace, backspacing handling:
			// process CH_BS: delete prev buffer char if there is one. We could handle
			// backspace in the lower half, but it would not know the size of the buffer.
			// Thus user could keep hitting backspace, and the bottom-half would be
			// obliged to keep doing BS+' '+BS, possibly even backspacing over the prompt!
			if(num_ch_read > 0) { // if there's at least 1 char to back up
				num_ch_read--; // 1 less char read
				str--;         // backup 1 char in str

				CharEnQ(CH_BS, &terminals[term_num].echo_q);    // cursor back
				CharEnQ(CH_SPACE, &terminals[term_num].echo_q); // disp space (erase)
				CharEnQ(CH_BS, &terminals[term_num].echo_q);    // cursor back

				// Now tickle to the output logic. If we missed an interrupt, then must
				// force out the first char. Without this, erasing doesn't appear until
				// the user types a char later.
				if(terminals[term_num].missed_intr) IRQ34ISROutChar(term_num);
			}
		}
		else if(ch == CH_CR) {   // do nothing, don't need \r from terminal
		} else {                 // a normal character typed
			*str++ = ch;
			num_ch_read++;
		}
	} while(ch != CH_CR &&           // until CR is entered
			ch != CH_ESCAPE &&       // will we ever get an escape char?
			num_ch_read < NUM_BYTE); // too many chars? save last char for null

	*str = '\0';   // null-terminate
}

//****************************** LOWER HALF ******************************
//********************************* BELOW ********************************
// upon either IRQ 3 or 4, below called by Kernel()
//************************************************************************
//************************************************************************
void IRQ34ISR() {
	int status, i;

	// when an interrupt occurs, check all UART's to see which one needs service
	for(i=0; i<NUM_TERM; i++) { // can actually check all terminals in a loop
		// if xmitter empty but no no char queued, we missed an interrupt, disable
		// them at serial port
		while( !(IIR_NOPEND & (status = inportb(terminals[i].io_base+IIR)))) {
			switch(status) {
				case IIR_RXRDY: IRQ34ISRInChar(i); break; // recv buffer has data
				case IIR_TXRDY: IRQ34ISROutChar(i); break; // xmit buffer has space
			}
		} // while UART needs servicing

		// We might have to echo the char we just received. IRQ34ISRInChar() can insert
		// to echo_q, however only call IRQ34ISROutChar() if TBE is true. Can't do it
		// inside "while" loop in case trigger includes TXRDY and RXRDY. We might call
		// IRQ34ISROutChar() for RXRDY case (to handle echo), then call again for TXRDY
		// case, which we must do.
		// Calling out here means "after all required interrupt processing is there
		// anything to echo?" You can trigger this by typing quickly, when chars are
		// being echoed. If LSR_TXRDY gives errors, then try LSR_TSRE instead.
		if(terminals[i].missed_intr) IRQ34ISROutChar(i);
	}
	outportb(0x20, 0x63); // 0x20 is PIC control reg, 0x63 dismiss IRQ 3
	outportb(0x20, 0x64); // 0x20 is PIC control reg, 0x64 dismiss IRQ 4
}

//
// remove char from outgoing_q and send it out via UART
//
void IRQ34ISROutChar(int term_num) {
	char ch;

	terminals[term_num].missed_intr = FALSE;

	if(CharEmptyQ(&terminals[term_num].outgoing_q) &&
			CharEmptyQ(&terminals[term_num].echo_q)) {
		// nothing to display but the xmitter is ready, missed a chance to send a char
		terminals[term_num].missed_intr = TRUE; // set it so can use output later
		return;                                 // nothing more to do more
	}
	if( !CharEmptyQ(&terminals[term_num].echo_q) ) { // echo has higher priority
		ch = CharDeQ(&terminals[term_num].echo_q); // get a char from echo queue
	} else { // output ch from out queue, this makes room for more
		ch = CharDeQ(&terminals[term_num].outgoing_q); // get a char from out queue
		SemPostISR(terminals[term_num].outgoing_sid);  // post a space (char)
	}
	outportb(terminals[term_num].io_base + DATA, ch); // output ch to UART
}

// read incoming char from UART and store into input queue, if there's no
// room, place a bell character to echo queue to send a alarm sound to the
// terminal instead
void IRQ34ISRInChar(int term_num) {
	char ch,status;

	status = inportb(terminals[term_num].io_base + LSR);
	// what's keyed-in, mask with 127, to get 7-bit ASCII char
	ch = inportb(terminals[term_num].io_base + DATA) & 0x7F;

	// Error inside UART. We've cleared the condition by reading it.
	// However, the character is messed up, replace it.
	if(QBIT_ANY_ON(status, LSR_FE | LSR_PE | LSR_OE)) ch = CH_QMARK; // '?'

	if(CharFullQ(&terminals[term_num].incoming_q)) { // in buffer full (typed too fast?)
		CharEnQ(CH_BELL, &terminals[term_num].echo_q); // "bell" goes to terminal
	}
	else { // add char to in queue, signal data now avail
		if(ch != 0) { // got a valid input char from terminal keyboard
			CharEnQ(ch, &terminals[term_num].incoming_q); // put ch into in queue
			SemPostISR(terminals[term_num].incoming_sid); // post data-avail semaphore
		}
		if(terminals[term_num].echo_on) { // echo char if echo mode set
			// Echo only characters with a representation. This includes printable and
			// spacing chars. Other are commands (control) chars we must interpret
			if(isprint(ch) || isspace(ch)) { // ch is printable or space
				CharEnQ(ch, &terminals[term_num].echo_q); // put ch into echo queue
				if(ch == CH_CR)
					CharEnQ(CH_LF, &terminals[term_num].echo_q); // add LF if it's CR
			} else { // it's a control char
				// let upper-level do all backspacing. if the line has been completely
				// backspaced, then we should ignore any further BS inputs. Unless we want
				// to look like SVR3. remaining control chars echoed like ^A for ctrl-A
				if(ch != CH_BS) {
					CharEnQ(CH_CARET, &terminals[term_num].echo_q); // add '^'
					CharEnQ(ch ^ 0x40,&terminals[term_num].echo_q); // add '^'
				}
			}
		}
	}
}

