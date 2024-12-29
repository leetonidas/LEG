
def CC_LEG : CallingConv<[
	CCIfType<[i128], CCAssignToRegWithShadow<[R0, R2, R4, R6], [R1, R3, R5, R7]>>,
	CCIfType<[i64], CCAssignToReg<[R0, R1, R2, R3, R4, R5, R6, R7]>>,
//	CCIfType<[i32], CCAssignToReg<[R0h, R1h, R2h, R3h, R4h, R5h, R6h, R7h]>>,
//	CCIfType<[i16], CCAssignToReg<[R0q, R1q, R2q, R3q, R4q, R5q, R6q, R7q]>>,
//	CCIfType<[i8], CCAssignToReg<[R0b, R1b, R2b, R3b, R4b, R5b, R6b, R7b]>>,
	CCAssignToStack<8, 8>
]>;

def RetCC_LEG : CallingConv<[
	CCIfType<[i128], CCAssignToRegWithShadow<[R0, R2, R4, R6], [R1, R3, R5, R7]>>,
	CCIfType<[i64], CCAssignToReg<[R0, R1, R2, R3, R4, R5, R6, R7]>>,
//	CCIfType<[i32], CCAssignToReg<[R0h, R1h, R2h, R3h, R4h, R5h, R6h, R7h]>>,
//	CCIfType<[i16], CCAssignToReg<[R0q, R1q, R2q, R3q, R4q, R5q, R6q, R7q]>>,
//	CCIfType<[i8], CCAssignToReg<[R0b, R1b, R2b, R3b, R4b, R5b, R6b, R7b]>>
]>;

def CSR_LEG : CalleeSavedRegs<(add BP, (sequence "R%u", 28, 16))>;
