// control states
`define CTL_STATE_IDLE 0
`define CTL_STATE_FETCH0 1
`define CTL_STATE_FETCH1 2
`define CTL_STATE_DEC0 3
`define CTL_STATE_DEC1 4
`define CTL_STATE_EXEC0 5
`define CTL_STATE_EXEC1 6

// DMA states
`define DMA_STATE_IDLE 0
`define DMA_STATE_START 1
`define DMA_STATE_READ 2
`define DMA_STATE_PREPARE 3
`define DMA_STATE_WRITE 4
`define DMA_STATE_UPDATE 5     
`define DMA_STATE_WAIT 6        //state 6 is used to synchronize between the DMA's states
                                // and the processor states, so the two wouldn't conflict


// opcodes
`define ADD 0
`define SUB 1
`define LSF 2
`define RSF 3
`define AND 4
`define OR 5
`define XOR 6
`define LHI 7
`define LD 8
`define ST 9
    //DMA opcodes
`define COPY 10
`define POLL 11
    //end DMA
`define JLT 16
`define JLE 17
`define JEQ 18
`define JNE 19
`define JIN 20
`define HLT 24
