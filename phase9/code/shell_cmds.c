// shell_cmds.c, Phase 7, Sac State CpE/CSc 159
// shell subroutines to perform "dir/type/print" commands

#include "spede.h"
#include "types.h"
#include "sys_calls.h"
#include "op_codes.h" // operation codes: OK, STAT, etc.
#include "filesys.h"
#include "externs.h"
#include "q_mgmt.h"   // MyStrCmp(), etc.
#include "shell_cmds.h"

// build a string: a detail line of attributes of the given file/directory
// that p points to (stat_t)
void DirLine(stat_t *p, char *line) {
	// we get back in msg.bytes two parts: stat_t type and path
	// by type-casting "p+1" pointer, we get a pointer to the path
	char *path = (char *)(p + 1);

	// build output in the line from what p points
	sprintf(line, "----  size=%5d     %s\n", p->size, path);

	if ( S_ISDIR(p->mode) ) line[0] = 'd';  // mode is directory
	if ( QBIT_ON(p->mode, S_IROTH) ) line[1] = 'r'; // mode is readable
	if ( QBIT_ON(p->mode, S_IWOTH) ) line[2] = 'w'; // mode is writable
	if ( QBIT_ON(p->mode, S_IXOTH) ) line[3] = 'x'; // mode is executable
}

// do "dir" listing, Shell() communicates with FileSys() and Stdout()
// make sure str passed has trailing 0: "dir\0" or "dir something\0"
void ShellDir(char *str, int stdout_pid, int file_sys_pid) {
	char path[NUM_BYTE], line[NUM_BYTE];
	stat_t *p;
	msg_t msg;
	int fd;
	// if str is "dir\0" assume home (root) dir "/"
	// else, assume user specified a path after first 4 letters "dir "
	if(MyStrCmp(str, "dir\0") == 1) {
		path[0] = '/';
		path[1] = '\0'; // null-terminate the path[]
	} else { // skip 1st 4 letters "dir " and get the path
	str += 4;
		MyStrCpy(path, str); // make sure str is null-terminated from Shell()
	}



	MyStrCpy(msg.bytes, path); // copy whats left of the path
	msg.numbers[0] = STAT; // set the stat

	MsgSnd(file_sys_pid, &msg); // add tot he message queue
	MsgRcv(&msg); // read the return code

	if(msg.numbers[0] != OK) { //if not ok
		MsgSnd(stdout_pid, &msg); //send stdout_pid
		MsgRcv(&msg);

		return;        // we can't continue
	}


	p = (stat_t *)msg.bytes;    // path

	if( ! S_ISDIR(p->mode) ) // if not dir, it's a file, detail-list it
	{
		DirLine(p, line); // line is to be built and returned by subroutine
		MyStrCpy(msg.bytes, line);
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
		return;
	}

	MyStrCpy(msg.bytes, path);
	msg.numbers[0] = OPEN;
	MsgSnd(file_sys_pid, &msg);
	MsgRcv(&msg);
	fd = msg.numbers[2];
	while(msg.numbers[0] == OK) {

		msg.numbers[0] = READ;
		msg.numbers[2] = fd;
		MsgSnd(file_sys_pid, &msg);
		MsgRcv(&msg);
		p = (stat_t *)msg.bytes;
		DirLine(p, line);
		if (msg.numbers[0] != OK)
		{
			return; // we cant go past this if the DirLine doesnt work
		}
		MyStrCpy(msg.bytes, line);
		MsgSnd(stdout_pid,&msg);
		MsgRcv(&msg);

	}

	msg.numbers[0] = CLOSE;
	msg.numbers[2] = fd;
	MsgSnd(file_sys_pid, &msg);
	MsgRcv(&msg);


	if (msg.numbers[0] != OK)
	{
		MyStrCpy(msg.bytes, "Error: Can not close!\n\0");
		MsgSnd(stdout_pid,&msg);
		MsgRcv(&msg);
	}
}

void ShellType(char *str, int stdout_pid, int file_sys_pid) {
	char path[NUM_BYTE];
	stat_t *p;
	msg_t msg;


	if(MyStrCmp("type\0", str))
	{
		path[1] = '\0'; // null-terminate
		path[0] = '/'; // start for file path
	}
	else // if not the type, get the path
	{
		str = str + 5;
		MyStrCpy(path, str);
	}
	MyStrCpy(msg.bytes, path);
	msg.numbers[0] = STAT;

	MsgSnd(file_sys_pid, &msg);
	MsgRcv(&msg);
	// If the resutl is not ok:
	if (msg.numbers[0] != OK)
	{
		MyStrCpy(msg.bytes,"Error: Can not read file!\n\0");
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
		return; // cant continue
	}

	p = (stat_t *)msg.bytes;

	while(msg.numbers[0] == OK) { // while it can read

		if (S_ISDIR(p->mode))
		{
			MyStrCpy(msg.bytes,"Error: Cant read a dir!\n\0");
			MsgSnd(stdout_pid, &msg);
			MsgRcv(&msg);
			return;
		}else{
			MyStrCpy(msg.bytes, path);
			msg.numbers[0] = OPEN;
			MsgSnd(file_sys_pid, &msg);
			MsgRcv(&msg);

			msg.numbers[0] = READ;
			MsgSnd(file_sys_pid, &msg);
			MsgRcv(&msg);

			while(msg.numbers[0] != END_OF_FILE)
			{
				MsgSnd(stdout_pid, &msg);
				MsgRcv(&msg);

				msg.numbers[0] = READ;
				MsgSnd(file_sys_pid, &msg);
				MsgRcv(&msg);
			}
			//Reached end of file
		}
	}

	msg.numbers[0] = CLOSE;
	MsgSnd(file_sys_pid, &msg);
	MsgRcv(&msg);


	if (msg.numbers[0] != OK)
	{
		MyStrCpy(msg.bytes,"Error: Can not read file!\n\0");
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
		return; // cant continue
	}
}



