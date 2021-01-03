`include "../ex3/fulladder.v"
module  add4( sum, co, a, b, ci);
  input   [3:0] a, b;
  input   ci;
  output  [3:0] sum;
  output  co;
  wire [2:0] residuals;
  fulladder g0(sum[0], residuals[0],a[0], b[0], ci);
  fulladder g1(sum[1], residuals[1],a[1], b[1], residuals[0]);
  fulladder g2(sum[2], residuals[2],a[2], b[2], residuals[1]);
  fulladder g3(sum[3], co,a[3], b[3], residuals[2]);

endmodule


