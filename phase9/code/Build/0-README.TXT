About nap.s and hello.s:

nap.s is coded in GNU-assembler (GAS) which nap for 3 seconds
and exits (with exit code). See code.

hello.s is GNU assembly code that sends a message "Hallo..."
to stdout terminal. In order to run it correctly, hardcoded
stdout PID and memory address must match yours.
"as -a hello.s" shows relative addresses in code.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Example: build "nap.out" and include it into "bin/nap.out."

Copy the involved files in a separate area such as "8/builds/" folder.

1. Generate "nap.o" object by entering the following shell command:

   "as -o nap.o nap.s"

3. Link "nap.o" object file into an ELF executable "nap.out" by:

   "link386 -nostartfiles -userapp -o nap.out nap.o"

4. Extract the binary executable into text by running 
   "bin2hex.pl nap.out"

5. To include "nap.out.hex" into the file system, modify file_sys.c
   with the following, right before the "bin/" directory is defined.

char content_nap[]= {
   #include "nap.out.hex"
};
#define SIZE_NAP_OUT (sizeof(content_nap))

// and change "bin/" to include the above object:

static dir_t bin_dir[]= {
   { 20, MODE_DIR, ".", ~0, (char *)bin_dir },
   { 21, MODE_DIR, "..", ~0, (char *)root_dir },
   { 22, MODE_EXEC, "nap.out", SIZE_NAP_OUT, content_nap },
   { 0, 0, NULL, 0, NULL },
   { END_DIR_INODE, 0, NULL, 0, NULL }
};

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
SOME TECHNICAL NOTES:

Without "-nostartfiles" option in the linking stage, the default C runtime
libraries ("crt1.o") will be linked and and extra code will be added.

By default the linker will link the code at 1MB memory location and the
"-userapp" option links it at 2GB instead (if using FLAMES to load it
to run). These values aren't relevant for our projects since it is not
going to be directly downloaded to the target PC to run by FLAMES (like
MyOS.dli).

The ELF file header and program header are 40 and 88 bytes, so, the first
128 (0x80) bytes of the executable "nap.out" is not the text/code. At
offset 128 is where the actual code starts. To verify this, the ELF program
header has a field containing this value. Use the following shell command
to see it:

   objdump --all-headers --full-contents nap.out

Since the code segment of the ELF file starts after 128 bytes from the
beginning, the Perl program bin2hex.pl skips the first 128 bytes.

If using C code to build "nap.out" (has a main()) and calls "Sleep();"
defined in sys_calls.c, use the "-fPIC" (Position Independent Code) as
one of gcc386 compiler flags in order to make "nap.out" loadable at
any address (not fixed).
