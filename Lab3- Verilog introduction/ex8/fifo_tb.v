`timescale 1ns / 1ps


module fifo_tb();
   reg [1:0] in; //M=2 inputs
   reg clk, reset, push, pop;
   wire [1:0] out; //M=2 out - [M-1:0] 
   wire full;
   
   // Module correctness validation registers
   //reg out_n_validation, fullQ_validation;
   reg emptyQ_validation,n_equals_zero_validation, other_n_validation, n_equals_N_validation;
   fifo #(4,2) uut(clk, reset, in, push, pop, out, full);
   always #5 clk = ~clk;
   
   //$display("time %d: in %b, out %b", $time, in, out);
   
   initial
     begin
        $dumpfile("waves.vcd");
        $dumpvars;
        clk = 0;
        //Input initialization
        push = 0;
        pop = 0;
        in = 2'b10; // First element is arbitrarily set to '2'
        reset = 1;
        //Correctness validation bits initialization
        emptyQ_validation=1; //
        n_equals_zero_validation=1; //
        other_n_validation=1;//
        n_equals_N_validation=1;//
        //t=-1
        #10; //t=0
        reset = 0; // reset turn off
        emptyQ_validation = emptyQ_validation & (~out) & (~full);
        if (emptyQ_validation)
	$display("RESET SUCCEEDED");
	//..............................Run begins here..........................   
        //push == 0 (reset is now off); pop is * 
        #10;    //t=1: W(t) = <>, W(t+1) = <>
        // Case: push == 0; pop == 0; when empty
        n_equals_zero_validation = n_equals_zero_validation & (~out) & (~full);
        //push == 1 ->Inserting the number '2' to the queue that was initiated beforehand
        push = 1;
        #10;    //t=2: W(t) = <>, W(t+1) = <2> -> out(t) should be NULL
        // Case: push == 1; pop == 0; while empty
        n_equals_zero_validation = n_equals_zero_validation & (~out) & (~full);
        push = 0;
        #10;    //t=3: W(t) = <2>, W(t+1) = <2> -> out(t) should be '2'
        // Case: push == 0; pop == 0; when not empty but in order to VALIDATE out(t)
        n_equals_zero_validation = n_equals_zero_validation & (out == 2'b10) & (~full);
        //Maybe add emptying the queue and validating that out == 0; -> would take you 10ns to empty (pop=1) in which out=='2' and another 10ns to keep the queue empty and out==NULL
        pop = 1;
        #10;    //t=4: W(t) = <2>, W(t+1) = <> -> out(t) should be '2'
        // Case: push == 0; pop == 1; //EMPTYING QUEUE
        n_equals_zero_validation = n_equals_zero_validation & (out == 2'b10) & (~full); //Queue is emptied properly
        #10;    //t=5: W(t) = <>, W(t+1) = <> -> out(t) should be NULL
        // Case: push == 0; pop == 1; while Empty
        n_equals_zero_validation = n_equals_zero_validation & (~out) & (~full);         //Queue ignores pop while being emptied properly
        push=1;
        #10;    //t=6: W(t) = <>, W(t+1) = <2> -> out(t) should be NULL                 //Queue ignores pop while being filled with one element when already emptied
        // Case: push == 1; pop == 1; while Empty
        n_equals_zero_validation = n_equals_zero_validation & (~out) & (~full);
        if (n_equals_zero_validation)
	$display("n-EQUALS-ZERO CASES SUCCEEDED");
	//Inserting more elements to the queue to make n -> 0<n<N
	    pop=0;
        in = 2'b01;
        #10;    //t=7: W(t) = <2>, W(t+1) = <1,2> -> out(t) should be '2'
        other_n_validation = other_n_validation & (out == 2'b10) & (~full);
        in = 2'b11;
        #10     //t=8: W(t) = <1,2>, W(t+1) = <3,1,2> -> out(t) should be '2'
        
        
        //Another correctness validation for the case there are 3 elements in the 4-items-queue
        // Case: push == 1; pop == 0; 
        other_n_validation = other_n_validation & (out == 2'b10) & (~full);
        //pop an element and insert the number '0'
        pop = 1;
        in = 2'b00;
        #10     // t=9: W(t) = <3,1,2>, W(t+1) = <0,3,1> -> out(t) should be '2'
        // Case: push == 1; pop == 1; 
        other_n_validation = other_n_validation & (out == 2'b10) & (~full);
        //pop an element without inserting ANYTHING
        push = 0;
        #10     // t=10: W(t) = <0,3,1>, W(t+1) = <0,3> -> out(t) should be '1'
        // Case: push == 0; pop == 1; 
        other_n_validation = other_n_validation & (out == 2'b01) & (~full);
        //don't do anything
        pop = 0;
        #10     // t=11: W(t) = <0,3>, W(t+1) = <0,3> -> out(t) should be '3'
        other_n_validation = other_n_validation & (out == 2'b11) & (~full);
        if (other_n_validation)
    $display("OTHER-n CASES SUCCEEDED");
    //Inserting more elements to the queue to make n==N ( full queue )
        push = 1;
        in = 2'b01;
        #10;    // t=12: W(t) = <0,3>, W(t+1) = <1,0,3> -> out(t) should be '3'
        n_equals_N_validation = n_equals_N_validation & (out == 2'b11) & (~full);
        in = 2'b10;
        #10     // t=13: W(t) = <1,0,3>, W(t+1) = <2,1,0,3> -> out(t) should be '3' still NOT FULL IN TIME t
        //Another correctness validation for the case there are 3 elements in the 4-items-queue
        n_equals_N_validation = n_equals_N_validation & (out == 2'b11) & (~full);
        
        
        #10;    // t=14: W(t) = <2,1,0,3>, W(t+1) = <2,1,0,3> -> out(t), full(t) should be 1
        //Attempting to insert another element when the queue is already FULL
        // Case: push == 1; pop == 0; when full
        n_equals_N_validation = n_equals_N_validation & (out == 2'b11) & (full);
        push = 0;
        #10;    // t=15: W(t) = <2,1,0,3>, W(t+1) = <2,1,0,3> -> out(t) should be '3', full(t) should be 1
        // Case: push == 0; pop == 0; when full
        n_equals_N_validation = n_equals_N_validation & (out == 2'b11) & (full);
        // Insert one element and pop one
        push = 1;
        in = 2'b01;
        pop = 1;
        #10     // t=16: W(t) = <2,1,0,3>, W(t+1) = <1,2,1,0> -> out(t) should be '3', full(t) should be 1
        // Case: push == 1; pop == 1; when full
        n_equals_N_validation = n_equals_N_validation & (out == 2'b11) & (full);
        push = 0;
        #10     // t=17: W(t) = <1,2,1,0>, W(t+1) = <1,2,1> -> out(t) should be '0', full(t) should be 1
        // Case: push == 0; pop == 1; when full        
        n_equals_N_validation = n_equals_N_validation & (out == 2'b00) & (full);
        pop=0;
        #10     // t=18: W(t) = <1,2,1,>, W(t+1) = <1,2,1> -> out(t) should be '1', full(t) should be 0
        n_equals_N_validation = n_equals_N_validation & (out == 2'b01) & (~full);   //Attempting to see full(t) change when the queue is not longer full
        if (n_equals_N_validation)
    $display("n-EQUALS-N CASES SUCCEEDED");
        
	if (emptyQ_validation & n_equals_zero_validation & other_n_validation & n_equals_N_validation)
	$display("OVERALL CORRECT");
	 $finish;
     end 
endmodule



