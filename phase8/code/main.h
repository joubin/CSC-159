// main.h, 159

#ifndef _MAIN_H_
#define _MAIN_H_

void SetIDTEntry(int, func_ptr_t);
void main();
void InitControl();
void InitData();
void Scheduler();
void Kernel(tf_t *);

#endif
