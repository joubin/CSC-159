// proc.c, 159
// processes are here

#include "sys_calls.h"
#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below

void IdleProc()
{
    int i;
    while(1)
    {
        cons_printf("0 ");
        for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
    }
}


void SimpleProc(){
    int pid;
    pid = GetPid();
    while(1)
    {
        int sleep_secs = (pid %5)+1; 
        cons_printf("%d ", pid);
        Sleep(sleep_secs);
    }
}
