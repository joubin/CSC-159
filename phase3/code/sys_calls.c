#include "types.h"

int GetPid()
{
    int pid;

    asm("int $0x30; movl %%eax, %0"
            : "=g" (pid)
            :
            : "eax");

    return pid;
}

void Sleep(int sleep_secs) // sleep for how many secs
{
    asm("movl %0, %%eax; int $0x31"  // copy secs to eax, call intr
            :                            // no output
            : "g" (sleep_secs)           // 1 input
            : "eax");                    // 1 overwritten register
}

int Spawn(func_ptr_t addr) {
	int pid;
	asm("movl %0, %%eax; int $0x32"
		:
		: "g" (addr)
		: "eax");
	
	asm("movl %%eax, %0"
		: "=g" (pid)
		:
		: "eax");
	return pid;
}
int SemInit(int sem_count) {
	int sid;

	asm("movl %0, %%eax; int $0x33"
		:
		: "g" (sem_count)
		: "eax");

	asm("movl %%eax, %0"
		: "=g" (sid)
		:
		: "eax");
	return sid;
}
void SemWait(int sid) {
	asm("movl %0, %%eax; int $0x34"
		:
		: "g" (sid)
		: "eax");
}
void SemPost(int sid) {
	asm("movl %0, %%eax; int $0x35"
		:
		: "g" (sid)
		: "eax");

}
