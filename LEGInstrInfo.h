#pragma once

#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/DiagnosticInfo.h"

#define GET_INSTRINFO_HEADER
#define GET_INSTRINFO_OPERAND_ENUM
#include "LEGGenInstrInfo.inc"

namespace llvm {

class LEGSubtarget;

// enum for condition code if neccessary

class LEGInstrInfo : public LEGGenInstrInfo {
public:
	explicit LEGInstrInfo();

	void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, const DebugLoc &DL, MCRegister DstReg, MCRegister SrcReg, bool KillSrc) const override;

	void storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register SrcReg, bool IsKill, int FI, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI, Register VReg) const override;

	void loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register DestReg, int FI, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI, Register VReg) const override;

	// branch optimization (e.g. getting rid of 'br' that could be fallthroughs)
	bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB, MachineBasicBlock *&FBB, SmallVectorImpl<MachineOperand> &Cond, bool AllowModify) const override;
	unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond, const DebugLoc &dl, int *BytesAdded = nullptr) const override;
	unsigned removeBranch(MachineBasicBlock &MBB, int *BytesRemoved = nullptr) const override;
};

}