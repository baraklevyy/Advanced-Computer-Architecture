`include "../ex5/addsub.v"
module sat_count(clk, reset, branch, taken, prediction);
   parameter N=2;
   input clk, reset, branch, taken;
   output prediction;
   reg [3:0] cnt;
   wire [3:0] cntpp;
   reg [3:0] log = {N{1'b1}};

   addsub add55(.result(cntpp), .operand_a(cnt), .operand_b(4'b1), .mode(~taken));
   always @(posedge clk)
    begin
	if (reset)
	  cnt <= 0;
	else begin
          if (branch)
	    if (taken)
	      if (log>cnt)
	        cnt <= cntpp;
	      else
	        cnt <= log;
	    else
              if (0==cnt)
	        cnt <= 0;
	      else
	        cnt <= cntpp;
          else
	    cnt <= cnt;
        end
     end
    assign prediction = cnt[N-1];
   
endmodule

