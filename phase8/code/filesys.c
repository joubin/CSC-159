// filesys.c, Phase 7, Sac State CpE/CSc 159
//
// a File System that does not use disks, but data structures
// a "memory-based" file system with preloaded objects in "www/" and "bin/"

#include "spede.h"
#include "types.h"
#include "sys_calls.h"
#include "op_codes.h" // operation codes: OK, STAT, etc.
#include "filesys.h" // needed types and data defined/declared here
#include "q_mgmt.h"

char content_msg_txt[]=
{
   'W', 'i', 'l', 'l', ' ', 'I', ' ', 'd', 'r', 'e', 'a', 'm', ',', ' ',
   'D', 'a', 'v', 'e', '.', '.', '.', '?', '\n', '\0'
};
#define SIZE_MSG_TXT ( sizeof( content_msg_txt ) )

char content_README[]=
{
   'Y', 'o', 'u', ' ', 'h', 'a', 'v', 'e', ' ', 'n', 'o', 'w', ' ', 'g', 'o', 't', ' ',
   '1', '5', '9', ' ', 'F', 'i', 'l', 'e', ' ', 'S', 'y', 's', 't', 'e', 'm', ' ',
   'w', 'o', 'r', 'k', 'i', 'n', 'g', '!', '\n', 'B', 'r', 'a', 'v', 'o', '!', '\n', '\0'
};
#define SIZE_README ( sizeof( content_README ) )

// These are plain-text files in the file system. C compiler adds a null
// at the end of a string when initialized (not char-based arrays like above).
// Use C string concatenation to join lines is fine and '\0' is still added at
// the end. Reduce the size by 1 to be accurate (what strlen() would give).

char content_index_html[]=
{
   "<HTML>\n"
   "<HEAD>\n"
   "<TITLE>Sac State CpE/CSc 159 O S Pragmatics</TITLE>\n"
   "</HEAD>\n"
   "<BODY>\n"
   "Life is good, stocks are up, people are wonderful!\n"
   "</BODY></HTML>\n"
};
#define SIZE_INDEX_HTML ( sizeof( content_index_html ) ) // null is added+counted

char content_hello_html[]=
{
   "<HTML><HEAD>\n"
   "<TITLE>Hello, Sac State 159ers!</TITLE>\n"
   "</HEAD>\n"
   "<BODY BgColor=#FFFFFF>"
   "Welcome to the simulated File System of 159 OS!\n"
   "</BODY></HTML>\n"
};
#define SIZE_HELLO_HTML ( sizeof( content_hello_html ) ) // null is added+counted

// We'll later declare "root_dir[]" later. Here is a forward declare.
extern dir_t root_dir[];                   // (Declare in advance.)

dir_t bin_dir[]=
{
   { 20, MODE_DIR, ".", ~0, (char *)bin_dir },   // current dir
   { 21, MODE_DIR, "..", ~0, (char *)root_dir }, // parent dir, forward declared
   { 0, 0, NULL, 0, NULL },                      // no entries in dir
   { END_DIR_INODE, 0, NULL, 0, NULL }           // end of bin_dir[]
};
#define NUM_BIN_DIRENTS NELEMENTS( bin_dir )

dir_t www_dir[]=
{
   { 10, MODE_DIR, ".", ~0, (char *)www_dir },
   { 11, MODE_DIR, "..", ~0, (char *)root_dir },
   { 12, MODE_FILE, "index.html", SIZE_INDEX_HTML, (char *)content_index_html },
   { 13, MODE_FILE, "hello.html", SIZE_HELLO_HTML, (char *)content_hello_html },
   { 0, 0, NULL, 0, NULL },          
   { END_DIR_INODE, 0, NULL, 0, NULL }
};
#define NUM_WWW_DIRENTS NELEMENTS( www_dir )

