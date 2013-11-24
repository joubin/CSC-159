// shell_cmds.c, Phase 7, Sac State CpE/CSc 159
// shell subroutines to perform "dir/type/print" commands
// complete the pseudo code below, prototype them into "shell_cmds.h"

#include "spede.h"
#include "types.h"
#include "sys_calls.h"
#include "op_codes.h" // operation codes: OK, STAT, etc.
#include "filesys.h"
#include "externs.h"
#include "q_mgmt.h"   // MyStrCmp(), etc.

// buld a string: a detail line of attributes of the given file/directory
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
void ShellDir(char *str, int stdout, int filesys) {
   char path[NUM_BYTE], line[NUM_BYTE];
   stat_t *p;
   msg_t msg;

// if str is "dir\0" assume home (root) dir "/"
// else, assume user specified a path after first 4 letters "dir "
   if(MyStrCmp(str, "dir\0") == 1) {
      path[0] = '/';
      path[1] = '\0'; // null-terminate the path[]
   } else { // skip 1st 4 letters "dir " and get the path
      str += 4;
      MyStrCpy(path, str); // make sure str is null-terminated from Shell()
   }

//*************************************************************************
// write code:
// prep msg: path[] in msg.bytes:
//   MyMemCpy(msg.bytes, path, sizeof(path));
// put op code STAT in msg.nums[0]
// send msg to FileSys, receive from FileSys, result is msg.nums[0]
//*************************************************************************

   if(msg.nums[0] != OK) {

//*************************************************************************
// write code:
// send an error msg to Stdout
// receive synchro msg from Stdout
//*************************************************************************
      return;        // we can't continue
   }

// By op_code STAT, file sys returns in msg.bytes a "stat_t" type,
// take a peek, if user directed us to a file, then display info for only
// that file; otherwise, it's a directory, needs to display each entry in
// the dir content as if we are "listing" it
   p = (stat_t *)msg.bytes;    // p, status type pointer

   if( ! S_ISDIR(p->mode) ) // if not dir, it's a file, detail-list it
   {
      DirLine(p, line); // line is to be built and returned by subroutine

//*************************************************************************
// write code:
// prep and send msg to Stdout with line[] copied to msg.bytes
// receive synchro msg from Stdout
//*************************************************************************
      return;     // we can't continue
   }
// it's a dir, we list its content (entries), one by one in a loop
// we 1st request FileSys to open the dir, and then issue a read on
// each entry in it in the loop

//*************************************************************************
// write code:
// prep msg: put path[] in msg.bytes, put op code OPEN in msg.nums[0]
// send msg to FileSys, receive msg from file sys (msg.nums[0] is result
// code and msg.nums[2] is the fd assigned)
//*************************************************************************

   while(msg.nums[0] == OK) {
//*************************************************************************
// write code:
// prep msg: put op code READ in msg.nums[0], send to FileSys,
// receive from FileSys, what's read is in msg.bytes
// if result not OK, break loop
//*************************************************************************

      p = (stat_t *)msg.bytes;
      DirLine(p, line); // convert msg.bytes into a detail line

//*************************************************************************
// write code:
// prep and send msg to Stdout to show the converted line[]
// receive synchro msg from Stdout
//*************************************************************************
   }

//*************************************************************************
// write code:
// request FileSys to close
// if result NOT OK, display an error msg...
//*************************************************************************
}

void ShellType(char *str, int stdout, int filesys) {
   char path[NUM_BYTE];
   stat_t *p;
   msg_t msg;

//*************************************************************************
// write code:
// if str is "type\0" display "Usage: type <filename>\n\0"
//    return;                  // can't continue
//
// skip 1st 5 characters in str ("type ") to get the rest (str)
// copy str to msg.bytes
// ask FileSys to STAT this
// if result NOT OK, display an error msg...
//    return;         // we can't continue
//
// By op_code STAT, file sys returns in msg.bytes a "stat_t" type,
// take a peek, if user directed us to a file, then display info for only
// that file; otherwise, it's a directory, display an error msg to user.
//
// p = (stat_t *)msg.bytes;    // p, status type pointer
// if( S_ISDIR(p->mode) ) {    // if dir, can't do it
//    display an error msg...
//    return;                  // we can't continue
//*************************************************************************

//*************************************************************************
// write code:
// request FileSys to OPEN (prep msg, send FileSys)
// then start READ loop (until NOT OK)
   while(msg.nums[0] == OK) {          // so long OK
//    request FileSys to READ (prep msg, send FileSys)
//    receive back in msg, text in msg.bytes
//
//    if(msg.nums[0] != OK) break;
//
//    send it to Stdout to show it to terminal
//    receive synchro msg back
//*************************************************************************

//*************************************************************************
// write code:
// request FileSys to close
// if result NOT OK, display an error msg...
//*************************************************************************
}

//*************************************************************************
// write code:
// Other shell commands...
//*************************************************************************
