/**
 * Onn Rengingad 304845951
 *
 * Bar Ben Ari 204034284
 *
 * Barak Levy 311431894
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
#define JLT 16
#define JLE 17
#define JEQ 18
#define JNE 19
#define JIN 20
#define HLT 24
#define MAX_MEM_LINES 65536

void freeAll(long int* mem_out, long int* data, long int* regs);

/** void add(long int *data, long int *regs)
 *
 * Addition: R[dst] = R[src0] + R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
int add(long int *data, long int *regs) {
	//Computation
	if (data[3] > 1) regs[data[3]] = regs[data[4]] + regs[data[5]];
	else printf("Error: wrong use of add function.\n");

	//OVERFLOW CHECK
	if (((regs[data[4]] < 0) && (regs[data[5]] < 0) && (regs[data[3]] > 0)) \
	|| ((regs[data[4]] > 0) && (regs[data[5]] > 0) && (regs[data[3]] < 0))) {
		printf("OVERFLOW in ADD operation - exiting");
		return 1;
	}
	return 0;//No Overflow
}


/** void sub(long int *data, long int *regs)
 *
 * Subtraction: R[dst] = R[src0] - R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
int sub(long int *data, long int *regs) {
	//Computation
	if (data[3] > 1) regs[data[3]] = regs[data[4]] - regs[data[5]];
	else printf("Error: wrong use of sub function.\n");

	//OVERFLOW CHECK
	if (((regs[data[4]] > 0) && (regs[data[5]] < 0) && (regs[data[3]] < 0)) \
		|| ((regs[data[4]] < 0) && (regs[data[5]] > 0) && (regs[data[3]] > 0))) {
		printf("OVERFLOW in SUB operation - exiting");
		return 1;
	}
	return 0;//No Overflow
}

/** void lsf(long int *data, long int *regs)
 *
 * Left shift: R[dst] = R[src0] << R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
int lsf(long int *data, long int *regs) {
	long int msb = 0, lsb = 0;
	int i = 0;

	//OVERFLOW CHECK
	msb = (regs[data[4]] >> 31) & 1;
	for (i = 0; i <= 30; i++) {
		lsb = (regs[data[4]] >> (30 - i)) & 1;
		if (msb != lsb)
			break;
	}
	if (i < regs[data[5]]) {
		printf("OVERFLOW in LSF operation - exiting");
		return 1;
	}

	//Computation
	if (data[3] > 1) regs[data[3]] = regs[data[4]] << regs[data[5]];
	else printf("Error: wrong use of lsf function.\n");
	return 0; //No Overflow
}

/** void rsf(long int *data, long int *regs)
 *
 * Signed right shift: R[dst] = R[src0] >> R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void rsf(long int *data, long int *regs) {
	if (data[3] > 1) regs[data[3]] = regs[data[4]] >> regs[data[5]];
	else printf("Error: wrong use of rsf function.\n");
}

/** void and(long int *data, long int *regs)
 *
 * Bitwise logical and: R[dst] = R[src0] & R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void and(long int *data, long int *regs) {
	if (data[3] > 1) regs[data[3]] = regs[data[4]] & regs[data[5]];
	else printf("Error: wrong use of and function.\n");
}

/** void or(long int *data, long int *regs)
 *
 * Bitwise logical or: R[dst] = R[src0] | R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate..
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void or(long int *data, long int *regs) {
	if (data[3] > 1) regs[data[3]] = regs[data[4]] | regs[data[5]];
	else printf("Error: wrong use of or function.\n");
}

/** void xor(long int *data, long int *regs)
 *
 * Bitwise logical xor: R[dst] = R[src0] ^ R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void xor(long int *data, long int *regs) {
	if (data[3] > 1) regs[data[3]] = regs[data[4]] ^ regs[data[5]];
	else printf("Error: wrong use of xor function.\n");
}

/** lhi(long int *data, long int *regs)
 *
 * Load the high bits [31:16] of the destination register with immediate {15:0].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void lhi(long int *data, long int *regs) {
	if (data[3] > 1) regs[data[3]] = (regs[data[3]] & 0x0000ffff)|(data[6] << 16);
	else printf("Error: wrong use of lhi function.\n");
}

/** ld(long int *data, long int *regs, long int *mem_out)
 *
 * Loads memory contents at address specified by R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 * @param long int *mem_out - A pointer to the SRAM data.
 *
 * @return - void.
 */
