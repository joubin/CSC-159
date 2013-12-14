// isr.c, 159
// ISRs called from Kernel()

#include "spede.h"
#include "types.h"
#include "isr.h"
#include "q_mgmt.h"
#include "externs.h"
#include "proc.h"

void MyBzero(void *s, int n) {
	int i;
	for (i = 0; i < n; i++) {
		*((char*)s+n) = '\0';
	}
}

void SpawnISR(int pid, func_ptr_t addr)
{
	MyBzero((void *)user_stacks[pid], USER_STACK_SIZE);
	MyBzero(&mboxes[pid], sizeof(mbox_t));
	// 1st. point to just above of user stack, then drop by 64 bytes (tf_t)
	pcbs[pid].tf_p = (tf_t *)&user_stacks[pid][USER_STACK_SIZE];
	pcbs[pid].tf_p--;    // pointer arithmetic, now points to trapframe

	// fill in CPU's register context

	pcbs[pid].tf_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
	pcbs[pid].tf_p->eip = (unsigned int) addr; // new process code
	pcbs[pid].tf_p->cs = get_cs();
	pcbs[pid].tf_p->ds = get_ds();
	pcbs[pid].tf_p->es = get_es();
	pcbs[pid].tf_p->fs = get_fs();
	pcbs[pid].tf_p->gs = get_gs();

	pcbs[pid].tick_count = pcbs[pid].total_tick_count = 0;
	pcbs[pid].state = READY;
   	pcbs[pid].cr3 = kernel_cr3;

	if(pid != 0) EnQ(pid, &ready_q);  // IdleProc (PID 0) is not queued
}


void KillISR()
{
	if(cur_pid < 1) return; // skip Idle Proc or when cur_pid has been set to -1

	pcbs[cur_pid].state = AVAIL;
	EnQ(cur_pid, &avail_q);
	cur_pid = -1;          // no proc running any more
}

void ShowStatusISR()
{
	int i;
	char *state_names[] = {"AVAIL\0", "READY\0", "RUN\0", "SLEEP\0", "WAIT\0"};

	for(i=0; i<NUM_PROC; i++)
	{
		printf("pid %i, state %s, tick_count %i, total_tick_count %i \n",
				i,
				state_names[pcbs[i].state],
				pcbs[i].tick_count,
				pcbs[i].total_tick_count);
	}

	printf("READY: ");
	for (i=0; i<NUM_PROC; i++)
	{
		printf("%i ", ready_q.q[i]);
	}
	printf("\n       Head: %i  Tail: %i  Count: %i\n", ready_q.head, ready_q.tail, ready_q.count);

	printf("AVAIL: ");
	for (i=0; i<NUM_PROC; i++)
	{
		printf("%i ", avail_q.q[i]);
	}
	printf("\n       Head: %i  Tail: %i  Count: %i\n", avail_q.head, avail_q.tail, avail_q.count);

}

void TimerISR()
{
	outportb(0x20, 0x60); // dismiss timer interrupt

	// deal with sleep items
	sys_tick++;


	while(!EmptyQ(&sleep_q) && (pcbs[sleep_q.q[sleep_q.head]].wake_tick <= sys_tick)) {
		int tmpPID = DeQ(&sleep_q);
		pcbs[tmpPID].state=READY;
		EnQ(tmpPID, &ready_q);
	}

	if(cur_pid == 0) return; // if Idle process, no need to do this on it

	pcbs[cur_pid].tick_count++;


	if(pcbs[cur_pid].tick_count == TIME_SLICE) // running up time, preempt it?
	{
		pcbs[cur_pid].tick_count = 0; // reset (roll over) usage time
		pcbs[cur_pid].total_tick_count += TIME_SLICE; // sum to total

		pcbs[cur_pid].state = READY;  // change its state
		EnQ(cur_pid, &ready_q);       // move it to ready_q
		cur_pid = -1;                 // no longer running
	}
}

