
#include "LEG.h"
#include "LEGTargetMachine.h"
#include "MCTargetDesc/LEGMCTargetDesc.h"


#include "llvm/CodeGen/SelectionDAGISel.h"

#define DEBUG_TYPE "leg-isel"
#define PASS_NAME "LEG DAG->DAG Pattern Instruction Selection"

using namespace llvm;

namespace {
class LEGDAGToDAGISel : public SelectionDAGISel {

public:
	static char ID;

	LEGDAGToDAGISel() = delete;

	explicit LEGDAGToDAGISel(LEGTargetMachine &TargetMachine, CodeGenOpt::Level OptLevel)
		: SelectionDAGISel(ID, TargetMachine, OptLevel)
		, Subtarget(nullptr) {}

	bool runOnMachineFunction(MachineFunction &MF) override;

	// used by tblgen
	bool selectAddrRegImm(SDValue N, SDValue &Base, SDValue &Offset) const;

#include "LEGGenDAGISel.inc"

private:

	template<unsigned NodeType> void select(SDNode *N);
	void Select(SDNode *N) override;
	const LEGSubtarget *Subtarget = nullptr;
};

} // namepsace

char LEGDAGToDAGISel::ID = 0;

INITIALIZE_PASS(LEGDAGToDAGISel, DEBUG_TYPE, PASS_NAME, false, false)

bool LEGDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
	Subtarget = &MF.getSubtarget<LEGSubtarget>();
	bool res = SelectionDAGISel::runOnMachineFunction(MF);
	return res;
}

bool LEGDAGToDAGISel::selectAddrRegImm(SDValue N, SDValue &Base, SDValue &Offset) const {
	SDLoc DL(N);

	if (FrameIndexSDNode *FI = dyn_cast<FrameIndexSDNode>(N)) {
		Base = CurDAG->getTargetFrameIndex(FI->getIndex(), MVT::i64);
		Offset = CurDAG->getTargetConstant(0, SDLoc(N), MVT::i64);
		return true;
	}

	errs() << "addrRegImm\n";
	N.dump();
	return false;
}

template<>
void LEGDAGToDAGISel::select<ISD::FrameIndex>(SDNode *N) {
	SDLoc DL(N);

	int FI = cast<FrameIndexSDNode>(N)->getIndex();
	SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i64);
	SDValue Imm = CurDAG->getTargetConstant(0, DL, MVT::i64);
	ReplaceNode(N, CurDAG->getMachineNode(LEG::ADDi, DL, MVT::i64, TFI, Imm));
}

void LEGDAGToDAGISel::Select(SDNode *N) {
	if (N->isMachineOpcode()) {
		LLVM_DEBUG(errs() << "== "; N->dump(CurDAG); errs() << "\n");
		N->setNodeId(-1);
		return;
	}

	if (N->getOpcode() == ISD::FrameIndex) {
		select<ISD::FrameIndex>(N);
		return;
	}

	SelectCode(N);
}

FunctionPass *llvm::createLEGISelDag(LEGTargetMachine &TM, CodeGenOpt::Level OptLevel) {
	return new LEGDAGToDAGISel(TM, OptLevel);
}