void ld(long int *data, long int *regs, long int *mem_out) {
	if (data[3] > 1) regs[data[3]] = mem_out[regs[data[5]]];
	else printf("Error: wrong use of ld function.\n");
}

/** st(long int *data, long int *regs, long int *mem_out)
 *
 * Writes R[src0] to memory at address R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 * @param long int *mem_out - A pointer to the SRAM data.
 *
 * @return - void.
 */
void st(long int *data, long int *regs, long int *mem_out) {
	mem_out[regs[data[5]]] = regs[data[4]];
}

/** jlt(long int *data, long int *regs)
 *
 * Jump to pc = immediate if R[src0] < R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void jlt(long int *data, long int *regs) {
	if(regs[data[4]] < regs[data[5]]){
		regs[7] = data[0];
		data[0] = (data[6] & 65535)-1;
	}
}

/** jle(long int *data, long int *regs)
 *
 * Jump to pc = immediate if R[src0] <= R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void jle(long int *data, long int *regs) {
	if(regs[data[4]] <= regs[data[5]]){
		regs[7] = data[0];
		data[0] = (data[6] & 65535)-1;
	}
}

/** jeq(long int *data, long int *regs)
 *
 * Jump to pc = immediate if R[src0] == R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void jeq(long int *data, long int *regs) {
	if(regs[data[4]] == regs[data[5]]){
		regs[7] = data[0];
		data[0] = (data[6] & 65535)-1;
	}
}

/** jne(long int *data, long int *regs)
 *
 * Jump to pc = immediate if R[src0] != R[src1].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void jne(long int *data, long int *regs) {
	if(regs[data[4]] != regs[data[5]]){
		regs[7] = data[0];
		data[0] = (data[6] & 65535)-1;
	}
}

/** jin(long int *data, long int *regs)
 *
 * Jump to  pc = R[src0].
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void jin(long int *data, long int *regs) {
	regs[7] = data[0];
	data[0] = regs[data[4]] - 1;
}

/** memInCopy(FILE *memin, long int *mem_out, FILE *trace, char *progName)
 *
 * Copy the memin file to the mem_out array.
 *
 * @param File *memin - A pointer to the input file.
 * @param long int *mem_out - A pointer to the SRAM data.
 * @param File int *trace - A pointer to the trace file.
 * @param char *progName - the program name.
 *
 * @return - void.
 */
void memInCopy(FILE *memin, long int *mem_out, FILE *trace, char *progName) {
	int line = 0;
	while (fscanf(memin, "%08x",(unsigned int*) &(mem_out[line])) != EOF) line++;
	fprintf(trace, "program %s loaded, %d lines\n\n", progName, line);
	if (fclose(memin) == EOF){
		printf("Error: failed closing memin file.\n");
	}
}

/** transWord(long int *data, long int *mem_out, long int *regs)
 *
 * Splits the instruction: data[0] = pc, data[1] = instruction, data[2] = opcode, data[3] = dst, data[4] = src0, data[5] = src1, data[6] = immediate.
 * Save in regs[1] the immediate.
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *mem_out - A pointer to the SRAM data.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void transWord(long int *data, long int *mem_out, long int *regs) {
	data[1] = mem_out[data[0]];
	data[2] = data[1] & 0x3E000000;
	data[2] = data[2] >> 0x19;
	data[3] = data[1] & 0x01c00000;
	data[3] = data[3] >> 0x16;
	data[4] = data[1] & 0x00380000;
	data[4] = data[4] >> 0x13;
	data[5] = data[1] & 0x00070000;
	data[5] = data[5] >> 0x10;
	regs[1] = data[1] & 0x0000ffff;
	regs[1] = (regs[1] << 16) >> 16;
	data[6] = regs[1];
}

/** opcodeStr(long int *data)
 *
 * Create a name(string) for the current instruction (for the trace prints).
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 *
 * @return char* opcode - instruction name (string).
 */
char* opcodeStr(long int *data) {
	switch (data[2]) {
	case ADD:
		return "ADD";
	case SUB:
		return "SUB";
	case LSF:
		return "LSF";
	case RSF:
		return "RSF";
	case AND:
		return "AND";
	case OR:
		return "OR";
	case XOR:
		return "XOR";
	case LHI:
		return "LHI";
	case LD:
		return "LD";
	case ST:
		return "ST";
	case JLT:
		return "JLT";
	case JLE:
		return "JLE";
	case JEQ:
		return "JEQ";
	case JNE:
		return "JNE";
	case JIN:
		return "JIN";
	case HLT:
		return "HLT";
	default:
		printf("Error: not an opcode");
		return NULL;
	}
}