dir_t root_dir[]=
{
   { 1, MODE_DIR, ".", ~0, (char *)root_dir },
   { 2, MODE_DIR, "bin", sizeof(bin_dir), (char *)bin_dir },
   { 3, MODE_DIR, "www", sizeof(www_dir), (char *)www_dir },
   { 4, MODE_FILE, "msg.txt", SIZE_MSG_TXT, (char *)content_msg_txt },
   { 5, MODE_READFILE, "README", SIZE_README, (char *)content_README },
   { 0, 0, NULL, 0, NULL },
   { END_DIR_INODE, 0, NULL, 0, NULL }
};
#define NUM_ROOT_DIRENTS NELEMENTS( root_dir )


// *********************************************************************
//
fd_t fds[ NUM_FD ];  // one file descriptor (fd) is for every OPEN call
//
// *********************************************************************


// A handy routine to compare two strings, up to a certain length,
// to see if they are the same, ignoring lettering case.
// RETURN: TRUE if they're the same up to 'len', FALSE if not.
// if you haven't had one:
int StrCmpLen( char *s, char *t, int len ) {
   while( len >0 && *s != '\0' && *s == *t ) {
      --len;
      ++s;
      ++t;
   }
   if( len == 0 || ( *s == '\0' && *t == '\0' ) ) return 1;
   return 0;
}

// *********************************************************************
// File System, a process to answer queries and deliver file contents
// via message IPC
// *********************************************************************
void FileSys()
{
   int shell_pid, // the shell that's asking FileSys for service
       result,    // the result to be included to return to shell
       op_code,   // what type of service is shell asking?
       i;

   msg_t msg;

// after all the directory entries have been assigned, then fill in the size
// of this "directory". _dir[1] is ".." which must point to parent dir
   root_dir[0].size = sizeof( root_dir );

   bin_dir[0].size = sizeof( bin_dir );
   bin_dir[1].size = root_dir[0].size;

   www_dir[0].size = sizeof( www_dir );
   www_dir[1].size = root_dir[0].size;

// mark all file descriptors as available, make sure NOT_USED in op_codes.h
   for( i = 0; i < NUM_FD; i++ ) fds[i].owner = NOT_USED;

   while( 1 ) // start serving requests from shells
   {
      MsgRcv( &msg );

      shell_pid = msg.sender;    // shell's pid to return query results
      op_code = msg.numbers[0];  // each op_code is defined in op_codes.h
      
      switch( op_code ) // depending on what's requested
      {
// STAT: Shell wants to "stat" the file (name in msg.bytes),
// the stat of file will be returned by using msg.bytes as well
         case STAT:
            result = Stat( msg.bytes, (stat_t *)msg.bytes );
            break;

// OPEN: Shell wants to open a file (msg.bytes), FD will be returned
// in msg.numbers[2], the ownership (shell_pid) will be registered to that FD
         case OPEN:
            result = Open( msg.bytes, shell_pid, &msg.numbers[2] );
            break;

// READ: Shell wants content of FD (msg.numbers[2]) returns (msg.bytes) and
// # of bytes read ( msg.numbers[1] )
         case READ:
            result = Read( msg.numbers[2], msg.bytes, NUM_BYTE,
                          shell_pid, &msg.numbers[1] );
            break;

// CLOSE: Shell wants to close FD (msg.numbers[2]), we check if the owner
// of that FD is indeed Shell (shell_pid)
         case CLOSE:
            result = Close( msg.numbers[2], shell_pid );
            break;

/****************************** NOT USED ************************************
// SEEK: Shell wants to position R/W-pointer of FD by giving FD
// in msg.numbers[2], offset in msg.numbers[3], whence in msg.numbers[4],
// returns absolute offset (from beginning of file) in msg.numbers[5]
         case SEEK:
            result = Seek( msg.numbers[2], msg.numbers[3], msg.numbers[4],
                           shell_pid, &msg.numbers[5] );
            break;
****************************** NOT USED ************************************/

// unknown op_code received in msg.numbers[0]
         default:
            result = UNKNOWN_OP_CODE;
            cons_printf( "FileSys: Bad op_code %d in message from PID #%d\n",
                         op_code, shell_pid );
      }

// reply msg to shell, result in msg.numbers[0] and other things
      msg.numbers[0] = result;
      MsgSnd( shell_pid, &msg );
   }
}

