/**
 * Onn Rengingad 304845951
 *
 * Bar Ben Ari 204034284
 * 
 * Barak Levy 311431894
 */

/* initialize variables */
asm_cmd(ADD, 2, 1, 0, 1); // 0: R2 = 1
asm_cmd(ST, 0, 2, 1, 1000); // 1: mem[1000] = R2
asm_cmd(ADD, 3, 0, 2, 0); // 2: R3 = R2
asm_cmd(ST, 0, 3, 1, 1001); // 3: mem[1001] = R3
asm_cmd(ADD, 4, 0, 1, 1040); // 4: R4 = 1040
asm_cmd(ADD, 5, 0, 1, 1002); // 5: R5 = 1002

/* loop that calculate the 3-40 Fibonacci numbers */
asm_cmd(JEQ, 0, 4, 5, 13); // 6: if R4 == R5 jump to pc = 13
asm_cmd(ADD, 6, 2, 3, 0); // 7: R6 = R2 + R3
asm_cmd(ST, 0, 6, 5, 0); // 8: mem[R5] = R6 
asm_cmd(ADD, 2, 3, 0, 0); // 9: R2 = R3
asm_cmd(ADD, 3, 6, 0, 0); // 10: R3 = R6
asm_cmd(ADD, 5, 5, 1, 1); // 11: R5 += 1
asm_cmd(JEQ, 0, 0, 0, 6); // 12: jump to pc = 6
asm_cmd(HLT, 0, 0, 0, 0); // 13: halt