/** updateTrace(long int *data, long int *mem_out, long int *regs, FILE *trace, int cnt_inst, char* opcode)
 *
 * Print the into the trace file the current instruction trace.
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *mem_out - A pointer to the SRAM data.
 * @param long int *regs - A pointer to the registers memory.
 * @param File int *trace - A pointer to the trace file.
 * @param int cnt_inst - instruction counter.
 * @param char *opcode - opcode string.
 *
 * @return - void.
 */
void updateTrace(long int *data, long int *mem_out, long int *regs, FILE *trace, int cnt_inst, char *opcode) {
	fprintf(trace, "--- instruction %i (%04x) @ PC %ld (%04lx) -----------------------------------------------------------\n", cnt_inst, cnt_inst, data[0], data[0]);
	fprintf(trace, "pc = %04ld, inst = %08lx, opcode = %ld (%s), ", data[0], data[1], data[2], opcode);
	fprintf(trace, "dst = %ld, src0 = %ld, src1 = %ld, immediate = %08lx\n", data[3], data[4], data[5], data[6]);
	fprintf(trace, "r[0] = %08lx r[1] = %08lx r[2] = %08lx r[3] = %08lx \nr[4] = %08lx r[5] = %08lx r[6] = %08lx r[7] = %08lx \n\n",regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7]);

	if(data[2] < 8) fprintf(trace, ">>>> EXEC: R[%ld] = %ld %s %ld <<<<\n\n", data[3], regs[data[4]], opcode, regs[data[5]]);
	else if (data[2] == 8) fprintf(trace, ">>>> EXEC: R[%ld] = MEM[%ld] = %08lx <<<<\n\n", data[3], regs[data[5]], mem_out[regs[data[5]]]);
	else if (data[2] == 9) fprintf(trace, ">>>> EXEC: MEM[%ld] = R[%ld] = %08lx <<<<\n\n", regs[data[5]], data[4], regs[data[4]]);
	else if (data[2] == 24) fprintf(trace, ">>>> EXEC: HALT at PC %04lx<<<<\n", data[0]);
}

/** execute(long int *data, long int *mem_out, long int *regs)
 *
 * Execute the current instruction of the program.
 *
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *mem_out - A pointer to the SRAM data.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void execute(long int *data, long int *mem_out, long int *regs, FILE *trace, FILE *memout) {
	int OF = 0;
	switch (data[2]) {
	case ADD:
		OF = add(data, regs);
		break;
	case SUB:
		OF = sub(data, regs);
		break;
	case LSF:
		OF = lsf(data, regs);
		break;
	case RSF:
		rsf(data, regs);
		break;
	case AND:
		and(data, regs);
		break;
	case OR:
		or(data, regs);
		break;
	case XOR:
		xor(data, regs);
		break;
	case LHI:
		lhi(data, regs);
		break;
	case LD:
		ld(data, regs, mem_out);
		break;
	case ST:
		st(data, regs, mem_out);
		break;
	case JLT:
		jlt(data, regs);
		break;
	case JLE:
		jle(data, regs);
		break;
	case JEQ:
		jeq(data, regs);
		break;
	case JNE:
		jne(data, regs);
		break;
	case JIN:
		jin(data, regs);
		break;
	}
	if (OF == 1) {
		freeAll(mem_out, data, regs);
		if (fclose(memout) == EOF) printf("Error: failed close memout file.\n");
		if (fclose(trace) == EOF) printf("Error: failed close trace file.\n");
		exit(1);
	}
	data[0] = (data[0] + 1) % MAX_MEM_LINES;
}

/** finishFiles(FILE *memout, FILE *trace, long int *mem_out, long int *data, int cnt_inst)
 *
 * Complete printing data to the trace and sram_out files.
 *
 * @param File *memout - A pointer to the sram_out file.
 * @param File int *trace - A pointer to the trace file.
 * @param long int *mem_out - A pointer to the SRAM data.
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param int cnt_inst - instruction counter.
 *
 * @return - void.
 */
