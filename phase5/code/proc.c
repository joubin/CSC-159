// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "proc.h"
#include "sys_calls.h"
#include "q_mgmt.h"
#include "irq7.h"

void IdleProc()
{
	int i;
	while(1)
	{
		cons_printf("0 ");
		for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
	}
}


void Init() // handles key events, move the handling code out of Kernel()
{
	int i;
	int printdriver_pid = Spawn(PrintDriver);
	msg_t msg;
	MyStrCpy(msg.bytes,"walls\n");
	while (1){
		cons_printf("%d ", GetPid());
		for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
		if (cons_kbhit()){
			char key = cons_getchar(); // get the pressed key
			switch(key) // see if it's one of the following for service
			{
				case 'p':
					MsgSnd(printdriver_pid, &msg);
					break;
				case 'b': breakpoint(); break; // this stops when run in GDB mode
				case 'q': exit(0);
			} // switch(key)
		} // if(cons_kbhit())

	}
}
