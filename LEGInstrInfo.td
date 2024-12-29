
include "LEGInstrFormat.td"

// LEG Type Profile

def SDT_LEGCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i64>, SDTCisVT<1, i64>]>;
def SDT_LEGCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i64>, SDTCisVT<1, i64>]>;
def SDT_LEGCall         : SDTypeProfile <0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_LEGWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;

// LEG Specific Node

def LEGretflag       : SDNode<"LEGISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def LEGcallseq_start : SDNode<"ISD::CALLSEQ_START", SDT_LEGCallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def LEGcallseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_LEGCallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def LEGcall : SDNode<"LEGISD::CALL", SDT_LEGCall, [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def LEGWrapper : SDNode<"LEGISD::WRAPPER", SDT_LEGWrapper>;

let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : LEGInstPseudo<(outs), (ins i64imm:$amt1, i64imm:$amt2), "#ADJCALLSTACKDOWN",
                              [(LEGcallseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : LEGInstPseudo<(outs), (ins i64imm:$amt1, i64imm:$amt2), "#ADJCALLSTACKUP",
                              [(LEGcallseq_end timm:$amt1, timm:$amt2)]>;
}

// LEG Instructions
/*
defm ADD : LEGBinOp<0, "ADD", add>;

def RET : LEGInstBase<0, 8, (outs), (ins), "ret", [(LEGretflag)]> {
	let Size = 4;
	let Inst{26-22} = 0b00001;
}
*/

def simm27_shr2_pcrel : Operand<OtherVT> {
	let EncoderMethod = "getImmOpValSr2";
	let PrintMethod = "printBranchOperand";
	let MCOperandPredicate = [{
		int64_t Imm;
		if (MCOp.evaluteAsConstantImm(Imm))
		  retrn isShiftedInt<27, 2>(Imm);
		return MCOp.isBareSymbolRef();
	}];
	let OperandType = "OPERAND_PCREL";
}

def simm22_shr2_pcrel : Operand<OtherVT> {
	let EncoderMethod = "getImmOpValSr2";
	let PrintMethod = "printBranchOperand";
	let MCOperandPredicate = [{
		int64_t Imm;
		if (MCOp.evaluteAsConstantImm(Imm))
		  retrn isShiftedInt<22, 2>(Imm);
		return MCOp.isBareSymbolRef();
	}];
	let OperandType = "OPERAND_PCREL";
}

let isCommutable = true in {
	defm AND : LEGBinOp<2, "AND", and, uimm14>;
	defm XOR : LEGBinOp<1, "XOR", xor, uimm14>;
	defm OR : LEGBinOp<0, "OR", or, uimm14>;

	let isAdd = true in
	defm ADD : LEGBinOp<3, "ADD", add, uimm14>;
	defm MUL : LEGBinOp<5, "MUL", mul, uimm14>;
}

defm SUB : LEGBinOp<4, "SUB", sub, uimm14>;
defm DIV : LEGBinOp<6, "DIV", udiv, uimm14>;
defm SDIV : LEGBinOp<7, "SDIV", sdiv, simm14>;
defm REM : LEGBinOp<8, "REM", urem, uimm14>;
defm SREM : LEGBinOp<9, "SREM", srem, simm14>;
defm SLL : LEGBinOp<10, "SLL", shl, uimm14>;
defm SAR : LEGBinOp<11, "SAR", sra, uimm14>;
defm SLR : LEGBinOp<12, "SLR", srl, uimm14>;
defm ROTL : LEGBinOp<13, "ROTL", rotl, uimm14>;
defm ROTR : LEGBinOp<14, "ROTR", rotr, uimm14>;

class PredRel {
	string BaseOpcode;
	string PredSense; 
}

def getPredOpcode : InstrMapping {
	let FilterClass = "PredRel";
	let RowFields = ["BaseOpcode"];
	let ColFields = ["PredSense"];
	let KeyCol = ["none"];
	let ValueCols = [["true"], ["false"]];
}

let isBarrier = true, isTerminator = true in {
	let Defs = [SP], Uses = [SP] in
	def RET : LEGInstBase<1, (outs), (ins), "RET", [(LEGretflag)]> {
		let isReturn = 1;
		let Inst{28} = 1;
		let Inst{27-0} = 0;
	}

	let Uses = [SP], isCall = 1, isBarrier = 1 in
	def CALL : LEGInstBase<1, (outs), (ins GPRnopc:$tar, uimm20:$numargs), "CALL\t$tar", []> {
		bits<5> tar;
		bits<20> numargs;
		let Inst{28} = 1;
		let Inst{27-26} = 2;
		let Inst{25} = 1;
		let Inst{24-20} = tar;
		let Inst{19-0} = numargs;
	}
}

let BaseOpcode = "BR", isBranch = 1, isBarrier = 1, isTerminator = 1 in {	
	def BR : PredRel, LEGInstBase<1, (outs), (ins simm27_shr2_pcrel:$tar), "BR\t$tar", []> {
		let PredSense = "none";
	}

	def BR_NOENC : PredRel, LEGInstBase<1, (outs), (ins simm27_shr2_pcrel:$tar), "BR.<noenc>\t$tar", []>, Requires<[HasNotInsEncryption]> {
		let isBranch = true;
		bits<25> tar;
		let Inst{28} = 1;
		let Inst{27-26} = 1;
		let Inst{25} = 1;
		let Inst{24-0} = tar;
		let PredSense = "false";
	}

	def BR_ENC : PredRel, LEGInstBase<1, (outs), (ins simm27_shr2_pcrel:$tar), "BR.<enc>\t$tar", []>, Requires<[HasInsEncryption]> {
		let isBranch = true;
		bits<25> tar;
		let Inst{28} = 1;
		let Inst{27-26} = 1;
		let Inst{25} = 1;
		let Inst{24-0} = tar;
		let Size = 8;
		let PredSense = "true";
	}

}

// conditional branches must not be barriers to be detected as conditional branches in MCInstDesc
let BaseOpcode = "BRCC", isBranch = 1, isTerminator = 1 in {
	def BRCC : PredRel, LEGInstBase<1, (outs), (ins GPRnopc:$cond, simm22_shr2_pcrel:$tar), "BRCC\t$cond, $tar", []> {
		let PredSense = "none";
	}

	def BRCC_NOENC : PredRel, LEGInstBase<1, (outs), (ins GPRnopc:$cond, simm22_shr2_pcrel:$tar), "BRCC.<noenc>\t$cond, $tar", []>, Requires<[HasNotInsEncryption]> {
		let isBranch = true;
		bits<5> cond;
		bits<20> tar;
		let Inst{28} = 1;
		let Inst{27-26} = 1;
		let Inst{25} = 0;
		let Inst{24-20} = cond;
		let Inst{19-0} = tar;
		let PredSense = "false";
	}

	def BRCC_ENC : PredRel, LEGInstBase<1, (outs), (ins GPRnopc:$cond, simm22_shr2_pcrel:$tar), "BRCC.<enc>\t$cond, $tar", []>, Requires<[HasInsEncryption]> {
		let isBranch = true;
		bits<5> cond;
		bits<20> tar;
		let Inst{28} = 1;
		let Inst{27-26} = 1;
		let Inst{25} = 0;
		let Inst{24-20} = cond;
		let Inst{19-0} = tar;
		let Size = 8;
		let PredSense = "true";
	}
}

def : Pat<(br bb:$tar), (BR_NOENC simm27_shr2_pcrel:$tar)>, Requires<[HasNotInsEncryption]>;
def : Pat<(br bb:$tar), (BR_ENC simm27_shr2_pcrel:$tar)>, Requires<[HasInsEncryption]>;
def : Pat<(brcond i64:$cond, bb:$tar), (BRCC_NOENC i64:$cond, simm22_shr2_pcrel:$tar)>, Requires<[HasNotInsEncryption]>;
def : Pat<(brcond i64:$cond, bb:$tar), (BRCC_ENC i64:$cond, simm22_shr2_pcrel:$tar)>, Requires<[HasInsEncryption]>;


defm SETEQ : LEGSetCC<0, "SETEQ", seteq>;
defm SETNE : LEGSetCC<1, "SETNE", setne>;
defm SETLE : LEGSetCC<2, "SETLE", setle>;
defm SETLT : LEGSetCC<3, "SETLT", setlt>;
defm SETULE : LEGSetCC<4, "SETULE", setule>;
defm SETULT : LEGSetCC<5, "SETULT", setult>;

def : Pat<(select GPR:$cond, GPR:$ra, GPR:$rb), (ADDr (MULr (SUBr GPR:$rb, GPR:$ra), GPR:$cond), GPR:$ra)>;

//class  SetSwapPatRI<PatFrag cond_op, Instruction Inst>
//		: Pat<(set GPRnopc:$tar, (cond_op GPR:$ra, GPR:$rb)),
//		      (Inst GPRnopc:$tar, GPR:$rb, GPR:$ra)>;
//
//def : SetSwapPatRI<setgt, SETLEr>;
//def : SetSwapPatRI<setge, SETLTr>;
//def : SetSwapPatRI<setugt, SETULEr>;
//def : SetSwapPatRI<setuge, SETULTr>;

def LDI : LEGInstBase<2, (outs GPRnopc : $tar), (ins i64imm : $imm), "LDI\t$tar, $imm", [(set i64:$tar, imm:$imm)]> {
	bits<5> tar;
	let Inst{28-27} = 0;
	let Inst{26-21} = 0;
	let Inst{20-16} = tar;
	let Inst{15-0} = 0;
	let Size = 12;
	let isMoveImm = true;
}

let mayLoad = 1 in {
	/*
	def LDrri : LEGInstBase<2, (outs GPRnopc : $tar), (ins membio : $addr), "LD\t$tar, $addr", [(set i64:$tar, (load addr:$addr))]> {
		bits<21> addr;
		let Inst{28-27} = 1;
		let Inst{26} = 1;
		let Inst{25-21} = tar;
		let Inst{20-0} = addr;
	}
	*/

	def LDri : LEGInstBase<2, (outs GPRnopc : $tar), (ins membo : $addr), "LD\t$tar, $addr", [(set i64:$tar, (load addr:$addr))]> {
		bits<21> addr;
		bits<5> tar;
		let Inst{28-27} = 1;
		let Inst{26} = 0;
		let Inst{25-21} = tar;
		let Inst{20-0} = addr;
	}
}

class STOp<bits<2> width, string suffix> : LEGInstBase<2, (outs), (ins GPR : $val, membo : $addr), !strconcat(!strconcat("ST", suffix), "\t$val, $addr"), []> {
	bits<21> addr;
	bits<5> val;
	let Inst{28} = 1;
	let Inst{27-26} = width;
	let Inst{25-21} = val;
	let Inst{20-0} = addr;
	let mayStore = 1;
}

def STBri : STOp<0,"B">;
def STQri : STOp<1,"Q">;
def STHri : STOp<2,"H">;
def STri : STOp<3,"W">;

/*
let mayStore = 1 in {
	def STri : LEGInstBase<2, (outs), (ins GPR : $val, membo : $addr), "ST\t$val, $addr", [(store i64:$val, addr:$addr)]> {
		bits<21> addr;
		bits<5> val;
		let Inst{28} = 1;
		let Inst{26} = 0;
		let Inst{25-21} = val;
		let Inst{20-0} = addr;
	}
}

*/

def : Pat<(i64(LEGWrapper tglobaladdr:$addr)), (LDI tglobaladdr:$addr)>;

def : Pat<(LEGcall tglobaladdr:$addr, uimm20:$imm), (CALL (LDI tglobaladdr:$addr), uimm20:$imm)>;

def : Pat<(extload (add GPR:$base, simm16:$off)), (LDri GPR:$base, simm16:$off)>;
def : Pat<(extload GPR:$base), (LDri GPR:$base, 0)>;
def : Pat<(load (add GPR:$base, simm16:$off)), (LDri GPR:$base, simm16:$off)>;
def : Pat<(load GPR:$base), (LDri GPR:$base, 0)>;

// truncating stores
// TODO: make store atomic

multiclass StPat<PatFrag StoreOp, STOp Inst> {
	def : Pat<(StoreOp GPR:$val, GPR:$tar), (Inst GPR:$val, GPR:$tar, 0)>;
	def : Pat<(StoreOp GPR:$val, (add GPR:$tar, simm16:$off)), (Inst GPR:$val, GPR:$tar, simm16:$off)>;
}

defm : StPat<truncstorei8, STBri>;
defm : StPat<truncstorei16, STQri>;
defm : StPat<truncstorei32, STHri>;
defm : StPat<store, STri>;

//def : Pat<(truncstorei8 (i64 GPR:$val), addr:$addr),
//		(STri (ORr (SLLi (SLRi (LDri addr:$addr), 8), 8), (ANDi GPR:$val, 255)), addr:$addr)>;
//def : Pat<(truncstorei8 (i64 GPR:$val), GPR:$base),
//		(STri (ORr (SLLi (SLRi (LDri GPR:$base, 0), 8), 8), (ANDi GPR:$val, 255)), GPR:$base, 0)>;
//def : Pat<(truncstorei8 (i64 GPR:$val), (add GPR:$base, simm16:$off)),
//		(STri (ORr (SLLi (SLRi (LDri GPR:$base, simm16:$off), 8), 8), (ANDi GPR:$val, 255)), GPR:$base, simm16:$off)>;
//
//def : Pat<(truncstorei16 (i64 GPR:$val), addr:$addr),
//		(STri (ORr (SLLi (SLRi (LDri addr:$addr), 16), 16), (SLRi (SLLi GPR:$val, 48), 48)), addr:$addr)>;
//def : Pat<(truncstorei16 (i64 GPR:$val), GPR:$base),
//		(STri (ORr (SLLi (SLRi (LDri GPR:$base, 0), 16), 16), (SLRi (SLLi GPR:$val, 48), 48)), GPR:$base, 0)>;
//def : Pat<(truncstorei16 (i64 GPR:$val), (add GPR:$base, simm16:$off)),
//		(STri (ORr (SLLi (SLRi (LDri GPR:$base, simm16:$off), 16), 16), (SLRi (SLLi GPR:$val, 48), 48)), GPR:$base, simm16:$off)>;
//
//def : Pat<(truncstorei32 (i64 GPR:$val), addr:$addr),
//		(STri (ORr (SLLi (SLRi (LDri addr:$addr), 32), 32), (SLRi (SLLi GPR:$val, 32), 32)), addr:$addr)>;
//def : Pat<(truncstorei32 (i64 GPR:$val), GPR:$base),
//		(STri (ORr (SLLi (SLRi (LDri GPR:$base, 0), 32), 32), (SLRi (SLLi GPR:$val, 32), 32)), GPR:$base, 0)>;
//def : Pat<(truncstorei32 (i64 GPR:$val), (add GPR:$base, simm16:$off)),
//		(STri (ORr (SLLi (SLRi (LDri GPR:$base, simm16:$off), 32), 32), (SLRi (SLLi GPR:$val, 32), 32)), GPR:$base, simm16:$off)>;
