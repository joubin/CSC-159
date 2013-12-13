# hello.s
# as -o hello.o hello.s
# link386 -nostartfiles -userapp -o hello.out hello.o
# bin2hex.pl hello.out (to generate hello.out.hex)
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#       stdout PID (6) is HARD-CODED (see below)
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# GNU assembler syntax, usage, etc. shell> as --help
# http://www.cs.utah.edu/dept/old/texinfo/as/as_toc.html

.data           # data section in memory is after .text
msg:            # 1st is our local msg_t msg
   .rept 8      # repeat 8 times (8 integers 1st in msg)
   .long 0      # int msg.sender, msg.send_tick, msg.numbers[6]
   .endr        # end repetition
   .ascii "Hallo, mein name ist a.out!\n\0" # msg.bytes (29 chars)
   .rept 72     # repeat 72 times (29+72=101 msg.bytes)
   .ascii "\0"      # padding 72 nulls
   .endr        # end repetition

.text           # text/code section of program
.global _start      # declare that _start is our main()
_start:             # instructions starts here

   popl %eax

# calc my msg addr and save copies to pop later when MsgSnd/MsgRcv
   pushl %esp       # esp is 4KB from beginning
   popl %edx        # pop it into edx to subtract
   subl $0x1000, %edx   # subtract 4KB is .text (14M+)
   movl $msg, %ecx  # addr of msg is 0x80000080+y since
   subl $0x80000080, %ecx # 2G set by link386, skip 128 B header
   addl %edx, %ecx  # add 14M+ is actual (e.g., 0xe00000+y)
   pushl %ecx       # save a copy
   pushl %ecx       # save another copy
   pushl %ecx       # save another copy

# stdout PID is hard-coded, need to check with its Spawn order
   popl %ebx        # actual addr of msg (14M+)
   int  $0x36     # MSGSND_INTR

# Stdout in irq34.c needs to reply this proc (use msg.sender)
   popl %eax        # actual addr of msg (14M+)
   int  $0x37     # MSGRCV_INTR

# exit with msg addr
   popl %eax        # exit code, can print with %x (in hex)
   int  $0x3A     # EXIT_INTR
