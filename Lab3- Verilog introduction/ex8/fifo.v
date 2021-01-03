module fifo(clk, reset, in, push, pop, out, full);
   parameter N=4; // determines the maximum number of words in queue.
   parameter M=2; // determines the bit-width of each word, stored in the queue.

   input clk, reset, push, pop;
   input [M-1:0] in;
   output [M-1:0] out;
   output full;

   //integer n=0; // validate with Barak
   integer is_full = 0;	
   integer is_queue_empty = 0;
   reg [M*N-1:0] w_at_t = 0;
   reg [M-1:0] output_current = 0;
	always @(posedge clk or reset) begin
		if(reset == 1) begin
		w_at_t <= 0; is_queue_empty <= 0; is_full <= 0; output_current = 0; 
		end
		else //reset == 0
		  begin
		  //....................Update full(t) bit					
          if(is_queue_empty < N)
                is_full <= 0;
          else
                is_full <= 1;
          //.....................Updating output(t)
          output_current <= w_at_t & ((1 << M) - 1);
		  
		  //.....................Updating the queue - W(t+1)
		  if (0 == is_queue_empty) // Case 1: Empty
		    begin //To an empty queue you can only insert items pop==*
		      if (1 == push) begin
                  w_at_t <= (in<<(M*is_queue_empty)) | w_at_t ; //(in<<(M*n)) | w_at_t ;
                  is_queue_empty <=is_queue_empty + 1;
			  end
		      else begin 
                  w_at_t <= w_at_t;
                  is_queue_empty <= is_queue_empty;
			  end
		    end // Case 1: End
	      else //Queue isn't empty -> consider two cases: either full (n==N) or partly full(0<n>N)
            begin
			  if(N > is_queue_empty) begin //Case 2: Partly full
			      if (pop == 1 && push == 0) begin
			        w_at_t <= (w_at_t>>>M);
                    is_queue_empty <= is_queue_empty-1; //n<=n-1;
			        end else;
			      if (1 == push && 0 == pop) begin
			        w_at_t <= (w_at_t | in <<(M* is_queue_empty));
				    is_queue_empty <= is_queue_empty +1;
			        end else;
			      if (1 == push && 1 == pop) begin
			        w_at_t <= (w_at_t>>>M) | (in << (M*(is_queue_empty-1)));
			        is_queue_empty<=is_queue_empty;
				    end else;
				  if (0 == push && 0 == pop)begin
			        w_at_t <= w_at_t;
                    is_queue_empty<=is_queue_empty;
				    end else;
			  end // Case 2: End
			  else //Case 3: Full
                begin
				  if(0 == push && 1 == pop) begin
					 w_at_t <= (w_at_t >> M);
					 is_queue_empty<=is_queue_empty - 1;
					 end else;
				  if(1 == push && 1 == pop) begin
					 w_at_t <= (w_at_t>>>M) | (in<<(M*(is_queue_empty-1)));                          						  
					 is_queue_empty<=is_queue_empty;
					 end else; 
				  if(0 == pop) begin 
					 w_at_t <= w_at_t; 				 	          
					 is_queue_empty<=is_queue_empty;
					 end else;
				  end//Case 3: End
			end // Full and partly full End
		  
		  
	   
	   end //End reset == 0 
    end //End @always
    
    //...............Assign output registers to wires..................
    assign out = output_current;
    assign full = is_full; 
endmodule
