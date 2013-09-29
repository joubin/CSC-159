// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below

void SimpleProc() // a simulated user process (also as the IdleProc)
{
   int i;

   cons_printf("%d ", cur_pid);
   for(i=0; i<1666000; i++) IO_DELAY(); // delay for about 1 sec
}
