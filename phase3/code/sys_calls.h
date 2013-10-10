#ifndef _SYS_CALLS_H_
#define _SYS_CALLS_H_
int GetPid();
void Sleep(int);

int Spawn();
int SemInit();
void SemWait();
void SemPost();
#endif
