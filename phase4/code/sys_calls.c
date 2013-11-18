#include "types.h"

int GetPid()
{
<<<<<<< HEAD
    int pid;

    asm("int $0x30; movl %%eax, %0"
            : "=g" (pid)
            :
            : "eax");

    return pid;
=======
	int pid;

	asm("int $0x30; movl %%eax, %0"
			: "=g" (pid)
			:
			: "eax");

	return pid;
>>>>>>> b8a9e70220cc217dedb00b209013c8d21b381054
}

void Sleep(int sleep_secs) // sleep for how many secs
{
<<<<<<< HEAD
    asm("movl %0, %%eax; int $0x31"  // copy secs to eax, call intr
            :                            // no output
            : "g" (sleep_secs)           // 1 input
            : "eax");                    // 1 overwritten register
=======
	asm("movl %0, %%eax; int $0x31"  // copy secs to eax, call intr
			:                            // no output
			: "g" (sleep_secs)           // 1 input
			: "eax");                    // 1 overwritten register
>>>>>>> b8a9e70220cc217dedb00b209013c8d21b381054
}

int Spawn(func_ptr_t addr) {
	int pid;

	asm("movl %1, %%eax; int $0x32; movl %%ebx, %0"
<<<<<<< HEAD
		: "=g" (pid)
		: "g" (addr)
		: "eax", "ebx");
=======
			: "=g" (pid)
			: "g" (addr)
			: "eax", "ebx");
>>>>>>> b8a9e70220cc217dedb00b209013c8d21b381054

	return pid;
}
int SemInit(int sem_count) {
	int sid;

	asm("movl %1, %%eax; int $0x33; movl %%ebx, %0"
<<<<<<< HEAD
		: "=g" (sid)
		: "g" (sem_count)
		: "eax", "ebx");
=======
			: "=g" (sid)
			: "g" (sem_count)
			: "eax", "ebx");
>>>>>>> b8a9e70220cc217dedb00b209013c8d21b381054

	return sid;
}
void SemWait(int sid) {
	asm("movl %0, %%eax; int $0x34"
<<<<<<< HEAD
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
=======
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
void MsgSnd(int mid, msg_t * msg) {
	asm("movl %0, %%eax; movl %1, %%ebx; int $0x36"
			:
			: "g" (mid), "g" (msg)
			: "eax", "ebx");
}
void MsgRcv(int mid, msg_t * msg) {
	asm("movl %0, %%eax; movl %1, %%ebx; int $0x37"
			:
			: "g" (mid), "g" (msg)
			: "eax", "ebx");
}
>>>>>>> b8a9e70220cc217dedb00b209013c8d21b381054
