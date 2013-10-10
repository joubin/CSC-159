// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

void SpawnISR(int, func_ptr_t);
void KillISR();
void TimerISR();
void ShowStatusISR();
void SleepISR(int);

void SemInitISR(int);
void SemWaitISR(int);
void SemPostISR(int);
#endif
