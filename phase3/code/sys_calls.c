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
