// main.c, 159
// simulated kernel with event/interrupt handling
// Team Name: walls
// Members: Joubin Jabbari and Robert Wall

#include "spede.h"
#include "types.h"
#include "main.h"
#include "isr.h"
#include "q_mgmt.h"
#include "proc.h" // SimpleProc()
#include "entry.h"

// globals:
int cur_pid, sys_tick;                        // current running PID, -1 means no process
char user_stacks[NUM_PROC][USER_STACK_SIZE]; // run-time stacks for processes
struct i386_gate *idt_table;
int print_sid;	// printing flow/control semaphore

q_t avail_sem_q;
mbox_t mboxes[NUM_PROC];			// Mailboxes
page_t	pages[NUM_PAGE];   			// Pages
sem_t sems[NUM_SEM];				// semaphores 
pcb_t pcbs[NUM_PROC];               // process table
q_t ready_q, avail_q, sleep_q;      // processes ready to run and not used

void SetIDTEntry(int entry_num, func_ptr_t entry_addr){
	struct i386_gate *gateptr = &idt_table[entry_num];
	fill_gate(gateptr, (int)entry_addr, get_cs(), ACC_INTR_GATE,0);
}
void main()
{

	InitData();  // initialize needed data for kernel

	InitControl();

	SpawnISR(0, IdleProc); // create IdleProc for OS to run if no user processes
	SpawnISR(1, Init);

	cur_pid = 0;

	Loader(pcbs[0].tf_p);
}

void InitControl()
{
	idt_table = get_idt_base();
	SetIDTEntry(TIMER_INTR, TimerEntry);
	SetIDTEntry(IRQ7_INTR, IRQ7Entry);
	SetIDTEntry(GETPID_INTR, GetPidEntry);
	SetIDTEntry(SLEEP_INTR, SleepEntry);
	SetIDTEntry(SPAWN_INTR, SpawnEntry);
	SetIDTEntry(SEMINIT_INTR, SemInitEntry);
	SetIDTEntry(SEMWAIT_INTR, SemWaitEntry);
	SetIDTEntry(SEMPOST_INTR, SemPostEntry);
	SetIDTEntry(MSGSND_INTR, MsgSndEntry);
	SetIDTEntry(MSGRCV_INTR, MsgRcvEntry);
	SetIDTEntry(IRQ3_INTR, IRQ3Entry);
	SetIDTEntry(IRQ4_INTR, IRQ4Entry);
	SetIDTEntry(FORK_INTR, ForkEntry);
	SetIDTEntry(WAIT_INTR, WaitEntry);
	SetIDTEntry(EXIT_INTR, ExitEntry);
	// 0x66 = 0110 0110 in binary
	// PIC mask will mask out any IRQs set to 1, starting from IRQ0,
	// so 0x66 masks IRQ 1, 2, 5, and 6
	outportb(0x21, 0x66);

}

void InitData()
{
	int i;

	// queue initializations, both queues are empty first
	InitQ(&avail_q);
	InitQ(&ready_q);
	InitQ(&avail_sem_q);

	for(i=2; i < NUM_PROC ; i++) // init pcbs[], skip 0 since it's Idle Proc
	{
		pcbs[i].state = AVAIL;
		EnQ(i, &avail_q);
	}

	for(i=NUM_SEM-1; i >= 0; i--) // init pcbs[], skip 0 since it's Idle Proc
	{
		EnQ(i, &avail_sem_q);

	}

	for(i=NUM_PAGE ; i > 0; i--)
	{
		pages[i].addr = (i*4096) + _topHeapMemory;
		pages[i].owner = -1;
	}

	cur_pid = -1;  // no process is running initially

	sys_tick = 0; // Set sys_tick to 0
	InitQ(&sleep_q); // reset sleep_q to empty

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
		case IRQ7_INTR:
			IRQ7ISR();
			break;
		case SLEEP_INTR:
			SleepISR(tf_p->eax);
			break;
		case GETPID_INTR:
			tf_p->eax = cur_pid;
			break;
		case SPAWN_INTR:
			if (EmptyQ(&avail_q)) {
				cons_printf("No more available PIDs!\n");
				tf_p->ebx = -1;
			}
			else {
				tf_p->ebx = DeQ(&avail_q);
				SpawnISR((int) tf_p->ebx, (func_ptr_t) tf_p->eax);
			}
			break;
		case SEMINIT_INTR:
			tf_p->ebx = SemInitISR(tf_p->eax);
			break;
		case SEMWAIT_INTR:
			SemWaitISR(tf_p->eax);
			break;
		case SEMPOST_INTR:
			SemPostISR(tf_p->eax);
			break;
		case MSGSND_INTR:
			MsgSndISR();
			break;
		case MSGRCV_INTR:
			MsgRcvISR();
			break;
		case IRQ3_INTR:
			IRQ34ISR();
			break;
		case IRQ4_INTR:
			IRQ34ISR();
			break;
		case FORK_INTR:
			if(!EmptyQ(&avail_q))
			{	
				pid = DeQ(&avail_q);
				ForkISR(pid,(int*)pcbs[cur_pid].tf_p->eax,pcbs[cur_pid].tf_p->ebx);
				pcbs[cur_pid].tf_p->eax=pid;
			}
			else
			{
				cons_printf("\nNo more available processes");
				pcbs[cur_pid].tf_p->eax = -1;
			}			
			break;
		case WAIT_INTR:
			WaitISR();
			break;
		case EXIT_INTR:
			ExitISR();
			break;
}

	Scheduler();                // select a process to run
	Loader(pcbs[cur_pid].tf_p); // run the process selected
}
