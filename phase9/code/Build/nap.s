# nap.s
# as -o nap.o nap.s
# link386 -nostartfiles -userapp -o nap.out nap.o
# bin2hex.pl nap.out (to generate nap.out.hex)
# as -a nap.s (address map)

.data               # data section of program

.text               # text/code section of program
.global _start          # declare that _start is our main()

_start:                 # instructions starts here
    movl    $3, %eax    # 3 sec
    int $49     # SLEEP_INTR

    movl    $99, %eax   # exit code
    int $58     # EXIT_INTR