void finishFiles(FILE *memout, FILE *trace, long int *mem_out, long int *data, int cnt_inst) {
	int i;
	fprintf(trace, "sim finished at pc %ld, %d instructions", data[0], cnt_inst);
	if (fclose(trace) == EOF) printf("Error: failed close trace file.\n");
	for (i = 0; i < MAX_MEM_LINES; i++) {
		if (fprintf(memout, "%08lx\n", mem_out[i]) < 0) printf("Error: failed writing to file memout.\n");
	}
	if (fclose(memout) == EOF) printf("Error: failed close memout file.\n");
}

/** freeAll(long int *mem_out, long int *data, long int *regs)
 *
 * Free all dynamic memory.
 *
 * @param long int *mem_out - A pointer to the SRAM data.
 * @param long int *data - A pointer to the program data, contain: pc, instruction, opcode, dst, src0, src1, immediate.
 * @param long int *regs - A pointer to the registers memory.
 *
 * @return - void.
 */
void freeAll(long int *mem_out, long int *data, long int *regs) {
	free(mem_out);
	free(regs);
	free(data);
}

/** int main(int argc, char *argv[])
 *
 * Allocate memory for SRAM, registers and program data.
 * Opening input file for read and trace + sram_out for write.
 * Execute the machine code.
 *
 * @argv[1] - input file.
 *
 * @return - return 0 if succeed, else return 1.
 */
int main(int argc, char *argv[]) {
	FILE *memin, *memout, *trace;
	long int* data, * mem_out, * regs;
	int cnt_inst = 0;
	char *opStr;
	
	if (argc != 2) {
		printf("Error: wrong number of arguments.\n");
		exit(1);
	}
	if ((memin = fopen(argv[1], "r")) == NULL) {
		printf("Error: failed open memin file.\n");
		exit(1);
	}
	if ((memout = fopen("sram_out.txt", "w")) == NULL) {
		if(fclose(memin) == EOF) printf("Error: failed close memin file.\n");
		printf("Error: failed open sram_out file.\n");
		exit(1);
	}
	if ((trace = fopen("trace.txt", "w")) == NULL) {
		printf("Error: failed open trace file.\n");
		if(fclose(memin) == EOF) printf("Error: failed close memin file.\n");
		if(fclose(memout) == EOF) printf("Error: failed close memout file.\n");
		exit(1);
	}
	if ((data = (long int*) calloc(7, sizeof(long int))) == NULL) {
		if (fclose(memin) == EOF) printf("Error: failed close memin file.\n");
		if (fclose(memout) == EOF) printf("Error: failed close memout file.\n");
		if (fclose(trace) == EOF) printf("Error: failed close trace file.\n");
		printf("Error: failed allocating data.\n");
		exit(1);
	}
	if ((mem_out = (long int*) calloc(MAX_MEM_LINES, sizeof(long int))) == NULL) {
		if (fclose(memin) == EOF) printf("Error: failed close memin file.\n");
		if (fclose(memout) == EOF) printf("Error: failed close memout file.\n");
		if (fclose(trace) == EOF) printf("Error: failed close trace file.\n");
		free(data);
		printf("Error: failed allocating mem_out.\n");
		exit(1);
	}
	if ((regs = (long int*) calloc(8, sizeof(long int))) == NULL) {
		if (fclose(memin) == EOF) printf("Error: failed close memin file.\n");
		if (fclose(memout) == EOF) printf("Error: failed close memout file.\n");
		if (fclose(trace) == EOF) printf("Error: failed close trace file.\n");
		free(data);
		free(mem_out);
		printf("Error: failed allocating regs.\n");
		exit(1);
	}
	memInCopy(memin, mem_out, trace, argv[1]);

	while (1) {
		transWord(data, mem_out, regs);
		opStr = opcodeStr(data);
		updateTrace(data, mem_out, regs, trace, cnt_inst, opStr);
		cnt_inst++;
		if (data[2] == HLT) break;
		else execute(data, mem_out, regs, trace, memout);
		if (data[2] > 15) fprintf(trace, ">>>> EXEC: %s %ld, %ld, %ld <<<<\n\n", opStr, regs[data[4]], regs[data[5]], data[0]);
	}
	finishFiles(memout, trace, mem_out, data, cnt_inst);
	freeAll(mem_out, data, regs);

	return 0;
}
