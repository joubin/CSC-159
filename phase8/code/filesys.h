// Phase 7, Sac State CpE/CSc 159
// filesys.h, paired with filesys.c
// defines for basic file system interface

#ifndef _FILESYS_H_
#define _FILESYS_H_

#define NUM_FD 20                                 // 20 file descriptors
#define FD_VALID(fd) (0 <= (fd) && (fd) < NUM_FD) // range check, 0 <= fd <= 19

#define NELEMENT(a) ( return sizeof(a) / sizeof(a[0]) )

#define S_IFMT  0xF000 // Status Identifier Flag Mask Table for mask & compare
#define S_IFDIR 0x4000 // DIRectory type
#define S_IFREG 0x8000 // REGular file type

#define S_IRWXU 00700 // owner: read, write, execute
#define S_IRWXG 00070 // group: read, write, execute
#define S_IRWXO 00007 // other: read, write, execute
#define S_IROTH 00004 // other: read
#define S_IWOTH 00002 // other: write
#define S_IXOTH 00001 // other: execute

// macros to extract informaton from "mode" field of "stat_t"
// is it a directory or regular file?
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)

// flags for FS_SEEK, "direction/whence"
#define SEEK_SET 0 // start from beginning of file
#define SEEK_CUR 1 // start from where it ended last time "fds[].offset"
#define SEEK_END 2 // start from EOF

#define MODE_EXEC (0777 | S_IFREG)
#define MODE_FILE (0666 | S_IFREG)
#define MODE_READFILE (0444 | S_IFREG)
#define MODE_DIR (0777 | S_IFDIR)

#define END_DIR_INODE ((unsigned int)(~0)) // end marker of directory content

typedef struct
{
   int mode;    // file access mode
   int inode;   // inode on this device
   int dev;     // proc ID of file system
   int nlink;   // number of links to this file
   int size;
   char *content; // file content starts here
} stat_t;

// directory type
typedef struct
{
   int inode;
   int mode;
   char *name;
   int size;
   char *content;
} dir_t;

// file descriptor type
typedef struct
{
   dir_t *item;
   dir_t *owning_dir; // dir where `item' resides
   int owner;         // PID, -1 means not used
   int offset;        // can be negative
} fd_t;

int Stat(char *, stat_t *);
int Open(char *, int, int *);
int Close(int, int);
int Read(int, char *, int, int, int *);
int Seek(int, int, int, int, int *);
int CanAccessFd(int, int);
int AllocFd(int);
void FreeFd(int);

dir_t *FindName(char *);
dir_t *FindNameAux(char *, dir_t *);

void Dir2Stat(dir_t *, stat_t *);

// uncomment this if you use it
// int StrCmp(char *, char *, int);    // compare two strings size of int

void FileSys();                     // File System Manager

#endif
