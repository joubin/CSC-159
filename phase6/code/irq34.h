// irq34.h  159 phase 6

#ifndef _IRQ34_H_
#define _IRQ34_H_

#include <spede/string.h>
#include <spede/ctype.h>           // isprint(), isspace()
#include <spede/sys/ascii.h>       // CH_BACKSPACE, etc
#include <spede/machine/io.h>
#include <spede/machine/rs232.h>   // for flags needed below
#include "types.h"

#define NUM_TERM 3                 // use at most 3 terminals

typedef struct
{
   int
      io_base,      // different in io_base but..
      irq,          // same IRQ # 4 for COM1/3/5/7 & IRQ # 3 for 2/4/6/8
      incoming_sid, // IRQ34ISRInChar() SemPostISR() with this
      outgoing_sid, // IRQ34ISROutChar() SemPostISR() with this
      echo_on,      // when entering password, can disable copy into echo_q
      missed_intr;  // mark it for initial/additional ISR putc/getc opeartion

   char_q_t
      incoming_q,   // 3 terminal queues: keyed-in, display, echo
      outgoing_q,
      echo_q;

} terminal_t;

void TerminalInit(int); // called by Init() to init terminals

void Stdin(); // a process that handles terminal keyboard input
void StdinString(int, char *); // subroutine of Stdin()

void Stdout(); // a process that handles terminal display output
void StdoutString(int, char *); // subroutine of Stdout()
void StdoutChar(int, char); // subroutine of StdoutString()

void IRQ34ISR(); // ISR for IRQ 3 and 4
void IRQ34ISROutChar(int); // subroutine of IRQ34ISR() to output a char
void IRQ34ISRInChar(int); // subroutine of IRQ34ISR() to input a char

#endif
