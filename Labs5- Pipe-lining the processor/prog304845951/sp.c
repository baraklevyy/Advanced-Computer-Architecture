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
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "llsim.h"

#define sp_printf(a...)						\
	do {							\
		llsim_printf("sp: clock %d: ", llsim->clock);	\
		llsim_printf(a);				\
	} while (0)

int nr_simulated_instructions = 0;
FILE *inst_trace_fp = NULL, *cycle_trace_fp = NULL;

typedef struct sp_registers_s {
	// 6 32 bit registers (r[0], r[1] don't exist)
	int r[8];

	// 32 bit cycle counter
	int cycle_counter;

	// fetch0
	int fetch0_active; // 1 bit
	int fetch0_pc; // 16 bits

	// fetch1
	int fetch1_active; // 1 bit
	int fetch1_pc; // 16 bits

	// dec0
	int dec0_active; // 1 bit
	int dec0_pc; // 16 bits
	int dec0_inst; // 32 bits

	// dec1
	int dec1_active; // 1 bit
	int dec1_pc; // 16 bits
	int dec1_inst; // 32 bits
	int dec1_opcode; // 5 bits
	int dec1_src0; // 3 bits
	int dec1_src1; // 3 bits
	int dec1_dst; // 3 bits
	int dec1_immediate; // 32 bits

	// exec0
	int exec0_active; // 1 bit
	int exec0_pc; // 16 bits
	int exec0_inst; // 32 bits
	int exec0_opcode; // 5 bits
	int exec0_src0; // 3 bits
	int exec0_src1; // 3 bits
	int exec0_dst; // 3 bits
	int exec0_immediate; // 32 bits
	int exec0_alu0; // 32 bits
	int exec0_alu1; // 32 bits

	// exec1
	int exec1_active; // 1 bit
	int exec1_pc; // 16 bits
	int exec1_inst; // 32 bits
	int exec1_opcode; // 5 bits
	int exec1_src0; // 3 bits
	int exec1_src1; // 3 bits
	int exec1_dst; // 3 bits
	int exec1_immediate; // 32 bits
	int exec1_alu0; // 32 bits
	int exec1_alu1; // 32 bits
	int exec1_aluout;

	//dma
	int dma_busy;
	int dma_state;
	int dma_left;
	int dma_src;
	int dma_dest;


	int bht[20]; //bht - size:20
} sp_registers_t;

/*
 * Master structure
 */
typedef struct sp_s {
	// local srams
#define SP_SRAM_HEIGHT	64 * 1024
	llsim_memory_t *srami, *sramd;
    int instruction_cnt;
	unsigned int memory_image[SP_SRAM_HEIGHT];
	int memory_image_size;

	int start;

	sp_registers_t *spro, *sprn;
} sp_t;

static void sp_reset(sp_t *sp)
{
	sp_registers_t *sprn = sp->sprn;

	memset(sprn, 0, sizeof(*sprn));
}

/*
 * opcodes
 */
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

//dma states
# define DMA_IDLE 0
# define DMA_WAIT 1
# define DMA_START 2

//dma vers
int active_dma = 0;
int fin_dma = 0;
int mem_busy = 0;

static char opcode_name[32][4] = {"ADD", "SUB", "LSF", "RSF", "AND", "OR", "XOR", "LHI",
				 "LD", "ST", "COPY", "POLL", "U", "U", "U", "U",
				 "JLT", "JLE", "JEQ", "JNE", "JIN", "U", "U", "U",
				 "HLT", "U", "U", "U", "U", "U", "U", "U"};

static void dump_sram(sp_t *sp, char *name, llsim_memory_t *sram)
{
	FILE *fp;
	int i;

	fp = fopen(name, "w");
	if (fp == NULL) {
                printf("couldn't open file %s\n", name);
                exit(1);
	}
	for (i = 0; i < SP_SRAM_HEIGHT; i++)
		fprintf(fp, "%08x\n", llsim_mem_extract(sram, i, 31, 0));
	fclose(fp);
}

