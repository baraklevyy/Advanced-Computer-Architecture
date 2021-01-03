`include "defines.vh"

/***********************************
 * CTL module
 * Students: Onn Rengingad 304845951  
 *           Barak Levy    311431894 
 *           Bar Ben Ari   204034284
 **********************************/
module CTL(
	   clk,
	   reset,
	   start,
	   sram_ADDR,
	   sram_DI,
	   sram_EN,
	   sram_WE,
	   sram_DO,
	   opcode,
	   alu0,
	   alu1,
	   aluout_wire
	   );

   // inputs
   input clk;
   input reset;
   input start;
   input [31:0] sram_DO;
   input [31:0] aluout_wire;

   // outputs
   output [15:0] sram_ADDR;
   output [31:0] sram_DI;
   output 	 sram_EN;
   output 	 sram_WE;
   output [31:0] alu0;
   output [31:0] alu1;
   output [4:0]  opcode;
   
   

   // registers
   reg [31:0] 	 r2;
   reg [31:0] 	 r3;
   reg [31:0] 	 r4;
   reg [31:0] 	 r5;
   reg [31:0] 	 r6;
   reg [31:0] 	 r7;
   reg [15:0] 	 pc;
   reg [31:0] 	 inst;
   reg [4:0] 	 opcode;
   reg [2:0] 	 dst;
   reg [2:0] 	 src0;
   reg [2:0] 	 src1;
   reg [31:0] 	 alu0;
   reg [31:0] 	 alu1;
   reg [31:0] 	 aluout;
   reg [31:0] 	 immediate;
   reg [31:0] 	 cycle_counter;
   reg [2:0] 	 ctl_state;

   integer 	 verilog_trace_fp, rc;

   initial
     begin
	verilog_trace_fp = $fopen("verilog_trace.txt", "w");
     end

   /***********************************
    * set up sram inputs (outputs from sp)
    * 
    * TODO: fill here*/
    reg      [15:0] sram_ADDR;
    reg      [31:0] sram_DI;
    reg      sram_EN;
    reg      sram_WE;
    initial
      begin
    sram_EN <= 0;
      end
      
      
     //DMA registers
     //A register array which will hold 3 elements for COPY:
     // DMA's source address for copy [0]
     reg [15:0] DMA_src_add;
     // DMA's destination address for copy [1]
     reg [15:0] DMA_dest_add;
     // DMA's copy block size [2]
     reg [15:0] DMA_b_size;
     // 16 bits for every register because the memory is of size 2 to the power of 16 (65536)
     //reg [15:0] DMA_copy [0:2];
     // DMA's data the needs to be written into memory or read from memory
     reg [31:0] DMA_data;
     // DMA write bit to that will sign the writing operation can begin or that it finished (of one data unit)
     reg DMA_write_bit;
     // DMA machine state
     reg [3:0] DMA_state;
    //**********************************/

   // synchronous instructions
   always@(posedge clk)
     begin
	if (reset) begin 
	   // registers reset
	   r2 <= 0;
	   r3 <= 0;
	   r4 <= 0;
	   r5 <= 0;
	   r6 <= 0;
	   r7 <= 0;
	   pc <= 0;
	   inst <= 0;
	   opcode <= 0;
	   dst <= 0;
	   src0 <= 0;
	   src1 <= 0;
	   alu0 <= 0;
	   alu1 <= 0;
	   aluout <= 0;
	   immediate <= 0;
	   cycle_counter <= 0;
	   ctl_state <= 0;
	   //DMA reset
	   DMA_state <= `DMA_STATE_IDLE;
	   DMA_write_bit <= 0;
	   DMA_src_add<=0;//DMA_copy[0] <= 0;
	   DMA_dest_add<=0;//DMA_copy[1] <= 0;
	   DMA_b_size<=0;//DMA_copy[2] <= 0;
	   DMA_data <= 0;
	   
	   
	end else begin
	   // generate cycle trace
	   $fdisplay(verilog_trace_fp, "cycle %0d", cycle_counter);
	   $fdisplay(verilog_trace_fp, "r2 %08x", r2);
	   $fdisplay(verilog_trace_fp, "r3 %08x", r3);
	   $fdisplay(verilog_trace_fp, "r4 %08x", r4);
	   $fdisplay(verilog_trace_fp, "r5 %08x", r5);
	   $fdisplay(verilog_trace_fp, "r6 %08x", r6);
	   $fdisplay(verilog_trace_fp, "r7 %08x", r7);
	   $fdisplay(verilog_trace_fp, "pc %08x", pc);
	   $fdisplay(verilog_trace_fp, "inst %08x", inst);
	   $fdisplay(verilog_trace_fp, "opcode %08x", opcode);
	   $fdisplay(verilog_trace_fp, "dst %08x", dst);
	   $fdisplay(verilog_trace_fp, "src0 %08x", src0);
	   $fdisplay(verilog_trace_fp, "src1 %08x", src1);
	   $fdisplay(verilog_trace_fp, "immediate %08x", immediate);
	   $fdisplay(verilog_trace_fp, "alu0 %08x", alu0);
	   $fdisplay(verilog_trace_fp, "alu1 %08x", alu1);
	   $fdisplay(verilog_trace_fp, "aluout %08x", aluout);
	   $fdisplay(verilog_trace_fp, "cycle_counter %08x", cycle_counter);
	   $fdisplay(verilog_trace_fp, "ctl_state %08x\n", ctl_state);
	   //DMA notes
	   //$fdisplay(verilog_trace_fp, "DMA_STATE %08x", DMA_state); //DMA
       //$fdisplay(verilog_trace_fp, "DMA_src %08x", DMA_copy[0]); //DMA
       //$fdisplay(verilog_trace_fp, "DMA_dst %08x", DMA_copy[1]); //DMA
       //$fdisplay(verilog_trace_fp, "DMA_length %08x", DMA_copy[2]); //DMA
       //$fdisplay(verilog_trace_fp, "DMA_data %08x", DMA_data); //DMA
       //$fdisplay(verilog_trace_fp, "DMA_write %08x\n", DMA_write_bit); //DMA

	   cycle_counter <= cycle_counter + 1;
	   case (ctl_state)
	     `CTL_STATE_IDLE: begin
                pc <= 0;
                if (start)
                  ctl_state <= `CTL_STATE_FETCH0;
             end //  `CTL_STATE_IDLE
         /***********************************
          * TODO: fill here*/
         `CTL_STATE_FETCH0: begin   //with DNA_STATE_START  
                ctl_state <= `CTL_STATE_FETCH1;
             end //  `CTL_STATE_FETCH0
         `CTL_STATE_FETCH1: begin  //with DNA_STATE_READ
                //sram_EN <= 0;
                inst <= sram_DO [31:0];
                ctl_state <= `CTL_STATE_DEC0;
             end //  `CTL_STATE_FETCH1
         `CTL_STATE_DEC0: begin   //with DNA_STATE_WRITE
                opcode <= inst [29:25];
                dst    <= inst [24:22];
                src0   <= inst [21:19];
                src1   <= inst [18:16];
                // immediate signed\unsigned adjustment
                if(0 != inst[15]) immediate <= 65535<<16;
                immediate[15:0] <= inst[15:0];
                ctl_state <= `CTL_STATE_DEC1;
             end //  `CTL_STATE_DEC0
          `CTL_STATE_DEC1: begin
                case (opcode)
                    `ADD,`SUB,`LSF,`RSF,`AND,`OR,`XOR,`LD,`ST,`COPY,`POLL,`JLT,`JLE,`JEQ,`JNE,`JIN,`HLT: begin
                        //Alu0 with src0  
                          if(0==src0)  alu0 <= 0; else;
                          if(1==src0)  alu0 <= immediate; else;
                          if(2==src0)  alu0 <= r2; else;
                          if(3==src0)  alu0 <= r3; else;
                          if(4==src0)  alu0 <= r4; else;
                          if(5==src0)  alu0 <= r5; else;
                          if(6==src0)  alu0 <= r6; else;
                          if(7==src0)  alu0 <= r7; else;
                        //Alu1 with src1                    
                          if(0==src1)  alu1 <= 0; else;
                          if(1==src1)  alu1 <= immediate; else;
                          if(2==src1)  alu1 <= r2; else;
                          if(3==src1)  alu1 <= r3; else;
                          if(4==src1)  alu1 <= r4; else;
                          if(5==src1)  alu1 <= r5; else;
                          if(6==src1)  alu1 <= r6; else;
                          if(7==src1)  alu1 <= r7; else;
                        end
                    `LHI: begin
                        //Alu0 with immediate
                          alu0 <= immediate;           
                        //Alu1 with dst                    
                          if(0==dst)  alu1 <= 0; else;
                          if(1==dst)  alu1 <= immediate; else;
                          if(2==dst)  alu1 <= r2; else;
                          if(3==dst)  alu1 <= r3; else;
                          if(4==dst)  alu1 <= r4; else;
                          if(5==dst)  alu1 <= r5; else;
                          if(6==dst)  alu1 <= r6; else;
                          if(7==dst)  alu1 <= r7; else;
                         end
                    endcase // opcode
                  ctl_state <= `CTL_STATE_EXEC0;
              end //  `CTL_STATE_DEC1
          `CTL_STATE_EXEC0: begin
                if((opcode != `LD) && (opcode != `ST) && (opcode != `HLT)) begin   ///
                    aluout <= aluout_wire;
                  end
                 else; 
                 ctl_state <= `CTL_STATE_EXEC1;
              end // `CTL_STATE_EXEC0
          `CTL_STATE_EXEC1: begin
                pc <= pc+1;
                case(opcode)
                    `ADD,`SUB,`LSF,`RSF,`AND,`OR,`XOR,`LHI: begin
                        if(2==dst)  r2 <= aluout_wire; else;
                        if(3==dst)  r3 <= aluout_wire; else;
                        if(4==dst)  r4 <= aluout_wire; else;
                        if(5==dst)  r5 <= aluout_wire; else;   
                        if(6==dst)  r6 <= aluout_wire; else;   
                        if(7==dst)  r7 <= aluout_wire; else;                   
                       end
                     `LD: begin
                        if(2==dst)  r2 <= sram_DO; else;
                        if(3==dst)  r3 <= sram_DO; else;
                        if(4==dst)  r4 <= sram_DO; else;
                        if(5==dst)  r5 <= sram_DO; else;   
                        if(6==dst)  r6 <= sram_DO; else;   
                        if(7==dst)  r7 <= sram_DO; else; 
                       end 
                       //DMA COPY
                     `COPY: begin
                        if(DMA_state == `DMA_STATE_IDLE) begin
                            if(2==dst)  DMA_dest_add <= r2 [15:0]; else;
                            if(3==dst)  DMA_dest_add <= r3 [15:0]; else;
                            if(4==dst)  DMA_dest_add <= r4 [15:0]; else;
                            if(5==dst)  DMA_dest_add <= r5 [15:0]; else;   
                            if(6==dst)  DMA_dest_add <= r6 [15:0]; else;   
                            if(7==dst)  DMA_dest_add <= r7 [15:0]; else; 
                            DMA_src_add <= alu0 [15:0]; // set the source add
                            DMA_b_size <= alu1 [15:0]; // set the block size
                            //Initiate DMA operation
                            DMA_state <= `DMA_STATE_START;
                          end //DMA_state == `DMA_STATE_IDLE
                       end
                       //DMA POLL current copy
                     `POLL: begin
                        if(2==dst)  r2 <= DMA_b_size; else;
                        if(3==dst)  r3 <= DMA_b_size; else;
                        if(4==dst)  r4 <= DMA_b_size; else;
                        if(5==dst)  r5 <= DMA_b_size; else;   
                        if(6==dst)  r6 <= DMA_b_size; else;   
                        if(7==dst)  r7 <= DMA_b_size; else;
                       end 
                     `JLT,`JLE,`JEQ,`JNE,`JIN: begin
                        if (aluout == 1) begin
                           r7 <= pc;
                           pc <= immediate;
                         end
                       end
                  endcase // opcode
                if (opcode == `HLT) begin
                   if (DMA_state != `DMA_STATE_IDLE)begin 
                        pc <= pc - 1; end
                   else begin
                       $fclose(verilog_trace_fp);
                       $writememh("verilog_sram_out.txt", top.SP.SRAM.mem);
                       $finish;
                     end
                end // opcode == HLT
                else begin
                   ctl_state <= `CTL_STATE_FETCH0; 
              end
           end // case `CTL_STATE_EXEC1
          default: begin end
         //**********************************/
	   endcase //ctl_state
	   
	   //DMA machine states during a copy operation
	   case(DMA_state)
	      `DMA_STATE_START: 
               DMA_state <= `DMA_STATE_READ;
          `DMA_STATE_READ: 
               //DMA_state <= `DMA_STATE_WRITE;
               DMA_state <= `DMA_STATE_PREPARE;
          `DMA_STATE_PREPARE: begin
               DMA_data <= sram_DO;
               DMA_state <= `DMA_STATE_WRITE;
             end
          `DMA_STATE_WRITE: begin
               DMA_state <= `DMA_STATE_UPDATE;
              end
           `DMA_STATE_UPDATE: begin
               if (DMA_b_size != 0) //length>0
                   DMA_b_size <= DMA_b_size - 1;
               DMA_src_add <= DMA_src_add + 1; //Advancing source address
               DMA_dest_add <= DMA_dest_add + 1; //Advancing destination address
               if (DMA_b_size > 1) 
                   DMA_state <= `DMA_STATE_WAIT; //WAIT another two states for LD\ST sync
               else     DMA_state <= `DMA_STATE_IDLE;
              end
           `DMA_STATE_WAIT:
                DMA_state <= `DMA_STATE_START;
       endcase // case(dma_state)
	   
	   
	end // !reset
   end // @posedge(clk)
     
     
   //Whenever there is a change of states in control or changes of any of the sram's ports we enter..  
   always@(sram_EN or ctl_state or sram_ADDR or sram_DI or sram_WE or DMA_state)
      begin
         sram_ADDR = 0;
         sram_DI = 0;
         sram_WE = 0;
         sram_EN = 0;
         case (ctl_state)
            `CTL_STATE_FETCH0: begin
                sram_ADDR = pc[15:0];
                sram_DI = 0;
                sram_EN = 1;
                sram_WE = 0;
               end
            `CTL_STATE_EXEC0: begin
                if (opcode == `LD) begin
                     sram_ADDR = alu1[15:0];
                     sram_DI = 0;
                     sram_EN = 1;
                     sram_WE = 0;
                  end
               end
            `CTL_STATE_EXEC1: begin
                if (opcode == `ST) begin
                     sram_ADDR = alu1[15:0];
                     sram_DI = alu0;
                     sram_EN = 1;
                     sram_WE = 1;
                   end
               end
            default: begin
               sram_ADDR = 0;
               sram_DI = 0;
               sram_EN = 0;
               sram_WE = 0;
             end
         endcase // ctl_state
          
          if (DMA_write_bit == 0) begin   
              if(DMA_state == `DMA_STATE_READ) begin
                  sram_ADDR = DMA_src_add;
                  sram_DI = 0;
                  sram_EN = 1;
                  sram_WE = 0;
                  DMA_write_bit = 1;
                 end           
               end // if(DMA_write == 0)
          else begin
              if(DMA_state == `DMA_STATE_WRITE) begin
                  sram_ADDR = DMA_dest_add;
                  sram_DI = DMA_data;
                  sram_EN = 1;
                  sram_WE = 1;
                  DMA_write_bit = 0;
                 end
               end // if(DMA_write != 0)
            
      end // @always@(ctl_state or sram_ADDR or sram_DI or sram_EN or sram_WE)    



endmodule // CTL
