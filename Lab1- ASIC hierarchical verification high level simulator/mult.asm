/**
 * Onn Rengingad 304845951
 *
 * Bar Ben Ari 204034284
 *
 * Barak Levy 311431894
 */

/* initialize variables */
asm_cmd(LD, 2, 0, 1, 1000); // 0: R2 = Memory[1000] Loading Multiplicand
asm_cmd(LD, 3, 0, 1, 1001); // 1: R3 = Memory[1001] Loading Multiplier
asm_cmd(ADD, 5, 0, 0, 0);	// 2: R5 = 0 => R5 is defined to be the RESULT of the multiplication

/* Main loop that computes the multiplication of the two numbers */
asm_cmd(AND, 4, 3, 1, 1);	// 3: R4 = R3&1 => The LSB of R3 describes the contribution of the Multiplier   (If it is 1, than sum it, otherwise, don't)
asm_cmd(JEQ, 0, 4, 0, 6);	// 4: JUMP - skipping the Multiplier's contribution, because its' CURRENT LSB is zero
asm_cmd(ADD, 5, 5, 2, 0);	// 5: R5 = R5+R2 => Result update   (If the current Multiplier LSB is 1, than sum it, otherwise, don't)
asm_cmd(LSF, 2, 2, 1, 1);	// 6: R2 = R2<<1 => Left shift for the becoming-greater Multiplicand
asm_cmd(RSF, 3, 3, 1, 1);	// 7: R3 = R3>>1 => Right shift of the Multiplier   (for further scanning)
asm_cmd(JNE, 0, 3, 0, 3);	// 8: JUMP to another addition if the Multiplier hasn't been scanned completely   (meaning - different than zero)

/* Last section that saves the multiplication result in the needed memory slot and exits */
asm_cmd(ST, 0, 5, 1, 1002); // 9: Memory[1002] = R5 => Storing the multiplication result at 1002
asm_cmd(HLT, 0, 0, 0, 0);	// 10: HALT!