// get info about a specific obj. if not found, returns an error code.
int Stat( char *name, stat_t *st_p )
{
   dir_t *dir_p = FindName( name );

   if( ! dir_p ) return NOT_FOUND;

   Dir2Stat( dir_p, st_p ); // copy what dir_p points to to where st_p points to

// this should be included to pass the filename (add 1 to length for null)
   MyMemCpy( (char *)( st_p + 1 ), dir_p->name, strlen( dir_p->name ) + 1 );

   return OK;
}

// access a file or directory, returns file descriptor to use for future
// READ calls, or error code if fails
int Open( char *name, int owner, int *fd_p )
{
   int fd;
   dir_t *dir_p;

   fd = AllocFd( owner );

   if( fd == NO_MORE_FD )
   {
      cons_printf( "FileSys: no more available file descriptor!\n" );

      return NO_MORE_FD;
   }

   dir_p = FindName( name );

   if( ! dir_p ) return NOT_FOUND;

   *fd_p = fd;           // fd to return
   fds[fd].item = dir_p; // dir_p is the name

   return OK;
}

int Close( int fd, int owner )
{
   int result;

   result = CanAccessFd( fd, owner ); // check if owner owns FD, or can't close it

   if( result == OK ) FreeFd( fd );

   return result;
}

// Copy bytes from file into user's buffer. Returns actual count of bytes
// transferred. Read from fds[fd].offset (initially given 0) for buff_size
// bytes, and record the offset. may reach EOF though...
int Read( int fd, char *buff, int buff_size, int owner, int *lp_actual )
{
   int result;
   int remaining;
   dir_t *lp_dir;

   *lp_actual=0;

   result = CanAccessFd( fd, owner ); // check if owner owns the fd

   if( result != OK ) return result;

   lp_dir = fds[fd].item;

   if( S_ISDIR(lp_dir->mode ) ) // it's a dir
   {
// if reading a directory, return stat_t structure followed by an obj name.
// a chunk returned per read. `offset' is index into root_dir[] table.
      dir_t *this_dir = lp_dir;
      stat_t *st_p = (stat_t *)buff;
      dir_t *stat_dir;

      if( buff_size < sizeof( *st_p ) + 2) return BUFF_TOO_SMALL;

// use current dir, advance to next dir for next time when called
      do
      {
         stat_dir = ( (dir_t *)this_dir->content );
         stat_dir += fds[fd].offset ;

         if( stat_dir->inode == END_DIR_INODE ) return END_OF_FILE;

         fds[fd].offset++;   // advance

      } while( stat_dir->name == NULL );

// MyBZero() fills buff with 0's, necessary to clean buff
// since Dir2Stat may not completely overwrite whole buff...
      MyBZero( buff, buff_size );
      Dir2Stat( stat_dir, st_p );

// copy obj name after stat_t, add 1 to length for null
      MyMemCpy( (char *)( st_p + 1 ), stat_dir->name, strlen( stat_dir->name ) + 1);

//     *lp_actual = sizeof(*stat_dir) + strlen((char *)(st_p + 1)) + 1;
      *lp_actual = sizeof( stat_t ) + strlen( stat_dir->name ) + 1;

   }
   else // a file, not dir
   {
// compute max # of bytes can transfer then MyMemCpy()
      remaining = lp_dir->size - fds[fd].offset;

      if( remaining == 0 ) return END_OF_FILE;

      MyBZero( buff, buff_size );  // null termination for any part of file read

      result = _MIN( buff_size - 1, remaining ); // -1 saving at least 1 last NULL

      MyMemCpy( buff, &lp_dir->content[ fds[ fd ].offset ], result );

      fds[ fd ].offset += result;  // advance our "current" ptr

      *lp_actual = result;
   }

   return OK;
}

