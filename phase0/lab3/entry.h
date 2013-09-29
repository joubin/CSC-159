// entry.h of entry.S, Timer Lab

#ifndef _ENTRY_H_
#define _ENTRY_H_

#define TIMER_INTR 0x20

#ifndef ASSEMBLER // skip if ASSEMBLER defined (in assembly code)

void TimerEntry(); // defined in entry.S, assembly won't take this

#endif

#endif
