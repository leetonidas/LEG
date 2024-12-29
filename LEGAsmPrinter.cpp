#include "LEG.h"
#include "MCTargetDesc/LEGInstPrinter.h"

#include "TargetInfo/LEGTargetInfo.h"


#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

#define DEBUG_TYPE "leg-asm-printer"

namespace llvm {

class LEGAsmPrinter : public AsmPrinter {
public:
	LEGAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
		: AsmPrinter(TM, std::move(Streamer)) {}

	StringRef getPassName() const override { return "LEG Assembly Printer"; }

	void emitInstruction(const MachineInstr *MI) override;

	void printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &O);
};

void LEGAsmPrinter::emitInstruction(const MachineInstr *MI) {
	MCInst TmpInst;
	lowerLEGMachineInstrToMCInst(*MI, TmpInst, *this);
	EmitToStreamer(*OutStreamer, TmpInst);
}

void LEGAsmPrinter::printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &O) {
	const MachineOperand &MO = MI->getOperand(OpNo);

	switch (MO.getType()) {
	case MachineOperand::MO_Register:
		O << LEGInstPrinter::getRegisterName(MO.getReg());
		break;
	case MachineOperand::MO_Immediate:
		O << MO.getImm();
		break;
	case MachineOperand::MO_GlobalAddress:
		O << getSymbol(MO.getGlobal());
		break;
	case MachineOperand::MO_ExternalSymbol:
		O << *GetExternalSymbolSymbol(MO.getSymbolName());
		break;
	case MachineOperand::MO_MachineBasicBlock:
		O << *MO.getMBB()->getSymbol();
		break;
	default:
		llvm_unreachable("Not implemented yet!");
	}
}

}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLEGAsmPrinter() {
	llvm::RegisterAsmPrinter<llvm::LEGAsmPrinter> X(llvm::getTheLEGTarget());
}