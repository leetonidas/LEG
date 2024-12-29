
#include "LEGInstrInfo.h"
#include "LEG.h"
#include "MCTargetDesc/LEGMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

#include "llvm/CodeGen/MachineInstrBuilder.h"

#include "LEGRegisterInfo.h"
#include "LEGTargetMachine.h"

#define GET_INSTRMAP_INFO
#define GET_INSTRINFO_CTOR_DTOR
#include "LEGGenInstrInfo.inc"
#undef GET_INSTRMAP_INFO

using namespace llvm;

LEGInstrInfo::LEGInstrInfo()
	// The parameters are the CallframeSetupOpcode and CallframeDestroyOpcode
	: LEGGenInstrInfo(LEG::ADJCALLSTACKDOWN, LEG::ADJCALLSTACKUP) {}

void LEGInstrInfo::copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, const DebugLoc &DL, MCRegister DstReg, MCRegister SrcReg, bool KillSrc) const {
	if (LEG::GPRnopcRegClass.contains(DstReg, SrcReg)) {
		BuildMI(MBB, MBBI, DL, get(LEG::ADDi), DstReg)
			.addReg(SrcReg, getKillRegState(KillSrc))
			.addImm(0);
		return;
		// copy 64 bit register
	} /*else if (LEG::GPR32nopcRegClass.contains(DstReg, SrcReg)) {
		// copy 32 bit register
	} else if (LEG::GPR16nopcRegClass.contains(DstReg, SrcReg)) {
		// copy 16 bit register
	} else if (LEG::GPR8nopcRegClass.contains(DstReg, SrcReg)) {
		// copy 8 bit register
	} */else {
		llvm_unreachable("Impossible reg-to-reg copy");
	}
}

void LEGInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register SrcReg, bool IsKill, int FI, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI, Register VReg) const {
	MachineFunction &MF = *MBB.getParent();
	DebugLoc DL;
	if (I != MBB.end())
		DL = I->getDebugLoc();

	const MachineFrameInfo &MFI = MF.getFrameInfo();
	MachineMemOperand *MMO = MF.getMachineMemOperand(
		MachinePointerInfo::getFixedStack(MF, FI),
		MachineMemOperand::MOStore, MFI.getObjectSize(FI),
		MFI.getObjectAlign(FI));

	if (TRI->isTypeLegalForClass(*RC, MVT::i64)) {
		BuildMI(MBB, I, DL, get(LEG::STri))
			.addReg(SrcReg, getKillRegState(IsKill))
			.addFrameIndex(FI)
			.addImm(0)
			.addMemOperand(MMO);
	} else {
		errs() << RC->getID() << " not i64\n";
	}
}

void LEGInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register DestReg, int FI, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI, Register VReg) const {
	MachineFunction &MF = *MBB.getParent();
	DebugLoc DL;
	if (I != MBB.end())
		DL = I->getDebugLoc();

	const MachineFrameInfo &MFI = MF.getFrameInfo();
	MachineMemOperand *MMO = MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOLoad, MFI.getObjectSize(FI), MFI.getObjectAlign(FI));

	BuildMI(MBB, I, DL, get(LEG::LDri), DestReg)
		.addFrameIndex(FI)
		.addImm(0)
		.addMemOperand(MMO);
}


