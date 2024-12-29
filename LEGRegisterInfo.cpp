#include "LEGRegisterInfo.h"
#include "LEG.h"
#include "LEGSubtarget.h"
#include "MCTargetDesc/LEGMCTargetDesc.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"


#define GET_REGINFO_TARGET_DESC
#include "LEGGenRegisterInfo.inc"

using namespace llvm;

LEGRegisterInfo::LEGRegisterInfo(unsigned HwMode)
	: LEGGenRegisterInfo(0, 0, 0, 0, HwMode) {}

const uint32_t *LEGRegisterInfo::getCallPreservedMask(const MachineFunction &MF, CallingConv::ID) const {
	return CSR_LEG_RegMask;
}

const MCPhysReg *LEGRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
	return CSR_LEG_SaveList;
}

BitVector LEGRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
	const LEGFrameLowering *TFI = getFrameLowering(MF);
	BitVector Reserved(getNumRegs());
	if (TFI->hasFP(MF))
		markSuperRegs(Reserved, LEG::BP);
	markSuperRegs(Reserved, LEG::SP);
	markSuperRegs(Reserved, LEG::PC);

	return Reserved;
}

//bool LEGRegisterInfo::hasReservedSpillSlot(const MachineFunction &MF, Register Reg, int &FrameIdx) const {}

bool LEGRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj, unsigned int FIOperandNum, RegScavenger *RS) const {
	assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");
	errs() << "eliminateFrameIndex\n";
	II->dump();

	MachineInstr &MI = *II;
	MachineFunction &MF = *MI.getParent()->getParent();
	MachineRegisterInfo &MRI = MF.getRegInfo();
	const LEGSubtarget &ST = MF.getSubtarget<LEGSubtarget>();
	DebugLoc DL = MI.getDebugLoc();

	int FrameIdx = MI.getOperand(FIOperandNum).getIndex();
	Register FrameReg;
	StackOffset Offset = getFrameLowering(MF)->getFrameIndexReference(MF, FrameIdx, FrameReg);
	Offset += StackOffset::getFixed(MI.getOperand(FIOperandNum + 1).getImm());

	// TODO optimize like in RISCV if MI is ADDi / SUBi
	MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset.getFixed());
	MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false, false, false);

	if ((MI.getOpcode() == LEG::ADDi || MI.getOpcode() == LEG::SUBi) &&
		MI.getOperand(0).getReg() == MI.getOperand(1).getReg() &&
		MI.getOperand(2).getImm() == 0) {
		MI.eraseFromParent();
		return true;
	}

	return false;
}

Register LEGRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
	return getFrameLowering(MF)->hasFP(MF) ? LEG::BP : LEG::SP;
}
