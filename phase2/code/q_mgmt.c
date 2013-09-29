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
	return (p->count == NUM_PROC);
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
		return -1;
	}
	
	pid = p->q[p->head];
	
	p->head += 1;

	if (p->head >= NUM_PROC) {
		p->head = 0;
	}

	p->count -= 1;

	return pid;
}

void EnQ(int element, q_t *p)
{
	if (FullQ(p)) {
		return;
	}
	p->q[p->tail] = element;

	p->tail += 1;

	if (p->tail >= NUM_PROC) {
		p->tail = 0;
	}

	p->count += 1;
}
