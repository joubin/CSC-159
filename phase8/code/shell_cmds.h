//shell_cmds.h

#ifndef _SHELL_CMDS_H_
#define _SHELL_CMDS_H

#include "filesys.h"

void DirLine(stat_t *, char *);
void ShellDir(char *,int,int);
void ShellType(char *,int,int);

void ShellHelp(int);
void ShellWho(int);
void ShellPrint(char *,int,int,int);

#endif