static void sp_ctl(sp_t *sp)
{
	sp_registers_t *spro = sp->spro;
	sp_registers_t *sprn = sp->sprn;
	int i;

	fprintf(cycle_trace_fp, "cycle %d\n", spro->cycle_counter);
	fprintf(cycle_trace_fp, "cycle_counter %08x\n", spro->cycle_counter);
	for (i = 2; i <= 7; i++)
		fprintf(cycle_trace_fp, "r%d %08x\n", i, spro->r[i]);

	fprintf(cycle_trace_fp, "fetch0_active %08x\n", spro->fetch0_active);
	fprintf(cycle_trace_fp, "fetch0_pc %08x\n", spro->fetch0_pc);

	fprintf(cycle_trace_fp, "fetch1_active %08x\n", spro->fetch1_active);
	fprintf(cycle_trace_fp, "fetch1_pc %08x\n", spro->fetch1_pc);

	fprintf(cycle_trace_fp, "dec0_active %08x\n", spro->dec0_active);
	fprintf(cycle_trace_fp, "dec0_pc %08x\n", spro->dec0_pc);
	fprintf(cycle_trace_fp, "dec0_inst %08x\n", spro->dec0_inst); // 32 bits

	fprintf(cycle_trace_fp, "dec1_active %08x\n", spro->dec1_active);
	fprintf(cycle_trace_fp, "dec1_pc %08x\n", spro->dec1_pc); // 16 bits
	fprintf(cycle_trace_fp, "dec1_inst %08x\n", spro->dec1_inst); // 32 bits
	fprintf(cycle_trace_fp, "dec1_opcode %08x\n", spro->dec1_opcode); // 5 bits
	fprintf(cycle_trace_fp, "dec1_src0 %08x\n", spro->dec1_src0); // 3 bits
	fprintf(cycle_trace_fp, "dec1_src1 %08x\n", spro->dec1_src1); // 3 bits
	fprintf(cycle_trace_fp, "dec1_dst %08x\n", spro->dec1_dst); // 3 bits
	fprintf(cycle_trace_fp, "dec1_immediate %08x\n", spro->dec1_immediate); // 32 bits

	fprintf(cycle_trace_fp, "exec0_active %08x\n", spro->exec0_active);
	fprintf(cycle_trace_fp, "exec0_pc %08x\n", spro->exec0_pc); // 16 bits
	fprintf(cycle_trace_fp, "exec0_inst %08x\n", spro->exec0_inst); // 32 bits
	fprintf(cycle_trace_fp, "exec0_opcode %08x\n", spro->exec0_opcode); // 5 bits
	fprintf(cycle_trace_fp, "exec0_src0 %08x\n", spro->exec0_src0); // 3 bits
	fprintf(cycle_trace_fp, "exec0_src1 %08x\n", spro->exec0_src1); // 3 bits
	fprintf(cycle_trace_fp, "exec0_dst %08x\n", spro->exec0_dst); // 3 bits
	fprintf(cycle_trace_fp, "exec0_immediate %08x\n", spro->exec0_immediate); // 32 bits
	fprintf(cycle_trace_fp, "exec0_alu0 %08x\n", spro->exec0_alu0); // 32 bits
	fprintf(cycle_trace_fp, "exec0_alu1 %08x\n", spro->exec0_alu1); // 32 bits

	fprintf(cycle_trace_fp, "exec1_active %08x\n", spro->exec1_active);
	fprintf(cycle_trace_fp, "exec1_pc %08x\n", spro->exec1_pc); // 16 bits
	fprintf(cycle_trace_fp, "exec1_inst %08x\n", spro->exec1_inst); // 32 bits
	fprintf(cycle_trace_fp, "exec1_opcode %08x\n", spro->exec1_opcode); // 5 bits
	fprintf(cycle_trace_fp, "exec1_src0 %08x\n", spro->exec1_src0); // 3 bits
	fprintf(cycle_trace_fp, "exec1_src1 %08x\n", spro->exec1_src1); // 3 bits
	fprintf(cycle_trace_fp, "exec1_dst %08x\n", spro->exec1_dst); // 3 bits
	fprintf(cycle_trace_fp, "exec1_immediate %08x\n", spro->exec1_immediate); // 32 bits
	fprintf(cycle_trace_fp, "exec1_alu0 %08x\n", spro->exec1_alu0); // 32 bits
	fprintf(cycle_trace_fp, "exec1_alu1 %08x\n", spro->exec1_alu1); // 32 bits
	fprintf(cycle_trace_fp, "exec1_aluout %08x\n", spro->exec1_aluout);

	fprintf(cycle_trace_fp, "\n");

	sp_printf("cycle_counter %08x\n", spro->cycle_counter);
	sp_printf("r2 %08x, r3 %08x\n", spro->r[2], spro->r[3]);
	sp_printf("r4 %08x, r5 %08x, r6 %08x, r7 %08x\n", spro->r[4], spro->r[5], spro->r[6], spro->r[7]);
	sp_printf("fetch0_active %d, fetch1_active %d, dec0_active %d, dec1_active %d, exec0_active %d, exec1_active %d\n",
		  spro->fetch0_active, spro->fetch1_active, spro->dec0_active, spro->dec1_active, spro->exec0_active, spro->exec1_active);
	sp_printf("fetch0_pc %d, fetch1_pc %d, dec0_pc %d, dec1_pc %d, exec0_pc %d, exec1_pc %d\n",
		  spro->fetch0_pc, spro->fetch1_pc, spro->dec0_pc, spro->dec1_pc, spro->exec0_pc, spro->exec1_pc);

	sprn->cycle_counter = spro->cycle_counter + 1;

	//The single processor is still in initial position, before being activated - START
	if (sp->start)
		sprn->fetch0_active = 1;

	// State: FETCH0
		//Default case - FETCH1 is shut down
	sprn->fetch1_active = 0;
		//If another instruction is needed to be fetched then activate FETCH1 state
	if (spro->fetch0_active) {
        sprn->fetch1_active = 1;
        	//Perform an instruction read from a SRAMi
	    if(!fin_dma) {
            llsim_mem_read(sp->srami, spro->fetch0_pc);
            sprn->fetch0_pc = (spro->fetch0_pc + 1) & 0x0000ffff;
            sprn->fetch1_pc = spro->fetch0_pc;
        }
	}

	// State: FETCH1
    if (spro->fetch1_active) {
    		//Activate DECODE0 state
        sprn->dec0_active = 1;
        if(!fin_dma){
        	//Extract the already read instruction from SRAMi
            sprn->dec0_pc = spro->fetch1_pc;
            sprn->dec0_inst = llsim_mem_extract_dataout(sp->srami, 31, 0);
        }
    }
    //Keep DECODE0 shut down in case an instruction was not read
    else sprn->dec0_active = 0;
	
	// State: DECODE0
    if (spro->dec0_active) {
    	//Activate DECODE1 (in case the DMA doesn't interrupt) - it will be activated in another case as well
        if(fin_dma) sprn->dec1_active = 1;
        else{
        	//Analyze opcode
            int opcode = (spro->dec0_inst >> 25) & 0x1f;

            //branch prediction (CONTROL hazard)
            if (opcode  > 15 && opcode != HLT) {
                int pc = spro->dec0_inst & 0xffff;
                //flush the pipeline
                if (spro->bht[pc % 20] > 1) {
                    sprn->fetch0_pc = pc;
                    sprn->fetch0_active = 1;
                    sprn->fetch1_active = 0;
                    sprn->dec0_active = 0;
                }
            }
            if (spro->dec1_opcode == ST && opcode == LD  && spro->dec1_active) {
                //structural hazard - stalls the pipeline. (STRUCTURAL hazard)
                sprn->fetch1_active = 0;
                sprn->dec1_active = 0;
                sprn->fetch0_active = spro->fetch1_active;
                sprn->fetch0_pc = spro->fetch1_pc;
                sprn->dec0_pc = spro->dec0_pc;
                sprn->dec0_inst = spro->dec0_inst;
                sprn->dec0_active = spro->dec0_active;
            }

            else {
            	//Extract all fields of the instruction
                sprn->dec1_immediate = spro->dec0_inst & 0xffff;
                sprn->dec1_opcode = (spro->dec0_inst >> 25) & 0x1f;
                sprn->dec1_inst = spro->dec0_inst;
                sprn->dec1_pc = spro->dec0_pc;
                sprn->dec1_dst = (spro->dec0_inst >> 22) & 0x7;
                sprn->dec1_src0 = (spro->dec0_inst >> 19) & 0x7;
                sprn->dec1_src1 = (spro->dec0_inst >> 16) & 0x7;
                sprn->dec1_active = 1;
            }
        }
    }
    //shut down DECODE1
    else sprn->dec1_active = 0;



	// State: DECODE1
    if (spro->dec1_active) {
        if(!fin_dma){
            //dec1 hazard handle (DATA hazard)
            int op = spro->exec1_opcode;
            if (spro->dec1_src0 == 1) sprn->exec0_alu0 = spro->dec1_immediate; //immediate register handling
            else if (spro->dec1_src0 == 0) sprn->exec0_alu0 = 0; //zero register handling
            else if (spro->exec1_opcode == LD && spro->exec1_active && spro->exec1_dst == spro->dec1_src0) sprn->exec0_alu0 = llsim_mem_extract_dataout(sp->sramd, 31, 0); // read after write MEMORY bypass
            else if (spro->exec1_active && spro->dec1_src0 == spro->exec1_dst && ((op >=0 && op < 8) || op == COPY || op == POLL)) sprn->exec0_alu0 = spro->exec1_aluout; // read after write ALU bypass
            else if (spro->dec1_src0 == 7 && spro->exec1_active && spro->exec1_aluout == 1 && (op  > 15 && op != HLT)) sprn->exec0_alu0 = spro->exec1_pc; // branch is taken in next stage - need to flush current write to R[7]
            else sprn->exec0_alu0 = spro->r[spro->dec1_src0]; //General cases (e.g. r2-7)


            if (spro->dec1_src1 == 1) sprn->exec0_alu1 = spro->dec1_immediate; //immediate register handling
            else if (spro->dec1_src1 == 0) sprn->exec0_alu1 = 0; //zero register handle
            else if (spro->exec1_opcode == LD && spro->exec1_active && spro->dec1_src1 == spro->exec1_dst) sprn->exec0_alu1 = llsim_mem_extract_dataout(sp->sramd, 31, 0); // read after write MEMORY bypass
            else if (spro->exec1_active && spro->exec1_dst == spro->dec1_src1 && ((op >=0 && op < 8) || op == COPY || op == POLL)) sprn->exec0_alu1 = spro->exec1_aluout; // read after write ALU bypass
            else if (spro->dec1_src1 == 7 && spro->exec1_active && spro->exec1_aluout == 1 && (op  > 15 && op != HLT)) sprn->exec0_alu1 = spro->exec1_pc; // branch is taken in next stage - need to flush current write to R[7]
            else sprn->exec0_alu1 = spro->r[spro->dec1_src1]; //General cases (e.g. r2-7)

            if (spro->dec1_opcode == LHI){ //LHI opcode settings r[dst] = {imm from 31 to 16, r[dst] from 15 to 0}
                sprn->exec0_alu1 = spro->dec1_immediate;
                sprn->exec0_alu0 = spro->r[spro->dec1_dst];
            }

            //Advance the current instruction's variables to the next state in the pipeline - EXEC0
            sprn->exec0_pc = spro->dec1_pc;
            sprn->exec0_inst = spro->dec1_inst;
            sprn->exec0_opcode = spro->dec1_opcode;
            sprn->exec0_immediate = spro->dec1_immediate;
            sprn->exec0_dst = spro->dec1_dst;
            sprn->exec0_src0 = spro->dec1_src0;
            sprn->exec0_src1 = spro->dec1_src1;
        }
        //Activate EXEC0 state
        sprn->exec0_active = 1;
    }
    //Turn off EXEC0 state
    else sprn->exec0_active = 0;



	// State: EXEC0
    if (spro->exec0_active) {
    	//setting easier-access-variables for ALU
        int alu0 = spro->exec0_alu0;
        int alu1 = spro->exec0_alu1;
        //exec0 hazard handle (DATA hazard)
        int op = spro->exec1_opcode;
        if (spro->exec0_src0 != 0 && spro->exec0_src0 != 1) { //if 2<=src0<=7
            if (spro->exec1_opcode== LD && spro->exec1_active && spro->exec0_src0 == spro->exec1_dst) alu0 = llsim_mem_extract_dataout(sp->sramd, 31, 0); // read after write MEMORY bypass
            else if (spro->exec1_active && spro->exec1_dst == spro->exec0_src0 && ((op >=0 && op < 8) || op == COPY || op == POLL)) alu0= spro->exec1_aluout;// read after write ALU bypass
            else if (spro->exec0_src0 == 7 && spro->exec1_active &&  (op  > 15 && op != HLT)) alu0 = spro->exec1_pc; // branch is taken in next stage - need to flush current write to R[7]
        }

        if (spro->exec0_src1 != 0 && spro->exec0_src1 != 1) { //if 2<=src1<=7
            if (op == LD && spro->exec1_active && spro->exec1_dst == spro->exec0_src1) alu1 = llsim_mem_extract_dataout(sp->sramd, 31, 0); // read after write MEMORY bypass
            else if (spro->exec1_active && spro->exec1_dst == spro->exec0_src1 && ((op >=0 && op < 8) || op == COPY || op == POLL)) alu1 = spro->exec1_aluout; // read after write ALU bypass
            else if (spro->exec0_src1 == 7 && spro->exec1_active && (op  > 15 && op != HLT)) alu1 = spro->exec1_pc; // branch is taken in next stage - need to flush current write to R[7]
        }

        //ALU execution operation
        if (spro->exec0_opcode == ADD) sprn->exec1_aluout = alu0 + alu1;
        else if (spro->exec0_opcode == SUB) sprn->exec1_aluout = alu0 - alu1;
        else if (spro->exec0_opcode == LSF) sprn->exec1_aluout = alu0 << alu1;
        else if (spro->exec0_opcode == RSF) sprn->exec1_aluout = alu0 >> alu1;
        else if (spro->exec0_opcode == AND) sprn->exec1_aluout = alu0 & alu1;
        else if (spro->exec0_opcode == OR) sprn->exec1_aluout = alu0 | alu1;
        else if (spro->exec0_opcode == XOR) sprn->exec1_aluout = alu0 ^ alu1;
        else if (spro->exec0_opcode ==LHI) sprn->exec1_aluout = (alu0 & 0xffff) | (alu1 << 16);
        else if (spro->exec0_opcode == LD) llsim_mem_read(sp->sramd, alu1 & 0xffff);
        else if (spro->exec0_opcode == JLT)
            if (alu0 < alu1) sprn->exec1_aluout = 1;
            else sprn->exec1_aluout = 0;
        else if (spro->exec0_opcode == JLE)
            if (alu0 <= alu1) sprn->exec1_aluout = 1;
            else sprn->exec1_aluout = 0;
        else if (spro->exec0_opcode == JEQ)
            if (alu0 == alu1) sprn->exec1_aluout = 1;
            else sprn->exec1_aluout = 0;
        else if (spro->exec0_opcode == JNE)
            if (alu0 != alu1) sprn->exec1_aluout = 1;
            else sprn->exec1_aluout = 0;
        else if (spro->exec0_opcode == POLL) sprn->exec1_aluout = spro->dma_left;

        //exec0 dma
        op = spro->exec1_opcode;
        int stat = 0;
        int op_change = 0;
        if (spro->dma_busy || (op == COPY && spro->exec1_active)) stat = 1; //Validate the DMA hasn't started a COPY already or is about to start one (if it is in EXEC1)
        if ((op >=0 && op < 8) || op == COPY || op == POLL) op_change = 1; //Make sure a Branch instruction or Halt isn't present at EXEC1
        if (spro->exec0_opcode == COPY && stat == 0) {
        //COPY instruction was inserted while the DMA isn't currently processing. Set the DMA variables for a new COPY
            if (spro->exec1_active && spro->exec1_dst == spro->exec0_src0 && op_change) // Update COPY source address register
                //src0 - DATA hazard handle (fetch the updated src0 register value from aluout of EXEC1 if it was activated in this cycle)
                sprn->dma_src = spro->exec1_aluout;
            else
                sprn->dma_src = spro->r[spro->exec0_src0];
            if (spro->exec1_dst == spro->exec0_src1 && spro->exec1_active && op_change) // Update COPY block length register
            	//src1 - DATA hazard handle (fetch the updated src1 register value from aluout of EXEC1 if it was activated in this cycle)
                sprn->dma_left = spro->exec1_aluout;
            else
                sprn->dma_left = spro->r[spro->exec0_src1]; //sprn->dma_left = spro->exec0_immediate;
            sprn->dma_dest = spro->r[spro->exec0_dst]; //Update destination address register
        }

        //Advance the current instruction's variables to the next state in the pipeline - EXEC1
        sprn->exec1_inst = spro->exec0_inst;
        sprn->exec1_alu0 = alu0;
        sprn->exec1_alu1 = alu1;
        sprn->exec1_opcode = spro->exec0_opcode;
        sprn->exec1_dst = spro->exec0_dst;
        sprn->exec1_src0 = spro->exec0_src0;
        sprn->exec1_src1 = spro->exec0_src1;
        sprn->exec1_immediate = spro->exec0_immediate;
        sprn->exec1_pc = spro->exec0_pc;
        sprn->exec1_active = 1;
    }
    //Turn off EXEC1 state
    else sprn->exec1_active = 0;



	// State: EXEC1
	if (spro->exec1_active) {
	    //print to instruction file
        sp_printf("exec1: pc %d, inst %08x, opcode %d, aluout %d\n", spro->exec1_pc, spro->exec1_inst, spro->exec1_opcode, spro->exec1_aluout);
        fprintf(inst_trace_fp, "\n--- instruction %d (%04x) @ PC %d (%04x) -----------------------------------------------------------\n", sp->instruction_cnt, sp->instruction_cnt, sp->spro->exec1_pc, sp->spro->exec1_pc);
        fprintf(inst_trace_fp, "pc = %04d, inst = %08x, opcode = %d (%s), dst = %d, src0 = %d, src1 = %d, immediate = %08x\n", sp->spro->exec1_pc, sp->spro->exec1_inst, sp->spro->exec1_opcode, opcode_name[sp->spro->exec1_opcode], sp->spro->exec1_dst, sp->spro->exec1_src0, sp->spro->exec1_src1, sbs(sp->spro->exec1_inst, 15, 0));
        fprintf(inst_trace_fp, "r[0] = %08x r[1] = %08x r[2] = %08x r[3] = %08x \n", 0, sp->spro->exec1_immediate, sp->spro->r[2], sp->spro->r[3]);
        fprintf(inst_trace_fp, "r[4] = %08x r[5] = %08x r[6] = %08x r[7] = %08x \n\n", sp->spro->r[4], sp->spro->r[5], sp->spro->r[6], sp->spro->r[7]);
        switch (sp->spro->exec1_opcode)
        {
            case ADD:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = %d ADD %d <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu0, sp->spro->exec1_alu1);
                break;
            case SUB:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = %d SUB %d <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu0, sp->spro->exec1_alu1);
                break;
            case AND:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = %d AND %d <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu0, sp->spro->exec1_alu1);
                break;
            case OR:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = %d OR %d <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu0, sp->spro->exec1_alu1);
                break;
            case XOR:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = %d XOR %d <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu0, sp->spro->exec1_alu1);
                break;
            case LHI:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d][31:16] = 0x%04x <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_immediate & 0xffff);
                break;
            case LSF:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = %d LSF %d <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu0, sp->spro->exec1_alu1);
                break;
            case RSF:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = %d RSF %d <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu0, sp->spro->exec1_alu1);
                break;
            case LD:
                fprintf(inst_trace_fp, ">>>> EXEC: R[%d] = MEM[%d] = %08x <<<<\n", sp->spro->exec1_dst, sp->spro->exec1_alu1, llsim_mem_extract_dataout(sp->sramd, 31, 0));
                break;
            case ST:
                fprintf(inst_trace_fp, ">>>> EXEC: MEM[%d] = R[%d] = %08x <<<<\n", sp->spro->exec1_alu1, sp->spro->exec1_src0, sp->spro->exec1_alu0);
                break;
            case JIN:
                fprintf(inst_trace_fp, ">>>> EXEC: JIN %d <<<<\n", sp->spro->exec1_alu0 & 0xffff);
                break;
            case HLT:
                fprintf(inst_trace_fp, ">>>> EXEC: HALT at PC %04x<<<<\n", sp->spro->exec1_pc);
                break;
            case JLT:
                fprintf(inst_trace_fp, ">>>> EXEC: JLT %d, %d, %d <<<<\n", sp->spro->exec1_alu0, sp->spro->exec1_alu1, sp->spro->exec1_aluout ? sp->spro->exec1_immediate & 0xffff : sp->spro->exec1_pc + 1);
                break;
            case JLE:
                fprintf(inst_trace_fp, ">>>> EXEC: JLE %d, %d, %d <<<<\n", sp->spro->exec1_alu0, sp->spro->exec1_alu1, sp->spro->exec1_aluout ? sp->spro->exec1_immediate & 0xffff : sp->spro->exec1_pc + 1);
                break;
            case JEQ:
                fprintf(inst_trace_fp, ">>>> EXEC: JEQ %d, %d, %d <<<<\n", sp->spro->exec1_alu0, sp->spro->exec1_alu1, sp->spro->exec1_aluout ? sp->spro->exec1_immediate & 0xffff : sp->spro->exec1_pc + 1);
                break;
            case JNE:
                fprintf(inst_trace_fp, ">>>> EXEC: JNE %d, %d, %d <<<<\n", sp->spro->exec1_alu0, sp->spro->exec1_alu1, sp->spro->exec1_aluout ? sp->spro->exec1_immediate & 0xffff : sp->spro->exec1_pc + 1);
                break;
            case COPY:
                fprintf(inst_trace_fp, ">>>> EXEC: COPY - Source address: %d, Destination address: %d, Length: %d <<<", sp->spro->dma_src, sp->spro->dma_dest, sp->spro->dma_left);
                break;
            case POLL:
                fprintf(inst_trace_fp, ">>>> EXEC: POOL res save in register %d <<<<", sp->spro->exec1_dst);
                break;
            default:
                break;
        }
        //Advance to the next instruction in the next CPU cycle
        sp->instruction_cnt += 1;
        if ((fin_dma == 1) || (spro->exec1_opcode == HLT)){
            if(spro->dma_left > 0) fin_dma = 1; //Keep cycling until DMA is done, and then finish the simulation...
            else{
            	//HALT!!!!!!!
                fprintf(inst_trace_fp, "sim finished at pc %d, %d instructions", sp->spro->exec1_pc, sp->instruction_cnt);
                fin_dma = 0;
                llsim_stop();
                dump_sram(sp, "srami_out.txt", sp->srami);
                dump_sram(sp, "sramd_out.txt", sp->sramd);
                //HALTED!!!!!
            }
        }
        //Branch instruction final execution
        else if (spro->exec1_opcode > 15 && spro->exec1_opcode != HLT){ // Branch operation
            int pc_next;
            int b_taken = 0;
            //Special case: Jump with no conditions
            if(spro->exec1_opcode == JIN){
                b_taken = 1;
                pc_next = spro->exec1_alu0 & 0xffff;
            }
            else {
            	//Other Branch instruction: validate jumping condition
                if (spro->exec1_aluout) {
                    b_taken = 1;
                    pc_next = spro->exec1_immediate & 0x0ffff;
                }
                //Branch was not taken -> proceed to consecutive instruction in SRAMi
                else pc_next = (spro->exec1_pc + 1) & 0x0ffff;
            }

            //Update bht
            if (b_taken == 1) {
                sprn->r[7] = spro->exec1_pc;
                if(spro->bht[spro->exec1_pc % 20] + 1 < 20) sprn->bht[spro->exec1_pc % 20] = spro->bht[spro->exec1_pc % 20] + 1;
            }
            else if (spro->bht[spro->exec1_pc % 20] - 1 > 0) sprn->bht[spro->exec1_pc % 20] = spro->bht[spro->exec1_pc % 20] - 1;

            //Flush pipeline if needed
            if ((spro->exec0_active && spro->exec0_pc != pc_next) || (spro->fetch0_active && spro->fetch0_pc != pc_next) || (spro->fetch1_active && spro->fetch1_pc != pc_next) || (spro->dec0_active && spro->dec0_pc != pc_next) || (spro->dec1_active && spro->dec1_pc != pc_next)){
                sprn->fetch0_pc = pc_next;
                sprn->fetch0_active = 1;
                sprn->fetch1_active = 0;
                sprn->dec0_active = 0;
                sprn->exec0_active = 0;
                sprn->dec1_active = 0;
                sprn->exec1_active = 0;
            }


        }
        //Load instruction: Read extraction
        else if (spro->exec1_opcode == LD) {
            if (spro->exec1_dst != 0 && spro->exec1_dst != 1) sprn->r[spro->exec1_dst] = llsim_mem_extract_dataout(sp->sramd, 31, 0);
        }
        //Store instruction: Store initialization & writing
        else if (spro->exec1_opcode == ST) {
            llsim_mem_set_datain(sp->sramd, spro->exec1_alu0, 31, 0);
            llsim_mem_write(sp->sramd, spro->exec1_alu1);
        }
        //All other operands : ADD,SUB,RSF,LSF,LHI,XOR,AND,OR
        else if (spro->exec1_dst != 0 && spro->exec1_dst != 1) sprn->r[spro->exec1_dst] = spro->exec1_aluout;
    }

	mem_busy = 0;
	//Activating DMA for COPY
    if (spro->exec1_opcode == COPY && active_dma == 0) active_dma = 1;
    //SRAMd access conflict prevention
    if (!fin_dma && sprn->dec1_opcode != LD && sprn->exec0_opcode != LD && sprn->exec1_opcode != LD && sprn->exec0_opcode != ST && sprn->dec1_opcode != ST && sprn->exec1_opcode != ST) mem_busy = 0;
    else if (!fin_dma) mem_busy = 1;

    //exec1 dma - DMA states nexus
    switch (spro->dma_state) {
        case DMA_IDLE:
        	//If the SRAMd is not currently accessed by the CPU & the DMA is needed to operate then allow the DMA to access the SRAMd for reading operation
            if (active_dma && !mem_busy) {
                sprn->dma_busy = 1;
                sprn->dma_state = DMA_WAIT;
            }
            //Else keep the DMA idle until is requested to operate
            else sprn->dma_state = DMA_IDLE;
            break;

        case DMA_WAIT:
        	//If the DMA is needed to copy data, then perform a read from SRAMd and proceed to Writing state
            llsim_mem_read(sp->sramd, spro->dma_src);
            sprn->dma_state = DMA_START;
            break;

        case DMA_START:
        	//DMA is in Writing state after data was read from a source address, and will now be written to a destination address
            llsim_mem_set_datain(sp->sramd, llsim_mem_extract_dataout(sp->sramd, 31, 0), 31, 0);
            llsim_mem_write(sp->sramd, spro->dma_dest);

            //DMA COPY variables update e.g. block size is reduced by 1
            sprn->dma_src = spro->dma_src + 1;
            sprn->dma_dest = spro->dma_dest + 1;
            sprn->dma_left = spro->dma_left - 1;

            //If the DMA is done copying, it becomes idle
            if ((spro->dma_left - 1 ) == 0) {
                sprn->dma_state = DMA_IDLE;
                sprn->dma_busy = 0;
                active_dma = 0;
            }
            else {
            	//Else the DMA proceeds to another Reading and Writing phase only if SRAMd is not busy
                if (mem_busy) sprn->dma_state = DMA_IDLE;
                else sprn->dma_state = DMA_WAIT;
            }
            break;
    }
}