void ShellHelp(int stdout_pid){
	msg_t msg;
	char print_help[] = "\n====================Walls OS=====================\n";
	MyStrCpy(msg.bytes, print_help);
	MsgSnd(stdout_pid, &msg);
	MsgRcv(&msg);

	//print buffer small, spliting to multiple messages
	MyStrCpy(print_help, "\tHelp\n\twho:\tDisplays our OS team name\n\n\tbye:\tQuits the shell\n");
	MyStrCpy(msg.bytes, print_help);
	MsgSnd(stdout_pid,&msg);
	MsgRcv(&msg);

	//Buffer still small, getting segfult, moving spliting message
	MyStrCpy(print_help, "\tprint <filename>:\tprints specified file\n");
	MyStrCpy(msg.bytes, print_help);
	MsgSnd(stdout_pid,&msg);
	MsgRcv(&msg);

	MyStrCpy(print_help,  "\tdir [path]:\tlists the content of the directory.\n\ttype <filename>:\tPrints the file\n");
	MyStrCpy(msg.bytes, print_help);
	MsgSnd(stdout_pid,&msg);
	MsgRcv(&msg);

	MyStrCpy(print_help,  "\n====================Walls OS=====================\n");
	MyStrCpy(msg.bytes, print_help);
	MsgSnd(stdout_pid,&msg);
	MsgRcv(&msg);

}


void ShellWho(int stdout_pid){
	// The who command :)
	msg_t msg;
	char dr_who[] = "Walls OS! !Windows\n\t\t all right reserved || Fall 2013\n\0";
	MyStrCpy(msg.bytes, dr_who);
	MsgSnd(stdout_pid, &msg);
	MsgRcv(&msg);
}

void ShellPrint(char *str, int stdout_pid, int printdriver_pid, int file_sys_pid) {
	char path[NUM_BYTE];
	stat_t *p;
	msg_t msg;


	if(MyStrCmp("print\0", str))
	{
		path[1] = '\0'; // null-terminate
		path[0] = '/'; // start for file path
	}
	else // print is 5 letters, start at the sixth
	{
		str = str + 6;
		MyStrCpy(path, str);
	}
	MyStrCpy(msg.bytes, path);
	msg.numbers[0] = STAT;

	MsgSnd(file_sys_pid, &msg);
	MsgRcv(&msg);

	if (msg.numbers[0] != OK)
	{
		MyStrCpy(msg.bytes,"Error: Can not read file!\n\0");
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
		return; // cant continue
	}

	p = (stat_t *)msg.bytes;

	while(msg.numbers[0] == OK) {  // while ok

		if (S_ISDIR(p->mode))
		{
			MyStrCpy(msg.bytes,"Error: Cant read a dir!\n\0");
			MsgSnd(stdout_pid, &msg);
			MsgRcv(&msg);
			return;
		}else{
			MyStrCpy(msg.bytes, path);
			msg.numbers[0] = OPEN;
			MsgSnd(file_sys_pid, &msg);
			MsgRcv(&msg);

			msg.numbers[0] = READ;
			MsgSnd(file_sys_pid, &msg);
			MsgRcv(&msg);

			while(msg.numbers[0] != END_OF_FILE)
			{
				MsgSnd(printdriver_pid, &msg);

				msg.numbers[0] = READ;
				MsgSnd(file_sys_pid, &msg);
				MsgRcv(&msg);
			}
			//Reached end of file
		}
	}

	msg.numbers[0] = CLOSE;
	MsgSnd(file_sys_pid, &msg);
	MsgRcv(&msg);


	if (msg.numbers[0] != OK)
	{
		MyStrCpy(msg.bytes,"Error: Can not read file!\n\0");
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
		return; // cant continue
	}
}


void ShellExecutable(char *str,int stdout_pid, int file_sys_pid)
{
	stat_t *p;
	msg_t msg;
	int result, child_pid, exit_code;

	MyStrCpy(msg.bytes, str);
	msg.numbers[0] = STAT;
	MsgSnd(file_sys_pid, &msg);
	MsgRcv(&msg);
	result = msg.numbers[0];
	p = (stat_t *)msg.bytes;

	if(result != OK)
	{
		MyStrCpy(msg.bytes, "Error: Can not read file!\n\0");
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
	}
	else if(result == OK && p->mode != MODE_EXEC)
	{
		MyStrCpy(msg.bytes, "Error: Can not execute");
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
	}
	else
	{
		p = (stat_t *)msg.bytes;
		child_pid = Fork((unsigned int *)p->content,p->size,stdout_pid);
		sprintf(msg.bytes, "child_pid is: %d\n", child_pid);
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);

		exit_code = Wait();
		sprintf(msg.bytes, "child pid is: %d and exit code is: %d\n", child_pid, exit_code);
		MsgSnd(stdout_pid, &msg);
		MsgRcv(&msg);
	}
}
