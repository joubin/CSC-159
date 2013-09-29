// main.c, 159
// simulated kernel with event/interrupt handling

#include "spede.h"
#include "types.h"
#include "main.h"
#include "isr.h"
#include "q_mgmt.h"
#include "proc.h" // SimpleProc()

// globals:
int cur_pid;                        // current running PID, -1 means no process
q_t ready_q, avail_q;               // processes ready to run and not used
pcb_t pcbs[NUM_PROC];               // process table
char user_stacks[NUM_PROC][USER_STACK_SIZE]; // run-time stacks for processes

void main()
{
   InitData();  // initialize needed data for kernel

   SpawnISR(0); // create IdleProc for OS to run if no user processes

   while(1)     // this is a simulated OS, real OS just loads a process
   {
      Scheduler();  // scheduler to select a process to run
      SimpleProc(); // simulated user process run
      Kernel();     // calling kernel to control
   }
}

void InitData()
{
   int i;

// queue initializations, both queues are empty first
   InitQ(&avail_q);
   InitQ(&ready_q);

   for(i=1; i<NUM_PROC; i++) // init pcbs[], skip 0 since it's Idle Proc
   {
      pcbs[i].state = AVAIL;
      EnQ(i, &avail_q);
   }

   cur_pid = -1;  // no process is running initially
}

void Scheduler() // this is kernel's process scheduler, simple round robin
{
   if(cur_pid > 0) return; // when cur_pid is not 0 (IdleProc) or -1 (none)

   if(cur_pid == 0) pcbs[0].state = READY; // skip when cur_pid is -1

   if(EmptyQ(&ready_q)) cur_pid = 0; // ready q empty, use IdleProc
   else cur_pid = DeQ(&ready_q);     // or get 1st process from ready_q

   pcbs[cur_pid].state = RUN;   // RUN state for newly selected process
}

void Kernel() // kernel begins its control upon/after interrupt
{
   int pid;
   char key;

   TimerISR(); // service the simulated timer interrupt

   if(cons_kbhit()) // if keyboard on the target PC has been pressed
   {
      key = cons_getchar(); // get the pressed key

      switch(key) // see if it's one of the following for service
      {
         case 'n':
            if(EmptyQ(&avail_q))
               cons_printf("No more process!\n");
            else
            {
               pid = DeQ(&avail_q);
               SpawnISR(pid);
            }
            break;
         case 'k': KillISR(); break;
         case 's': ShowStatusISR(); break;
         case 'b': breakpoint(); break; // this stops when run in GDB mode
         case 'q': exit(0);
      } // switch(key)
   } // if(cons_kbhit())
} // Kernel()