static void sp_run(llsim_unit_t *unit)
{
	sp_t *sp = (sp_t *) unit->private;
	//	sp_registers_t *spro = sp->spro;
	//	sp_registers_t *sprn = sp->sprn;

	//	llsim_printf("-------------------------\n");

	if (llsim->reset) {
		sp_reset(sp);
		return;
	}

	sp->srami->read = 0;
	sp->srami->write = 0;
	sp->sramd->read = 0;
	sp->sramd->write = 0;

	sp_ctl(sp);
}

static void sp_generate_sram_memory_image(sp_t *sp, char *program_name)
{
        FILE *fp;
        int addr, i;

        fp = fopen(program_name, "r");
        if (fp == NULL) {
                printf("couldn't open file %s\n", program_name);
                exit(1);
        }
        addr = 0;
        while (addr < SP_SRAM_HEIGHT) {
                fscanf(fp, "%08x\n", &sp->memory_image[addr]);
                //              printf("addr %x: %08x\n", addr, sp->memory_image[addr]);
                addr++;
                if (feof(fp))
                        break;
        }
	sp->memory_image_size = addr;

        fprintf(inst_trace_fp, "program %s loaded, %d lines\n", program_name, addr);

	for (i = 0; i < sp->memory_image_size; i++) {
		llsim_mem_inject(sp->srami, i, sp->memory_image[i], 31, 0);
		llsim_mem_inject(sp->sramd, i, sp->memory_image[i], 31, 0);
	}
}