/*
pitfalls when analyzing branches:
  - make sure your instructions are marked correctly
 	- isBranch
 	- isTerminator
 	- isBarrier (only on unconditional branches)
Error diagnosis:
  - The linker complains about an unresolvable temporary symbol:
    - you did not correctly set TBB / FBB or accidentily returned false instead of true
      - returning false without setting TBB / FBB tells the optimizer that this bb will fallthrough into the next one
        and if it actually ends in a branch (that was not marked as terminator) it may mark the actual target block
        as unreachable and remove it. The branch will no longer find its target block when emitted.
*/
bool LEGInstrInfo::analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB, MachineBasicBlock *&FBB, SmallVectorImpl<MachineOperand> &Cond, bool AllowModify) const {
	TBB = FBB = nullptr;
	Cond.clear();

	//errs() << "analyzing branches in ";
	//MBB.printName(errs());
	//errs() << "\n";

	MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
	if (I == MBB.end() || !I->isTerminator()) {
		//if (I != MBB.end())
		//	I->dump();
		//errs() << "block does not end in terminator!\n";
		return false;
	}

	unsigned NumTerminators = 0;
	MachineBasicBlock::iterator FirstUnconditional = MBB.end();
	for (auto J = I.getReverse(); J != MBB.rend() && J->getDesc().isTerminator(); ++J) {
		++NumTerminators;
		if (J->getDesc().isUnconditionalBranch() || J->getDesc().isIndirectBranch()) {
			FirstUnconditional = J.getReverse();
		}
	}

	if (NumTerminators > 2) {
		//errs() << "too many terminators\n";
		return true;
	}

	if (NumTerminators == 1 && I->getDesc().isUnconditionalBranch()) {
		TBB = I->getOperand(I->getNumOperands() - 1).getMBB();

		//MBB.printName(errs());
		//errs() << " unconditional branches to ";
		//TBB->printName(errs());
		//errs() << "\n";
		//I->dump();

		return false;
	}

	if (NumTerminators == 1 && I->getDesc().isConditionalBranch()) {
		Cond.push_back(I->getOperand(0));
		TBB = I->getOperand(I->getNumOperands() - 1).getMBB();

		//MBB.printName(errs());
		//errs() << " conditional branches to ";
		//TBB->printName(errs());
		//errs() << "\n";
		//I->dump();

		return false;
	}

	if (NumTerminators == 2 && std::prev(I)->getDesc().isConditionalBranch() && I->getDesc().isUnconditionalBranch()) {
		Cond.push_back(std::prev(I)->getOperand(0));
		TBB = std::prev(I)->getOperand(std::prev(I)->getNumOperands() - 1).getMBB();
		FBB = I->getOperand(I->getNumOperands() - 1).getMBB();

		//MBB.printName(errs());
		//errs() << " branches to ";
		//TBB->printName(errs());
		//errs() << " or ";
		//FBB->printName(errs());
		//errs() << "\n";
		//std::prev(I)->dump();
		//I->dump();

		return false;
	}

	//errs() << "did not recognize terminators\n";

	return true;
}

unsigned LEGInstrInfo::removeBranch(MachineBasicBlock &MBB, int *BytesRemoved) const {
	if (BytesRemoved)
		*BytesRemoved = 0;
	MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();

	if (BytesRemoved)
		*BytesRemoved += get(I->getOpcode()).getSize();

	if (I == MBB.end())
		return 0;

	// this check is important. LLVM may ask to remove branches of blocks not ending in branches!
	if (!I->getDesc().isConditionalBranch() && !I->getDesc().isUnconditionalBranch())
		return 0;

	I->eraseFromParent();

	I = MBB.end();
	if (I == MBB.begin())
		return 1;
	--I;
	if (!I->getDesc().isConditionalBranch())
		return 1;

	if (BytesRemoved)
		*BytesRemoved += get(I->getOpcode()).getSize();
	I->eraseFromParent();
	return 2;
}

unsigned LEGInstrInfo::insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
	if (BytesAdded)
		*BytesAdded = 0;

	assert(TBB && "insertBranch called with fallthrough");
	assert((Cond.size() == 1 || Cond.size() == 0) && "invalid branch condition");
	llvm::LEG::PredSense sense = MBB.getParent()->getSubtarget().getFeatureBits()[LEG::FeatureInsEncryption] ? llvm::LEG::PredSense_true : llvm::LEG::PredSense_false;
	uint32_t uncondopcode = getPredOpcode(LEG::BR, sense);
	if (Cond.empty()) {
		MachineInstr &MI = *BuildMI(&MBB, DL, get(uncondopcode)).addMBB(TBB);
		if (BytesAdded)
			*BytesAdded += get(uncondopcode).getSize();
		return 1;
	}

	uint32_t condopcode = getPredOpcode(LEG::BRCC, sense);
	MachineInstr &CondMI = *BuildMI(&MBB, DL, get(condopcode)).add(Cond[0]).addMBB(TBB);
	if (BytesAdded)
		*BytesAdded += get(condopcode).getSize();

	if (!FBB)
		return 1;

	MachineInstr &MI = *BuildMI(&MBB, DL, get(uncondopcode)).addMBB(FBB);
	if (BytesAdded)
		*BytesAdded += get(uncondopcode).getSize();
	return 2;
}
