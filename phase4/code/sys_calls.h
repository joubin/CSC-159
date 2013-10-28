#ifndef _SYS_CALLS_H_
#define _SYS_CALLS_H_

int GetPid();
void Sleep(int);
int Spawn(func_ptr_t);
int SemInit(int);
void SemWait(int);
void SemPost(int);
#endif
