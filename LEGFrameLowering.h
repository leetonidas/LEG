#pragma once

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class LEGSubtarget;
class LEGFrameLowering : public TargetFrameLowering {
public:
	explicit LEGFrameLowering(const LEGSubtarget &STI)
		: TargetFrameLowering(StackGrowsDown, Align(16), 0, Align(16))
		, STI(STI) {}
	void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
	void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

	//StackOffset getFrameIndexReference(const MachineFunction &MF, int FI, Register &FrameReg) const override;

	bool hasFP(const MachineFunction &MF) const override;

	MachineBasicBlock::iterator eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::iterator MI) const override;

	// use this function to force eliminateCallFramePseudoInstr to be called
	// else it depends on "needsFrameIndexResolution" which by default depends on
	// MF.getFrameInfo().hasStackObjects()
	bool canSimplifyCallFramePseudos(const MachineFunction &MF) const override;

protected:
	const LEGSubtarget &STI;
};
}
