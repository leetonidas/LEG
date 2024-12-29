#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCInst.h"

#include "LEG.h"
#include "LEGInstrInfo.h"

namespace llvm {
static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym, const AsmPrinter &AP) {
	MCContext &Ctx = AP.OutContext;
	const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

	assert(MO.getTargetFlags() == 0 && "unexpected target flag");

	return MCOperand::createExpr(Expr);
}

bool lowerLEGMachineOperandToMCOperand(const MachineOperand &MO, MCOperand &MCOp, const AsmPrinter &AP) {
	switch (MO.getType()) {
	default:
		MO.dump();
		report_fatal_error("LEG lowerOperand: unknown operand type");
	case MachineOperand::MO_Register:
		if (MO.isImplicit())
			return false;
		MCOp = MCOperand::createReg(MO.getReg());
		break;
	case MachineOperand::MO_RegisterMask:
		return false;
	case MachineOperand::MO_Immediate:
		MCOp = MCOperand::createImm(MO.getImm());
		break;
	case MachineOperand::MO_MachineBasicBlock:
		MCOp = lowerSymbolOperand(MO, MO.getMBB()->getSymbol(), AP);
		break;
	case MachineOperand::MO_GlobalAddress:
		MCOp = lowerSymbolOperand(MO, AP.getSymbolPreferLocal(*MO.getGlobal()), AP);
		break;
	case MachineOperand::MO_BlockAddress:
		MCOp = lowerSymbolOperand(MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP);
		break;
	case MachineOperand::MO_ExternalSymbol:
		MCOp = lowerSymbolOperand(MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);
		break;
	case MachineOperand::MO_JumpTableIndex:
		MCOp = lowerSymbolOperand(MO, AP.GetJTISymbol(MO.getIndex()), AP);
		break;
	case MachineOperand::MO_MCSymbol:
		MCOp = lowerSymbolOperand(MO, MO.getMCSymbol(), AP);
		break;
	}
	return true;
}

void lowerLEGMachineInstrToMCInst(const MachineInstr &MI, MCInst &OutMI, const AsmPrinter &AP) {
	OutMI.setOpcode(MI.getOpcode());

	for (const MachineOperand &MO : MI.operands()) {
		MCOperand MCOp;
		if (lowerLEGMachineOperandToMCOperand(MO, MCOp, AP))
			OutMI.addOperand(MCOp);
	}
}
}