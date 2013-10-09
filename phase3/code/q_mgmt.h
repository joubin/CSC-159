// q_mgmt.h, 159

#ifndef _Q_MGMT_H_
#define _Q_MGMT_H_

#include "types.h" // defining q_t needed below

int EmptyQ(q_t *);
int FullQ(q_t *);
void InitQ(q_t *);
int DeQ(q_t *);
void EnQ(int, q_t *);

#endif

