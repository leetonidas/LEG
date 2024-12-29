#pragma once

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "LEGGenRegisterInfo.inc"

namespace llvm {

struct LEGRegisterInfo : public LEGGenRegisterInfo {
public:
	LEGRegisterInfo(unsigned HwMode);


	const uint32_t *getCallPreservedMask(const MachineFunction &MF, CallingConv::ID) const override;
	const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

	BitVector getReservedRegs(const MachineFunction &MF) const override;

//	bool hasReservedSpillSlot(const MachineFunction &MF, Register Reg, int &FrameIdx) const override;

	bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj, unsigned FIOperandNum, RegScavenger *RS = nullptr) const override;

//	void resolveFrameIndex(MachineInstr &MI, Register BaseReg, int64_t Offset) const override;

	Register getFrameRegister(const MachineFunction &MF) const override;

};
}