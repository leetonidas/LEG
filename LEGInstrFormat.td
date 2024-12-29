/*
def legimm : Operand<OtherVT>;

foreach iw = [8, 16, 32, 64] in {
	def legimm#iw : Operand<!cast<ValueType>("i"#iw)>;
}

class codeFromWidth<int size> {
	bits<2> ret = !cond(!eq(size, 8) : 0,
		               !eq(size,16) : 1,
		               !eq(size,32) : 2,
		               !eq(size,64) : 3);
}

class legimmForWidth<int size> {
	DAGOperand ret = !cond(!eq(size, 8) : legimm8,
		                   !eq(size,16) : legimm16,
		                   !eq(size,32) : legimm32,
		                   !eq(size,64) : legimm64);
}

class LEGInstBase<int args, int width, dag outs, dag ins, string asmstr, list<dag> pattern>
		: Instruction {
	let Namespace = "LEG";
	bits<32> Inst;

	dag OutOperandList = outs;
	dag InOperandList = ins;
	let AsmString = asmstr;
	let Pattern = pattern;
	let Inst{31 - 30} = args;
	let Inst{29 - 28} = codeFromWidth<width>.ret;
}

multiclass LEGInst<bits<5> opcode, int args, RegisterClass rc, string asmstr, SDNode op> {
	defvar width = !head(rc.RegTypes).Size;
	defvar legimmcl = legimmForWidth<width>.ret;
	defvar immty = !cast<ValueType>("i"#width);
	defvar pattern = [(set rc : $rr, (op immty : $arg1, immty : $arg2))];
	def : LEGInstBase<args, width, (outs rc : $rr), (ins rc : $arg1, rc : $arg2), asmstr, [(set immty : $rr, (op immty : $arg1, immty : $arg2))]> {
		let Inst{26-22} = opcode;
		let Size     = 4;
		let Inst{27} = 0;
	}

	def Imm : LEGInstBase<args, width, (outs rc : $rr), (ins rc : $arg1, legimmcl : $arg2), asmstr # "i", [(set immty : $rr, (op immty : $arg1, immty : $arg2))]> {
		let Inst{26-22} = opcode;
		let Size     = !add(4,!div(width,8));
		let Inst{27} = 1;
	}
}

multiclass LEGBinOp<bits<5> opcode, string asmstr, SDNode op> {
	defm : LEGInst<opcode, 3, GPRnopc, asmstr, op>;
	defm H : LEGInst<opcode, 3, GPR32nopc, asmstr # "h" , op>;
	defm Q : LEGInst<opcode, 3, GPR16nopc, asmstr # "q", op>;
	defm B : LEGInst<opcode, 3, GPR8nopc, asmstr # "b" , op>;
}
*/
/*
let OperandType = "OPERAND_IMMEDIATE" in {
	def i14imm : Operand<i14>;
}
*/

def simm14 : Operand<i64>, ImmLeaf<i64, [{return isInt<14>(Imm);}]>;
def simm15 : Operand<i64>, ImmLeaf<i64, [{return isInt<15>(Imm);}]>;
def uimm14 : Operand<i64>, ImmLeaf<i64, [{return isUInt<14>(Imm);}]>;
def uimm20 : Operand<i64>, ImmLeaf<i64, [{return isUInt<20>(Imm);}]>;
def scale  : Operand<i64>, ImmLeaf<i64, [{return Imm == 1 || Imm == 2 || Imm == 4 || Imm = 8;}]>;
def simm16 : Operand<i64>, ImmLeaf<i64, [{return isInt<16>(Imm);}]>;

def addr : ComplexPattern<iPTR, 2, "selectAddrRegImm", [], []>;

def jtar_pcrel : Operand<OtherVT>;

def membo : Operand<iPTR> {
	let MIOperandInfo = (ops GPR, simm16);
	let PrintMethod = "printAddrModeRegImm";
	let EncoderMethod = "getAddrModeRegImm";
}

class LEGInstBase<bits<3> group, dag outs, dag ins, string asmstr, list<dag> pattern>
		: Instruction {
	let Namespace = "LEG";
	bits<32> Inst;
	let Inst{31-29} = group;
	dag OutOperandList = outs;
	dag InOperandList = ins;
	let AsmString = asmstr;
	let Pattern = pattern;
	let Size = 4;
}

class LEGInstPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
		: LEGInstBase<7, outs, ins, asmstr, pattern> {
	let isPseudo = 1;
	let isCodeGenOnly = 1;
}

multiclass LEGBinOp<bits<4> opcode, string asmstr, SDNode op, Operand immkind> {
	let Inst<28-25> = opcode in {
		def r : LEGInstBase<0, (outs GPRnopc : $rr), (ins GPR : $arg1, GPR : $arg2), asmstr # "\t$rr, $arg1, $arg2", [(set i64 : $rr, (op GPR : $arg1, GPR : $arg2))]> {
			bits<5> rr;
			bits<5> arg1;
			bits<5> arg2;
			let Inst{24} = 1;
			let Inst{23-19} = rr;
	    	let Inst{18-14} = arg1;
			let Inst{13-9} = arg2;
			let Inst{8-0} = 0;
		}

		def i : LEGInstBase<0, (outs GPRnopc : $rr), (ins GPR : $arg1, immkind : $arg2), asmstr # "I\t$rr, $arg1, $arg2", [(set i64 : $rr, (op GPR : $arg1, immkind : $arg2))]> {
			bits<5> rr;
			bits<5> arg1;
			bits<14> arg2;
			let Inst{24} = 0;
			let Inst{23-19} = rr;
	    	let Inst{18-14} = arg1;
			let Inst{13-0} = arg2;
		}

	}
}

multiclass LEGSetCC<bits<3> kind, string asmstr, PatFrag cond_op> {
	let Inst<28> = 0, Inst<27-25> = kind in {
		def r : LEGInstBase<1, (outs GPRnopc:$tar), (ins GPR:$opa, GPR:$opb), asmstr # "\t$tar, $opa, $opb", [(set i64 : $tar, (cond_op GPR:$opa, GPR:$opb))]> {
			bits<5> tar;
			bits<5> opa;
			bits<5> opb;
			let Inst{24} = 0;
			let Inst{23-19} = tar;
			let Inst{18-14} = opa;
			let Inst{13-9} = opb;
		}

		def i : LEGInstBase<1, (outs GPRnopc:$tar), (ins GPR:$opa, simm14:$opb), asmstr # "I\t$tar, $opa, $opb", [(set i64 : $tar, (cond_op GPR:$opa, simm14:$opb))]> {
			bits<5> tar;
			bits<5> opa;
			bits<14> opb;
			let Inst{24} = 1;
			let Inst{23-19} = tar;
			let Inst{18-14} = opa;
			let Inst{13-0} = opb;
		}
	}
}

/*
multiclass LEGjump<bits<2> opcode, string asmstr> {
	let Inst<28-27> = opcode in {


		def : LEGInstBase<2, (outs), (ins brtar : $arg1), asmstr # "\t$arg1", []> {
			bits<25> arg1;
			let Inst{26} = 0;
			let Inst{25} = 0;
			let Inst{24-0} = arg1;
		}

		def c : LEGInstBase<2, (outs), (ins brtar20 : $arg2)
	}
}*/