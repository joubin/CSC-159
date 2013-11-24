// op_codes.h, 159 7
//
// operation codes for OS services, e.g., FileSys
// here are codes covering more than what we need
//
// op code is put into msg.nums[0] to send
// recipient checks, so to know what to do

#ifndef _OP_CODES_H_
#define _OP_CODES_H_

// -------------------- General Code ----------------------
#define OK                0
#define NOT_OK           -1
#define NOT_USED         -1

// -------------------- MODE Related ----------------------
#define ECHO_OFF         77
#define ECHO_ON          88

// ------------------ FileSys Result ----------------------
#define UNKNOWN_OP_CODE  -99
#define NOT_FOUND        -11
#define NO_MORE_FD       -12 // all file descriptors in use
#define END_OF_FILE      -13 // no more content
#define BAD_PARAM        -14 // some parameter was invalid
#define BUFF_TOO_SMALL   -15 // your buffer too small

// ------------------ FileSys Service ---------------------
#define RDONLY 0
#define WRONLY 1
#define RDWR   2

#define STAT 20
// message passing:
//    supply filename string (NUL terminated) in message
//    return file status structure (overwrites the filename string)

#define OPEN 21
// message passing:
//    supply filename (NUL terminated)
//    supply operation flag such as RDONLY
//    return file descriptor if successful, -1 if failure

#define READ 22
// message passing:
//    supply file descriptor
//    supply buffer in a msg
//    return number of bytes read
//    return content read in a msg

#define CLOSE 23
// message passing:
//    supply file descriptor

#define SEEK 24
// message passing:
//    supply file descriptor
//    supply offset (a +/- value)
//    supply whence (base: head, tail, or the last position)
//    return absolute offset (head+offset)

#endif