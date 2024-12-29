#include "LEG.h"
#include "MCTargetDesc/LEGMCTargetDesc.h"
#include "LEGSubtarget.h"
#include "LEGFrameLowering.h"

#include "llvm/CodeGen/MachineFrameInfo.h"

using namespace llvm;

bool LEGFrameLowering::canSimplifyCallFramePseudos(const MachineFunction &MF) const {
	return true;
}

void LEGFrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
	MachineFrameInfo &MFI = MF.getFrameInfo();
	const LEGSubtarget &STI = MF.getSubtarget<LEGSubtarget>();
	const LEGInstrInfo &TII = *STI.getInstrInfo();
	MachineBasicBlock::iterator MBBI = MBB.begin();
	DebugLoc DL;

	uint64_t FrameSize = MFI.getStackSize();
	Align StackAlign = getStackAlign();
	FrameSize = alignTo(FrameSize, StackAlign);
	MFI.setStackSize(FrameSize);

	if (MF.getSubtarget().getFeatureBits()[LEG::FeatureInsEncryption]) {
		BuildMI(MBB, MBBI, DL, TII.get(LEG::ADDi), LEG::SP)
			.addReg(LEG::SP, RegState::Kill)
			.addImm(0)
			.setMIFlag(MachineInstr::FrameSetup);
	}

	if (FrameSize == 0 && MFI.adjustsStack())
		return;

	BuildMI(MBB, MBBI, DL, TII.get(LEG::SUBi), LEG::SP)
		.addReg(LEG::SP, RegState::Kill)
		.addImm(FrameSize)
		.setMIFlag(MachineInstr::FrameSetup);
}

void LEGFrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
	MachineFrameInfo &MFI = MF.getFrameInfo();
	const LEGSubtarget &STI = MF.getSubtarget<LEGSubtarget>();
	const LEGInstrInfo &TII = *STI.getInstrInfo();
	MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
	DebugLoc DL = MBBI->getDebugLoc();

	uint64_t FrameSize = MFI.getStackSize();
	Align StackAlign = getStackAlign();
	FrameSize = alignTo(FrameSize, StackAlign);
	MFI.setStackSize(FrameSize);

	if (FrameSize == 0 && MFI.adjustsStack())
		return;

	BuildMI(MBB, MBBI, DL, TII.get(LEG::ADDi), LEG::SP)
		.addReg(LEG::SP, RegState::Kill)
		.addImm(FrameSize)
		.setMIFlag(MachineInstr::FrameSetup);
}

bool LEGFrameLowering::hasFP(const MachineFunction &MF) const {
	const MachineFrameInfo &MFI = MF.getFrameInfo();
	return MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken();
}

MachineBasicBlock::iterator LEGFrameLowering::eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::iterator MI) const {
	errs() << "eliminateCallFramePseudoInstr\n";
	const LEGInstrInfo &TII = *STI.getInstrInfo();
	Register SPReg = LEG::SP;
	DebugLoc DL = MI->getDebugLoc();
	if (!hasReservedCallFrame(MF)) {
		int64_t Adjust = MI->getOperand(0).getImm();

		if (Adjust != 0) {
			Adjust = alignSPAdjust(Adjust);

			auto Opcode = LEG::SUBi;
			if (MI->getOpcode() == LEG::ADJCALLSTACKDOWN)
				Opcode = LEG::ADDi;

			const LEGRegisterInfo &RI = *STI.getRegisterInfo();
			BuildMI(MBB, MI, DL, TII.get(Opcode), LEG::SP)
				.addReg(LEG::SP, RegState::Kill)
				.addImm(Adjust);
		}
	}

	return MBB.erase(MI);
}

/*
StackOffset LEGFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI, Register &FrameReg) const {
	const MachineFrameInfo &MFI = MF.getFrameInfo();
	const TargetRegisterInfo *RI = MF.getSubtarget().getRegisterInfo();

	llvm_unreachable("getFrameIndexReference");
}
*/
