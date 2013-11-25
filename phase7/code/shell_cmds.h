//shell_cmds.h

#ifndef _SHELL_CMDS_H_
#define _SHELL_CMDS_H

#include "filesys.h"

void DirLine(stat_t *, char *);
void ShellDir(char *str,int,int);
void ShellType(char *str,int,int);

void ShellHelp(int);
void ShellWho(int);
void ShellBye();
void ShellPrint(char *str,int,int);

#endif