void sp_init(char *program_name)
{
	llsim_unit_t *llsim_sp_unit;
	llsim_unit_registers_t *llsim_ur;
	sp_t *sp;

	llsim_printf("initializing sp unit\n");

	inst_trace_fp = fopen("inst_trace.txt", "w");
	if (inst_trace_fp == NULL) {
		printf("couldn't open file inst_trace.txt\n");
		exit(1);
	}

	cycle_trace_fp = fopen("cycle_trace.txt", "w");
	if (cycle_trace_fp == NULL) {
		printf("couldn't open file cycle_trace.txt\n");
		exit(1);
	}

	llsim_sp_unit = llsim_register_unit("sp", sp_run);
	llsim_ur = llsim_allocate_registers(llsim_sp_unit, "sp_registers", sizeof(sp_registers_t));
	sp = llsim_malloc(sizeof(sp_t));
	llsim_sp_unit->private = sp;
	sp->spro = llsim_ur->old;
	sp->sprn = llsim_ur->new;

	sp->srami = llsim_allocate_memory(llsim_sp_unit, "srami", 32, SP_SRAM_HEIGHT, 0);
	sp->sramd = llsim_allocate_memory(llsim_sp_unit, "sramd", 32, SP_SRAM_HEIGHT, 0);
	sp_generate_sram_memory_image(sp, program_name);

	sp->start = 1;
	
	// c2v_translate_end
}