/******************************** NOT USED ***********************************
// "whence" tells which base to use, advance "offset" number of bytes from
// there, return base+offset in new_pos (also record it in fds[].offset
int Seek( int fd, int offset, int whence, int owner, int *new_pos )
{
   int result;

   result = CanAccessFd( fd, owner );

   if( result != OK ) return result;

   switch( whence )
   {
      case SEEK_SET: // start from beginning of file
         *new_pos = offset;
         break;

      case SEEK_CUR: // start from offset set by last seek (read can do it too)
         *new_pos = fds[fd].offset + offset;
         break;

      case SEEK_END: // start from end of file
         *new_pos = fds[fd].owning_dir->size + offset;
         break;

      default :
         return BAD_PARAM;
   }
// make sure offset in range. if before, then return ERROR. if beyond file
// size, keep it; owner may re-seek later
   if( *new_pos < 0 ) result = BAD_PARAM;
   else fds[fd].offset = *new_pos;

   return result;
}
*****************************************************************************/

// check ownership of fd and the fd is valid within range
int CanAccessFd( int fd, int owner )
{
   if( FD_VALID( fd ) && fds[fd].owner == owner) return OK;

   return BAD_PARAM;
}

// search our (fixed size) table of file descriptors. returns fds[] index if
// an unused entry is found, else -1 if all in use. if avail, then all fields
// are initialized.
int AllocFd( int owner )
{
   int i;

   for( i = 0; i < NUM_FD; i++ )
   {
      if( NOT_USED == fds[i].owner )
      {
         fds[i].owner = owner;
         fds[i].item = NULL;
         fds[i].offset = 0;

         return i;
      }
   }

   return NO_MORE_FD;   // no free file descriptors
}

// mark a file descriptor as now free
void FreeFd( int fd )
{
   fds[fd].owner = NOT_USED;
}

dir_t *FindName( char *name )
{
   dir_t *starting;

// assume every path relative to root directory. Eventually, the user
// context will contain a "current working directory" and we can possibly
// start our search there
   if( name[0] == '/' )
   {
      starting = root_dir;

      while( name[0] == '/' ) name++;

      if( name[0] == 0 ) return root_dir; // client asked for "/"
   }
   else
   {
// path is relative, so start off at CWD for this process
// but we don't have env var CWD, so just use root as well
      starting = root_dir; // should be what env var CWD is
   }

   if( name[0] == 0 ) return NULL;

   return FindNameAux( name, starting );
}

// go searching through a single dir for a name match. use StrCmpLen() for
// case-insensitive compare. use '/' to separate directory components
// if more after '/' and we matched a dir, recurse down there
// RETURN: ptr to dir entry if found, else NULL
// once any dir matched, don't return name which dir was matched
dir_t *FindNameAux( char *name, dir_t *this_dir )
{
   dir_t *dir_p = this_dir;
   int len = strlen( name );
   char *p;

// if name ends in '/', chances are we need to decend into the dir
   if( ( p = strchr( name, '/' ) ) != NULL) len = p - name; 

   for( ; dir_p->name; dir_p++ )
   {
      if( 1 == StrCmpLen( name, dir_p->name, len ) )
      {
         if( p && p[1] != 0 )
         {
// user is trying for a sub-dir. if there are more components, make sure this
// is a dir. if name ends with "/" we don't check. thus "hello.html/" is legal
             while( *p == '/' )
             {
                p++;
                if( '\0' == *p ) return dir_p; // "/README/////" is OK
             }
             name = p;

             return FindNameAux( name, (dir_t *)dir_p->content );
         }
         return dir_p;
      }
   }

   return NULL;   // no match found
}

// copy what dir_p points to (dir_t) to what st_p points to (stat_t)
void Dir2Stat( dir_t *dir_p, stat_t *st_p )
{
   st_p->dev = GetPid();            // FileSys() manages this i-node

   st_p->inode = dir_p->inode;
   st_p->mode = dir_p->mode;
   st_p->nlink = ( S_ISDIR( st_p->mode ) ) + 1;
   st_p->size = dir_p->size;
   st_p->content = dir_p->content;
}
