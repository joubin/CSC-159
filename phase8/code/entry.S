// entry.S, 159
// entry points of interrupts

#include <spede/machine/asmacros.h>
#include "entry.h"

// set DS and ES to Kernel's data segment
#define SET_KERNEL_SEGS movw $(SEL_KDATA), %ax; mov %ax, %ds; mov %ax, %es

.comm kernel_stack, KERNEL_STACK_SIZE, 1 // define kernel stack space
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

ENTRY (IRQ7Entry)
	pushl $IRQ7_INTR
	jmp EnterKernel

ENTRY (IRQ3Entry)
	pushl $IRQ3_INTR
	jmp EnterKernel

ENTRY (IRQ4Entry)
	pushl $IRQ4_INTR
	jmp EnterKernel

ENTRY(GetPidEntry)
	pushl $GETPID_INTR
	jmp EnterKernel

ENTRY(SleepEntry)
	pushl $SLEEP_INTR
	jmp EnterKernel

ENTRY(SpawnEntry)
	pushl $SPAWN_INTR
	jmp EnterKernel

ENTRY(SemInitEntry)
	pushl $SEMINIT_INTR
	jmp EnterKernel

ENTRY(SemWaitEntry)
	pushl $SEMWAIT_INTR
	jmp EnterKernel

ENTRY(SemPostEntry)
	pushl $SEMPOST_INTR
	jmp EnterKernel

ENTRY(MsgSndEntry)
	pushl $MSGSND_INTR
	jmp EnterKernel

ENTRY(MsgRcvEntry)
	pushl $MSGRCV_INTR
	jmp EnterKernel


ENTRY(ForkEntry)
	pushl $FORK_INTR
	jmp EnterKernel

ENTRY(WaitEntry)
	pushl $WAIT_INTR
	jmp EnterKernel

ENTRY(ExitEntry)
	pushl $EXIT_INTR
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
	SET_KERNEL_SEGS      // set the DS and ES to kernel's data segments
	leal kernel_stack + KERNEL_STACK_SIZE, %esp
	pushl %ecx
	call CNAME(Kernel)
