// externs.h, 159

#ifndef _EXTERNS_H_
#define _EXTERNS_H_

#include "types.h"              // q_t, pcb_t, NUM_PROC, USER_STACK_SIZE

extern int cur_pid;             // PID of running process, -1 is none running
extern q_t ready_q, avail_q, sleep_q;    // ready to run, not used proc IDs
extern pcb_t pcbs[NUM_PROC];    // process table
extern char user_stacks[NUM_PROC][USER_STACK_SIZE]; // proc run-time stacks
extern int sys_tick;

extern int common_sid;
extern int product_num;
extern sem_t sems[NUM_SEM];
extern q_t avail_sem_q;
extern mbox_t mboxes[NUM_PROC];            // Mailboxes

extern int print_sid;

#endif
