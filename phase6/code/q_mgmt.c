// q_mgmt.c, 159

#include "spede.h"
#include "types.h"
#include "externs.h"
#include "q_mgmt.h"

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
	return(p->count == NUM_MSG);
}

int MsgQEmpty(msg_q_t *p)
{
	return(p->count == 0);
}

void EnQMsg(msg_t *p, msg_q_t *q)
{
	int t_p = q->tail;
	if (MsgQFull(q)) {
		cons_printf("Message queue is full, can't enqueue!\n");
		return;
	}
	MyMemCpy((char*)&(q->msgs[t_p]),(char*)p,sizeof(msg_t));
	q->tail++;
	if(q->tail >= NUM_MSG)
		q->tail = 0;
	q->count++;
}

msg_t *DeQMsg(msg_q_t *p)
{
	msg_t *msg;
	int h_p = p -> head;

	if(p->count > 0)
	{
		msg = (msg_t*)&(p->msgs[h_p]);
		p->head++;
		if(p->head >= Q_SIZE)
			p->head = 0;
		p->count--;
	}
	else
		msg = 0;
	return msg;
}

void CharInitQ(char_q_t * p)
{
	p->count = p->head = p->tail = 0;
}

int CharEmptyQ(char_q_t *p)
{
	return (p->count == 0);
}

int CharFullQ(char_q_t *p)
{
	return (p->count == CHAR_Q_SIZE);
}

char CharDeQ(char_q_t *p) // return -1 if q is empty
{
	char result;

	if (CharEmptyQ(p)) {
		cons_printf("Queue is empty, can't dequeue!\n");
		return -1;
	}

	result = p->q[p->head];

	p->head += 1;

	if (p->head >= Q_SIZE) {
		p->head = 0;
	}

	p->count -= 1;

	return result;
}

void CharEnQ(char element, char_q_t *p)
{
	if (CharFullQ(p)) {
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

void MyBZero(char *p, int size)
{
   while( size-- ) *p++ = 0; // they'll be all nulls
}

void MyMemCpy(char *dest, char *src, int size)
{
   while(size--) *dest++ = *src++;
}

void MyStrCpy(char *dest, char *src)
{
	while(*src != '\0') *dest++ = *src++;
	*dest = '\0';
}

int MyStrCmp(char *first, char *second)
{
	do
	{
		if ((*first == '\0') && (*second == '\0'))
		{
			return 1;
		}
	} while (*(first++) == *(second++));
	return 0;
}
