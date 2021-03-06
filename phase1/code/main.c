// main.c, 159
// simulated kernel with event/interrupt handling
// Joubin Jabbari and Robert Wall
// Last edited: 2013-09-28 / phase 1

#include "spede.h"
#include "types.h"
#include "main.h"
#include "isr.h"
#include "q_mgmt.h"
#include "proc.h" // SimpleProc()
#include "entry.h"

// globals:
int cur_pid;                        // current running PID, -1 means no process
q_t ready_q, avail_q;               // processes ready to run and not used
pcb_t pcbs[NUM_PROC];               // process table
char user_stacks[NUM_PROC][USER_STACK_SIZE]; // run-time stacks for processes
struct i386_gate *idt_table;
void InitControl();
void SetIDTEntry(int entry_num, func_ptr_t entry_addr){
    struct i386_gate *gateptr = &idt_table[entry_num];
    fill_gate(gateptr, (int)entry_addr, get_cs(), ACC_INTR_GATE,0);
}
void main()
{
   InitData();  // initialize needed data for kernel

   InitControl();

   SpawnISR(0); // create IdleProc for OS to run if no user processes

   cur_pid = 0;
   EI();
   Loader(pcbs[0].tf_p);
}

void InitControl()
{
   idt_table = get_idt_base();
   SetIDTEntry(32, TimerEntry);
   outportb(0x21, ~0x01);
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

void Kernel(tf_t *tf_p) // kernel directly enters here when interrupt occurs
{
   // Save "tf_p" to pcbs[cur_pid].tf_p for future resume of process runtime
   pcbs[cur_pid].tf_p = tf_p;
   
   // tf_p->intr_id tells what intr made CPU get here, pushed in entry.S
   switch(tf_p->intr_id)
   {
      case TIMER_INTR:
         TimerISR(); // this must include dismissal of timer interrupt
         break;
   }

   // still handles other keyboard-generated simulated events
   if(cons_kbhit()) // check if a key was pressed (returns non zero)
   {
      char key = cons_getchar(); // get the pressed key

      switch(key) // see if it's one of the following for service
      {
         case 'n':
            if(EmptyQ(&avail_q))
               cons_printf("No more available PIDs!\n");
            else
            {
               SpawnISR(DeQ(&avail_q));
            }
            break;
         case 'k': KillISR(); break;
         case 's': ShowStatusISR(); break;
         case 'b': breakpoint(); break; // this stops when run in GDB mode
         case 'q': exit(0);
      } // switch(key)
   } // if(cons_kbhit())

   Scheduler();                // select a process to run
   Loader(pcbs[cur_pid].tf_p); // run the process selected
}
