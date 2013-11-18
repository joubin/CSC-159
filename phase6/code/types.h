// types.h, 159

#ifndef _TYPES_H_
#define _TYPES_H_

#include "trap.h"            // defines tf_t, trap frame type

#define TIME_SLICE 3       // max runtime per run per proc, 2 secs
#define NUM_PROC 20          // max number of processes allowed in the system
#define Q_SIZE 20            // queuing capacity
#define USER_STACK_SIZE 4096 // # of bytes for runtime stack
#define NUM_SEM 20 // max # of semaphors

// To handle messages
#define NUM_MSG 20     // # of msgs in msg_q of a mailbox
#define NUM_NUM 6      // # of numbers in int array of a message
#define NUM_BYTE 101   // # of chars in str of a message

#define BUFF_SIZE 3    // size of bounded buffer, # of msgs

typedef enum {AVAIL, READY, RUN, SLEEP, WAIT} state_t;

typedef struct            // PCB describes proc image
{
	state_t state;         // state of process
	int tick_count,        // accumulative run time since dispatched
	    total_tick_count,
	    wake_tick;  // accumulative run time since creation

	tf_t *tf_p;            // process register context
} pcb_t;

typedef struct // queue type: head & tail pointing array of PIDs
{
	int count, head, tail, q[Q_SIZE];
} q_t;

typedef void (* func_ptr_t)(); // void-returning function pointer type

typedef struct{
	int sem_count; // sem count
	q_t wait_q; // process wait for q
} sem_t;

typedef struct
{
	int sender;                 // sender
	int send_tick;              // time sent
	int numbers[NUM_NUM];       // several integers can be useful as msgs
	char bytes[NUM_BYTE];       // some chars can be useful as msgs
} msg_t;                       // msg type

typedef struct
{
	int count, head, tail;
	msg_t msgs[NUM_MSG];
} msg_q_t;

typedef struct
{
	q_t wait_q;
	msg_q_t msg_q;
} mbox_t;

#endif

