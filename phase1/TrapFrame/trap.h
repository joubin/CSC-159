// trap.h, 159
// defines the trapframe type, tf_t
 
#ifndef _TRAP_H_
#define _TRAP_H_
 
#ifndef ASSEMBLER
 
// This structure corresponds to the state of user registers 
// as saved upon kernel trap/interrupt entry. 
 
typedef unsigned short seg_type_t; // 16-bit segment value 
 
typedef struct
{ 
// Saved segment registers 
	seg_type_t	gs;          // unsigned short, 2 bytes 
	seg_type_t	_notgs;      // filler, 2 bytes 
	seg_type_t	fs; 
	seg_type_t	_notfs; 
	seg_type_t	es; 
	seg_type_t	_notes; 
	seg_type_t	ds; 
	seg_type_t	_notds; 
 
// PUSHA register state frame 
	unsigned int	edi; 
	unsigned int	esi; 
	unsigned int	ebp; 
	unsigned int	esp;  // Push: before PUSHA, Pop: skipped 
	unsigned int	ebx; 
	unsigned int	edx; 
	unsigned int	ecx; 
	unsigned int	eax; 
 
// to indicate the type of interrupt that has happened
	unsigned int	intr_id; 
 
// Processor state frame 
	unsigned int	eip; 
	unsigned int	cs; 
	unsigned int	eflags; 
} tf_t; 
 
#endif ASSEMBLER 

#endif _TRAP_H_
