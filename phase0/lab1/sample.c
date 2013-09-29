/*	sample.c - Sample Main program for SPEDE (lab manual page 8) */

#include <spede/stdio.h>
#include <spede/flames.h>

int main (void)
{
	int i;

	i = 128;
	printf("%d Hello world %d \nECS", i, 2 * i);
	cons_printf("--> Hello world <--\nCPE/CSC");
	return 0;
}