void SleepISR(int sleep_secs){
	q_t tmp_q;
	InitQ(&tmp_q);
	pcbs[cur_pid].wake_tick = (sys_tick + sleep_secs * 100);
	while( !(EmptyQ(&sleep_q)) &&
			(pcbs[sleep_q.q[sleep_q.head]].wake_tick <= pcbs[cur_pid].wake_tick)         ){
		int tmpPID= DeQ(&sleep_q);
		EnQ(tmpPID, &tmp_q);
	}
	EnQ(cur_pid, &tmp_q);
	while (!EmptyQ(&sleep_q)){
		EnQ(DeQ(&sleep_q), &tmp_q);
	}
	while(!EmptyQ(&tmp_q)){
		EnQ(DeQ(&tmp_q), &sleep_q);
	}
	pcbs[cur_pid].state = SLEEP;
	cur_pid = -1;
}

int SemInitISR(int sem_count){
	int sid;
	if (!EmptyQ(&avail_sem_q)) {
		sid = DeQ(&avail_sem_q);
		sems[sid].sem_count = sem_count;
		InitQ(&(sems[sid].wait_q));
	}
	else {
		sid = -1;
	}
	return sid;
}

void SemWaitISR(int sid){
	if (sems[sid].sem_count > 0) {
		sems[sid].sem_count -= 1;
	}
	else {
		pcbs[cur_pid].state = WAIT;
		EnQ(cur_pid, &(sems[sid].wait_q));
		cur_pid = -1;
	}
}

void SemPostISR(int sid){
	if (!EmptyQ(&(sems[sid].wait_q))) {
		int free_pid = DeQ(&(sems[sid].wait_q));
		pcbs[free_pid].state = READY;
		EnQ(free_pid, &ready_q);
	}
	else {
		sems[sid].sem_count += 1;
	}
}

void MsgRcvISR()
{
	int mid = cur_pid;
	msg_t *source, *destination = (msg_t *)pcbs[cur_pid].tf_p->eax;
	int tmp;
	if(!MsgQEmpty(&mboxes[mid].msg_q))
	{
		source = DeQMsg(&mboxes[mid].msg_q);
		memcpy((char*)destination,(char*)source,sizeof(msg_t));
		tmp = DeQ( &(mboxes[mid].wait_q));

		set_cr3(pcbs[tmp].cr3);
		memcpy((msg_t*)pcbs[tmp].tf_p->eax, source, sizeof(msg_t));
	    set_cr3(pcbs[mid].cr3);
	}
	else
	{
		EnQ(cur_pid, &mboxes[mid].wait_q);
		pcbs[cur_pid].state = WAIT;
		cur_pid=-1;
	}
}

void MsgSndISR()
{
	int mid,pid,head;
	msg_t *source, *destination;

	mid = pcbs[cur_pid].tf_p ->eax;
	source = (msg_t*)pcbs[cur_pid].tf_p -> ebx;

    source->sender = cur_pid;
    source->send_tick = sys_tick;

	if(!EmptyQ(&mboxes[mid].wait_q))
	{
		pid = DeQ(&mboxes[mid].wait_q);
		EnQ(pid,&ready_q);
		pcbs[pid].state = READY;

		destination = (msg_t *)pcbs[pid].tf_p -> eax;
		MyMemCpy((char*)destination,(char*)source,sizeof(msg_t));
	}
	else
	{
		EnQMsg(source, &mboxes[mid].msg_q);
		head = mboxes[mid].msg_q.head;
		destination = &mboxes[mid].msg_q.msgs[head];
	}
}

void IRQ7ISR() {
   SemPostISR(print_sid); // perform SemPostISR directly from here
   outportb(0x20, 0x67);  // 0x20 is PIC control, 0x67 dismisses IRQ 7
}

