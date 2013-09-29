/*	sample.c - Sample Main program for SPEDE (lab manual page 8) */

#include <spede/stdio.h>
#include <spede/flames.h>

int main (void)
{
	int i;
	int j;

	i = 128;

	for (j=0;j<5;j++)
	{
		printf("%d Hello world %d \nECS", i, 2 * i);
		cons_printf("--> Hello world <--\nCPE/CSC");
		i++;
	}
	return 0;
}
