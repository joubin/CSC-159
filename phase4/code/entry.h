// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR 0x20
#define GETPID_INTR 0x30
#define SLEEP_INTR 0x31

#define SPAWN_INTR  0x32
#define SEMINIT_INTR 0x33
#define SEMWAIT_INTR 0x34
#define SEMPOST_INTR 0x35
#define MSGSND_INTR 0x36 // Message sent mailbox
#define MSGRCV_INTR 0x37 // Message recive mailbox
#define SEL_KCODE 0x08    // kernel's code segment
#define SEL_KDATA 0x10    // kernel's data segment
#define KERNAL_STACK_SIZE 8192  // kernel's stack size, in chars


7

// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

#include "types.h" // for tf_t below

extern void TimerEntry();     // code defined in entry.S
extern void Loader(tf_t *);   // code defined in entry.S
extern void GetPidEntry();
extern void SleepEntry();

extern void SpawnEntry();
extern void SemInitEntry();
extern void SemWaitEntry();
extern void SemPostEntry();

__END_DECLS

#endif

#endif

