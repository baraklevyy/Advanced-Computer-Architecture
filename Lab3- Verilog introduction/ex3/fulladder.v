module fulladder( sum, co, a, b, ci);
  input   a, b, ci;
  output  sum, co;
  wire s, c1, c2, c3;


  xor(s,a,b);
  xor(sum,ci,s);
  and(c1,a,b);
  and(c2,b,ci);
  and(c3,a,ci);
  or(co,c1,c2,c3);

endmodule
