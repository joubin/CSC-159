// externs.h, 159

#ifndef _EXTERNS_H_
#define _EXTERNS_H_

#include "types.h"              // q_t, pcb_t, NUM_PROC, USER_STACK_SIZE

extern int cur_pid;             // PID of running process, -1 is none running
extern q_t ready_q, avail_q;    // ready to run, not used proc IDs
extern pcb_t pcbs[NUM_PROC];    // process table
extern char user_stacks[NUM_PROC][USER_STACK_SIZE]; // proc run-time stacks

#endif
