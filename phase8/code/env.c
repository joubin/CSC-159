// env.c, dump environment vars inherited from parent

#include <stdio.h>

void main(int argc, char *argv[]) {
   int i;

   printf("argc: %d\n", argc);

   for(i=0;i<100;i++)
      printf("argv[%i]: %s\n", i, argv[i]);
}