module main;
   reg clk, reset, branch, taken;
   wire prediction;
   reg reg0, reg1, reg2, reg3;
   sat_count count(.clk(clk), .reset(reset), .branch(branch), .taken(taken), .prediction(prediction));
   always #5 clk = ~clk;
   initial
     begin
        $dumpfile("waves.vcd");
        $dumpvars;
	clk = 0;
	branch = 0;
	taken = 0;
	reset = 1;
	reg1 = 1;
	reg2 = 1;
	reg3 = 1;
	reg0 = 1;
        #10;
	reset = 0;
        reg0 = reg0 & (~prediction);
        if (reg0)
	$display("RESET SUCCEDD");
        #10;
        taken = 1;
        #20;
        reg1 = reg1 & (~prediction);
        if (reg1)
	$display("BRANCH SUCCEED");
	#10;
	branch = 1;
	taken = 1;
        #10;
        reg2 = reg2 & (~prediction);
        #10;
        reg2 = reg2 & (prediction);
        #10;
        reg2 = reg2 & (prediction);
        #20;
        reg2 = reg2 & (prediction);
        if (reg2)
	$display("TAKEN SUCCEED CASE1");
        #10;
        branch = 1;
        taken = 0;
        #10;
        reg3 = reg3 & (prediction);
        #10;
        reg3 = reg3 & (~prediction);
        #10;
        reg3 = reg3 & (~prediction);
        #20; 
        reg3 = reg3 & (~prediction);
        if (reg3)
	$display("TAKEN SUCCEED CASE2");
	if (reg0 & reg1 & reg2 & reg3)
	$display("OVERALL CORRECT");
        $finish;
     end 
endmodule

