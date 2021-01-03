/**
 * Onn Rengingad 304845951
 *
 * Bar Ben Ari 204034284
 *
 * Barak Levy 311431894
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#define ADD 0
#define SUB 1
#define LSF 2
#define RSF 3
#define AND 4
#define OR  5
#define XOR 6
#define LHI 7
#define LD 8
#define ST 9
#define COPY 10
#define POLL 11
#define JLT 16
#define JLE 17
#define JEQ 18
#define JNE 19
#define JIN 20
#define HLT 24

#define MEM_SIZE_BITS	(16)
#define MEM_SIZE	(1 << MEM_SIZE_BITS)
#define MEM_MASK	(MEM_SIZE - 1)
unsigned int mem[MEM_SIZE];

int pc = 0;

static void asm_cmd(int opcode, int dst, int src0, int src1, int immediate)
{
	int inst;

	inst = ((opcode & 0x1f) << 25) | ((dst & 7) << 22) | ((src0 & 7) << 19) | ((src1 & 7) << 16) | (immediate & 0xffff);
	mem[pc++] = inst;
}

static void assemble_program(char *program_name)
{
	FILE *fp;
	int addr, last_addr;

	for (addr = 0; addr < MEM_SIZE; addr++)
		mem[addr] = 0;

	pc = 0;

	//Includes CPU Hazards validation tests: Data, Structural, Control(every branch operation is this hazard's validation)....

	//Setting registers for COPY operation
	asm_cmd(ADD, 4, 1, 0, 30);	 // 0: R4 = 30 -COPY source address
	asm_cmd(ADD, 5, 1, 0, 1000); // 1: R5 = 1000 -COPY destination address
	asm_cmd(ADD, 6, 1, 0, 1000); // 2: R6 = 1000 -COPY operation number of cells taken

	//Initiate copy regime in a different thread -  (ALSO A VALIDATATION FOR Data Hazard at register 6 and register 5)
	asm_cmd(COPY, 5, 4, 6, 0);   // 3: COPY - DMA copy operation is initiated:
								 // It will copy 1000 cells from address 
	
	
	//Begin an Accumulative-negative-sum computation in the main thread
		//Variables initiation
	asm_cmd(ADD, 2, 1, 0, 50);	 // 4: R2 = 50
	asm_cmd(ADD, 3, 1, 0, 200);  // 5: R3 = 200
	asm_cmd(ADD, 4, 0, 0, 10000);// 6: R4 = 0 (10000 in immediate isn't necessary)
	
	//Test Structural Hazard in Harvard SP implementation - Store value followed by a load value into\from SRAMd (lines 8 & 9)
	asm_cmd(ADD, 6, 0, 1, 151);	 //7: R6 = 151
	asm_cmd(ST, 0, 6, 1, 49);	 //8: Mem[49] = R6  (The expected image in SRAMd is that starting at address 49 there will be (in HEX) 97 96 95 94...)

		//Load values and compute
	asm_cmd(LD, 5, 0, 2, 0);     // 9: R5 = Mem[R2]
	asm_cmd(SUB, 4, 4, 5, 0);	 // 10: R4 -= R5
	asm_cmd(ADD, 2, 2, 1, 1);	 // 11: R2++
	asm_cmd(JLT, 0, 2, 3, 9);	 // 12: if R2<R3 jump to line 9


	//Polling the DMA copy operation after the main Accumulative-negative-sum computation operation had finished
	asm_cmd(POLL, 7, 0, 0, 0);   // 13 POLL - R[7] recieves the number of remaining cells to be copied by the DMA
	asm_cmd(JNE, 0, 7, 0, 13);	 // 14: if R[7]!=0 return to line 13 until COPY operation is done

	//Following the termination of the DMA and Sum calculation operations, we compare the copied block of addresses
		//Setting registers for comparison operation
	asm_cmd(ADD, 4, 1, 0, 30);	 // 15: R4 = 30 -COPY source address
	asm_cmd(ADD, 5, 1, 0, 1000); // 16: R5 = 1000 -COPY destination address
	asm_cmd(ADD, 6, 1, 0, 1000); // 17: R6 = 1000 -COPY operation total number of cells 
	
	asm_cmd(LD, 2, 0, 4, 0);	 // 18: R2 = Mem[R4]
	asm_cmd(LD, 3, 0, 5, 0);	 // 19: R3 = Mem[R5]
	asm_cmd(JNE, 0, 2, 3, 27);	 // 20: if R2!=R3 (meanining Mem[R4]!=Mem[R5]) than DMA copy operation failed! jumping to pc=27
	asm_cmd(ADD, 4, 4, 1, 1);	 // 21: R4++ (Advancing source address)
	asm_cmd(ADD, 5, 5, 1, 1);	 // 22: R5++ (Advancing destination address)
	asm_cmd(SUB, 6, 6, 1, 1);	 // 23: R6-- (Decresing the number of cells remanining to validate)
	asm_cmd(JNE, 0, 6, 0, 18);	 // 24: if R6!=0 than there are still copied cell that need to be validated. jumping to pc=18 to proceed validating 

	asm_cmd(ADD, 2, 0, 1, 1);	 // 25: R2=1 -> DMA Copy & Fib operation were successful
	asm_cmd(JIN, 0, 1, 0, 28);	 // 26: Jumping to HALT (Indirect jump to the end)
	asm_cmd(ADD, 2, 0, 0, 0);	 // 27: R2=0 -> DMA Copy Failed 
	asm_cmd(HLT, 0, 0, 0, 0);	 // 28: HALT....
	

	last_addr = 170;

	//Additional data to be copied
	for (addr = 50; addr < 150; addr++)
		mem[addr] = 200 - addr;

	fp = fopen(program_name, "w");
	if (fp == NULL) {
		printf("couldn't open file %s\n", program_name);
		exit(1);
	}
	addr = 0;
	while (addr < last_addr) {
		fprintf(fp, "%08x\n", mem[addr]);
		addr++;
	}
	fclose(fp);
}


int main(int argc, char *argv[])
{
	
	if (argc != 2){
		printf("usage: asm program_name\n");
		return -1;
	}else{
		assemble_program(argv[1]);
		printf("SP assembler generated machine code and saved it as %s\n", argv[1]);
		return 0;
	}
	
}
