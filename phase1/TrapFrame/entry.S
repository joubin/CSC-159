// entry.S, 159
// entry points of interrupts

#include <spede/machine/asmacros.h>
#include "entry.h"

// set DS and ES to Kernel's data segment
#define SET_KERNAL_SEGS movw $(SEL_KDATA), %ax; mov %ax, %ds; mov %ax, %es

.comm kernal_stack, KERNAL_STACK_SIZE, 1 // define kernel stack space
.text

// Loader() loads registers from trap frame
ENTRY(Loader)
   movl 4(%esp), %eax   // load stack pointer from eax
   movl %eax, %esp
   popl %gs
   popl %fs
   popl %es
   popl %ds
   popa                 // restore general registers
   add $4, %esp         // skip 4 bytes that stored the intr_id
   iret

// push intr type number then jump to common handler
ENTRY(TimerEntry)
   pushl $TIMER_INTR
   jmp EnterKernel

// Common IRQ entry, save context and call Kernel()
EnterKernel:
   pusha
   pushl %ds
   pushl %es
   pushl %fs
   pushl %gs
   movl %esp, %ecx
   cld
   SET_KERNAL_SEGS      // set the DS and ES to kernel's data segments
   leal kernal_stack + KERNAL_STACK_SIZE, %esp
   pushl %ecx
   call CNAME(Kernel)
