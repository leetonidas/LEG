#include "LEGInstPrinter.h"

#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"

#include "llvm/Support/FormattedStream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define PRITN_ALIAS_INSTR
#include "LEGGenAsmWriter.inc"

void LEGInstPrinter::printInst(const MCInst *MI, uint64_t Address, StringRef Annot, const MCSubtargetInfo &STI, raw_ostream &O) {
	//if (!printAliasInstr(MI, Address, STI, O))
	printInstruction(MI, Address, STI, O);

	printAnnotation(O, Annot);
}

void LEGInstPrinter::printOperand(const MCInst *MI, unsigned int OpNo, const MCSubtargetInfo &STI, raw_ostream &O, const char *Modifier) const {
	const MCOperand &Op = MI->getOperand(OpNo);
	if (Op.isReg()) {
		O << getRegisterName(Op.getReg());
	} else if (Op.isImm()) {
		O << formatImm(Op.getImm());
	} else {
		assert(Op.isExpr() && "unknown operand kind in printOperand");
		O << *Op.getExpr();
	}
}

void LEGInstPrinter::printAddrModeRegImm(const MCInst *MI, unsigned OpNo, const MCSubtargetInfo &STI, raw_ostream &O) const {
	const MCOperand &Op1 = MI->getOperand(OpNo);
	const MCOperand &Op2 = MI->getOperand(OpNo + 1);

	O << "[" << getRegisterName(Op1.getReg());
	int64_t imm = Op2.getImm();
	if (imm != 0)
		O << " + " << imm;
	O << "]";
}

void LEGInstPrinter::printBranchOperand(const MCInst *MI, uint64_t Address, unsigned OpNo, const MCSubtargetInfo &STI, raw_ostream &O) const {
	const MCOperand &MO = MI->getOperand(OpNo);
	if (!MO.isImm())
		return printOperand(MI, OpNo, STI, O);

	uint64_t Target = Address + MO.getImm();
	O << formatHex(Target);
}