// q_mgmt.h, 159

#ifndef _Q_MGMT_H_
#define _Q_MGMT_H_

#include "types.h" // defining q_t needed below

int EmptyQ(q_t *);
int FullQ(q_t *);
void InitQ(q_t *);
int DeQ(q_t *);
void EnQ(int, q_t *);

int MsgQFull(msg_q_t *);
int MsgQEmpty(msg_q_t *);
void EnQMsg(msg_t *, msg_q_t *);
msg_t *DeQMsg(msg_q_t *);

void CharInitQ(char_q_t *);
int CharFullQ(char_q_t *);
int CharEmptyQ(char_q_t *);
void CharEnQ(char, char_q_t *);
char CharDeQ(char_q_t *);

void MyBZero(char *, int);
void MyStrCpy(char *, char *);
void MyMemCpy(char *, char *, int);
int MyStrCmp(char *, char *);
int MyMemCmp(char *, char *, int);

#endif

