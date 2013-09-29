// types.h, 159

#ifndef _TYPES_H_
#define _TYPES_H_

#define TIME_SLICE 5         // max tick_count per run
#define NUM_PROC 20          // max number of processes in the system
#define Q_SIZE 20            // queuing capacity
#define USER_STACK_SIZE 4096 // run-time stack size per process

// this is the same as defining constants AVAIL=0, READY=1, RUN=2, etc.
typedef enum {AVAIL, READY, RUN, SLEEP, WAIT, WAIT_CHILD} state_t;

typedef struct               // PCB describes proc image
{
   state_t state;            // state of process
   int tick_count;           // accumulative run time since dispatched
   int total_tick_count;     // accumulative run time since created
} pcb_t;

typedef struct               // proc queue type
{
   int count, head, tail;    // count # of queued elements, head and tail are
   int q[Q_SIZE];            // indices into q[] array to place or get element
} q_t;

#endif
