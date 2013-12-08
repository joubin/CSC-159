// proc.c, 159
// processes are here

#include "spede.h"   // for IO_DELAY() needed here below
#include "externs.h" // for cur_pid needed here below
#include "proc.h"
#include "sys_calls.h"
#include "q_mgmt.h"
#include "irq7.h"
#include "irq34.h"
#include "shell_cmds.h"
#include "filesys.h"

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
	int filesys_pid = Spawn(FileSys);
	int shell1_pid = Spawn(Shell);
	int shell2_pid = Spawn(Shell);
	msg_t msg;
	msg.numbers[0] = printdriver_pid;
	msg.numbers[1] = 0;
	msg.numbers[2] = filesys_pid;
	MsgSnd(shell1_pid, &msg);
	msg.numbers[1] = 1;
	MsgSnd(shell2_pid, &msg);
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

void Shell()
{
	int term_num, stdin_pid, stdout_pid, print_driver_pid, file_sys_pid;
	char login[50], passwd[50], cmd_str[50];
	msg_t msg;

	MsgRcv(&msg);

	term_num = msg.numbers[1];
	print_driver_pid = msg.numbers[0];
	file_sys_pid = msg.numbers[2];
	TerminalInit(term_num);
	
	stdin_pid = Spawn(Stdin);
	stdout_pid = Spawn(Stdout);

	MsgSnd(stdin_pid, &msg);
	MsgSnd(stdout_pid, &msg);

	while(1)
	{
		while(1)
		{
			// Tell stdout to display login: and block on completion
			MyStrCpy(msg.bytes,"login: ");
			MsgSnd(stdout_pid, &msg);
			MsgRcv(&msg);

			// Ask stdin to get typed login and store in login
			MsgSnd(stdin_pid, &msg);
			MsgRcv(&msg);
			MyStrCpy(login, msg.bytes);

			// Tell stdout to display password: and block on completion
			MyStrCpy(msg.bytes,"password: ");
			MsgSnd(stdout_pid, &msg);
			MsgRcv(&msg);

			// Ask stdin to get typed password and store in passwd
			MsgSnd(stdin_pid, &msg);
			MsgRcv(&msg);
			MyStrCpy(passwd, msg.bytes);
			
			if (MyStrCmp(login,passwd))
			{
				break;
			}

			// Tell stdout to display error and block on completion
			MyStrCpy(msg.bytes,"Illegal login and password!\n\0");
			MsgSnd(stdout_pid, &msg);
			MsgRcv(&msg);
		}

		while(1)
		{
			// Tell stdout to display prompt and block on completion
			MyStrCpy(msg.bytes,"walls > ");
			MsgSnd(stdout_pid, &msg);
			MsgRcv(&msg);

			// Ask stdin to get typed password and store in passwd
			MsgSnd(stdin_pid, &msg);
			MsgRcv(&msg);
			MyStrCpy(cmd_str, msg.bytes);

				if (MyStrCmp(cmd_str, "help") == 1) // if help is typed in
				{
					ShellHelp(stdout_pid);
				}
				else if (MyStrCmp(cmd_str, "who") == 1) // if who is typed in
				{
					ShellWho(stdout_pid);
				}
				else if ((MyStrCmp(cmd_str, "bye") == 1) || (MyStrCmp(cmd_str, "logout") == 1)) // if bye or logout is typed in
				{
					MyStrCpy(msg.bytes,"Have a nice day!!\n");
					MsgSnd(stdout_pid,&msg);
					MsgRcv(&msg);
					break;
				}
				else if (MyMemCmp(cmd_str, "type", 4) == 1) // if type is typed in
				{
					ShellType(cmd_str, stdout_pid, file_sys_pid);
				}
				else if (MyMemCmp(cmd_str, "print", 5) == 1) // if print is typed in
				{
					ShellPrint(cmd_str,stdout_pid,print_driver_pid, file_sys_pid); 
					/** adding the file_sys_pid even though it wasnt in the requirments so that errors
					* go to consol 
					**/
				}
				else if (MyMemCmp(cmd_str, "dir", 3) == 1) // if dir is typed in
				{
					ShellDir(cmd_str, stdout_pid, file_sys_pid);
				}
				else if (cmd_str[0] == '\0') // if nothing is typed in
				{
					continue;
				}else //otherwise, print and error since I dont know what this is.
				{
					// Display error and block on completion
					ShellExecutable(cmd_str,stdout_pid,file_sys_pid);
					//MyStrCpy(msg.bytes,"Invalid command!\n\0");
					//MsgSnd(stdout_pid, &msg);
					//MsgRcv(&msg);	
				}
		}
	}
}