void ForkISR(int pid, int* addr, int size, int value)
{
	// Only thing new for ForkISR
	int i, t_pid;
    int * p;
    int ramPages[5];
    unsigned index;
    int j = 0;
    t_pid = -1;
	for(i=0;i<NUM_PAGE;i++) //TODO <-- Can I add both I and J like that?
	{
		if(pages[i].owner == -1)
			{
				ramPages[j++] = i;
				if (j >= 5)
				{
					break;
				}
			}
	}
	t_pid = DeQ(&avail_q);
	// check ram pages
	if (j != 5)
	{	
		pcbs[cur_pid].tf_p->eax = -1; // TODO should be NOT_OK from opcodes
		return;
	}
	pcbs[cur_pid].tf_p->eax = t_pid;
	if (t_pid == -1)
	{	
		pcbs[cur_pid].tf_p->eax = -1;
		return;
	}
	// The rest was a copy from spwnisr.
	// pages[i].owner = pid;
	// MyBzero((char *)VSTART, USER_STACK_SIZE);

	// MyBzero((void *)user_stacks[pid], USER_STACK_SIZE);
	// MyBzero(&mboxes[pid], sizeof(mbox_t));
	for (i = 0; i < j; ++i)
	{
		pages[ramPages[i]].owner = t_pid;
		MyBzero((void*)pages[ramPages[i]].addr, USER_STACK_SIZE);
	}


	// Main Table (MT): entries to look for subtables.
	// Code Table (CT): entries to look for code pages.
	// Stack Table (ST): entries to look for stack pages.
	// Code Page (CP): executable code.
	// Stack Page (SP): runtime stack, initially a trapframe.


	p = (int*)pages[ramPages[0]].addr;
	memcpy((void*)p, (void*)kernel_cr3, 48);	// first 16*3 from kernel pd
	index = (unsigned int)VSTART >> 22;		//  first 10 bits of cp
	*(p + index) = pages[ramPages[1]].addr + 3;		// ct to pd
	index = (unsigned int)(VEND - sizeof(int) - sizeof(tf_t)) >> 22;	
	*(p + index) = pages[ramPages[2]].addr + 3;		// put st address into pd
	
	p = (int*)pages[ramPages[1]].addr;
	index = (unsigned int)( VSTART & 0x003FFFFF ) >> 12;	// grab the first 10 bits of virtual memory
	*(p + index) = pages[ramPages[3]].addr + 3;		
	
	p = (int*)pages[ramPages[2]].addr;
	index = (unsigned int)( (VEND - sizeof(int) - sizeof(tf_t)) & 0x003FFFFF ) >> 12;	//second 10 bits of CT
	*(p + index) = pages[ramPages[4]].addr + 3;		// sp == st
	
	memcpy((void*)pages[ramPages[3]].addr, addr, size);

    p = (int *) (VSTART + USER_STACK_SIZE);
    p--;
    *p = value;

	pcbs[pid].tf_p = (tf_t*) p;
	pcbs[pid].tf_p--;    // points to trap frame

    memcpy((char *)(pages[i].addr), (char *)(addr), size);

	pcbs[pid].tf_p->eflags = EF_DEFAULT_VALUE|EF_INTR;

	pcbs[pid].tf_p->eip = VSTART;
	pcbs[pid].tf_p->cs = get_cs();
	pcbs[pid].tf_p->ds = get_ds();
	pcbs[pid].tf_p->es = get_es();
	pcbs[pid].tf_p->fs = get_fs();
	pcbs[pid].tf_p->gs = get_gs();
	pcbs[pid].tf_p = (tf_t*)(VEND - sizeof(int) - sizeof(tf_t));

	pcbs[pid].tick_count = pcbs[pid].total_tick_count = 0;
	pcbs[pid].state = READY;
	pcbs[pid].ppid = cur_pid;
	pcbs[pid].cr3 = pages[ramPages[0]].addr;
	EnQ(pid, &ready_q);
}

void WaitISR()
{
	int i;
	for(i=0;i<NUM_PROC;i++)
	{
		if(pcbs[i].ppid == cur_pid && pcbs[i].state == ZOMBIE)
		{
			pcbs[cur_pid].tf_p->eax = pcbs[i].exit_code;
			EnQ(i,&avail_q);
			pcbs[i].ppid = -1;
			return;
		}
	}
	pcbs[cur_pid].state = WAIT_CHILD;
	cur_pid=-1;
}

void ExitISR()
{
	int ppid = pcbs[cur_pid].ppid;

	if(ppid == -1)
	{
		pcbs[cur_pid].exit_code = pcbs[cur_pid].tf_p->eax;
		pcbs[cur_pid].state = ZOMBIE;
		pages[cur_pid].owner = -1;
		cur_pid = -1;
		return;
	}
	pcbs[ppid].state = READY;
	EnQ(ppid,&ready_q);
	pcbs[ppid].tf_p->eax = pcbs[cur_pid].tf_p->eax;
	pages[cur_pid].owner = -1;
	EnQ(cur_pid,&avail_q);
	cur_pid = -1;
}
