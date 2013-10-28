// q_mgmt.c, 159

#include "spede.h"
#include "types.h"
#include "externs.h"

int EmptyQ(q_t *p)
{
	return (p->count == 0);
}

int FullQ(q_t *p)
{
	return (p->count == Q_SIZE);
}

void InitQ(q_t *p)
{
	p->count = 0;
	p->head = 0;
	p->tail = 0;
}

int DeQ(q_t *p) // return -1 if q is empty
{
	int pid;

	if (EmptyQ(p)) {
		cons_printf("Queue is empty, can't dequeue!\n");
		return -1;
	}

	pid = p->q[p->head];

	p->head += 1;

	if (p->head >= Q_SIZE) {
		p->head = 0;
	}

	p->count -= 1;

	return pid;
}

void EnQ(int element, q_t *p)
{
	if (FullQ(p)) {
		cons_printf("Queue is full, can't enqueue!\n");
		return;
	}
	p->q[p->tail] = element;

	p->tail += 1;

	if (p->tail >= Q_SIZE) {
		p->tail = 0;
	}

	p->count += 1;
}

int MsgQFull(msg_q_t *p)
{
	return(p->count == 20);
}

int MsgQEmpty(msg_q_t *p)
{
	return(p->count == 0);
}

void EnQMsg(msg_t *p, msg_q_t *q)
{
	int t_p = q->tail;
	memcpy(&(q->msgs[t_p]),p,sizeof(msg_t));
	if(q->tail == (Q_SIZE-1))
		q->tail = 0;
	else
		q->tail++;
	q->count++;
}

msg_t *DeQMsg(msg_q_t *p)
{
	msg_t *msg;
	int h_p = p -> head;


	if(p->count > 0)
	{
		msg = (msg_t*)&(p->msgs[h_p]);
		if(p->head == (Q_SIZE-1))
			p->head = 0;
		else
			p->head++;
		p->count--;
	}
	else
		msg = 0;
	return msg;
}
