#include "LEG.h"
#include "LEGInstrInfo.h"
#include "LEGTargetMachine.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define LEG_EXPAND_PSEUDO_NAME "LEG pseudo instruction expansion pass"

namespace {
class LEGExpandPseudo : public MachineFunctionPass {
public:
	const LEGInstrInfo *TII;
	static char ID;

	LEGExpandPseudo() : MachineFunctionPass(ID) {
		initializeLEGExpandPseudoPass(*PassRegistry::getPassRegistry());
	}

	bool runOnMachineFunction(MachineFunction &MF) override;

	StringRef getPassName() const override { return LEG_EXPAND_PSEUDO_NAME; }

private:
	bool expandMBB(MachineBasicBlock &MBB);
	bool expandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI);
};

char LEGExpandPseudo::ID = 0;

bool LEGExpandPseudo::runOnMachineFunction(MachineFunction &MF) {
	errs() << "expanding pseudo insts\n";
	TII = static_cast<const LEGInstrInfo *>(MF.getSubtarget().getInstrInfo());
	bool changed = false;
	for (MachineBasicBlock &MBB : MF)
		changed |= expandMBB(MBB);
	return changed;
}

bool LEGExpandPseudo::expandMBB(MachineBasicBlock &MBB) {
	bool changed = false;

	for (MachineBasicBlock::iterator MBBI = MBB.begin(); MBBI != MBB.end(); MBBI = std::next(MBBI))
		changed |= expandMI(MBB, MBBI);

	return changed;
}

bool LEGExpandPseudo::expandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) {
	// TODO replace pseudo instructions
	return false;
}
}

INITIALIZE_PASS(LEGExpandPseudo, "leg-expand-pseudo", LEG_EXPAND_PSEUDO_NAME, false, false)

namespace llvm {
	FunctionPass *createLEGExpandPseudoPass() { return new LEGExpandPseudo(); }
}