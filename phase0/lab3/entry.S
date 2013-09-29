// entry.S, Timer Lab

#include <spede/machine/asmacros.h>
#include "entry.h"

ENTRY(TimerEntry)
	pusha
	cld
	call CNAME(TimerISR)
	popa
	iret
