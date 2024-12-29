#pragma once

#include "LEG.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/TargetParser/RISCVTargetParser.h"
#include <optional>

namespace llvm {

namespace LEGISD {
enum NodeType {
	FIRST_NUMBER = ISD::BUILTIN_OP_END,
	RET_FLAG,
	CALL,
	WRAPPER,
};
}

class LEGSubtarget;

class LEGTargetLowering : public TargetLowering {
	const LEGSubtarget &Subtarget;

public:
	explicit LEGTargetLowering(const TargetMachine &TM, const LEGSubtarget &STI);

	const LEGSubtarget &getSubtarget() const { return Subtarget; }

	SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

	SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;

	SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

	SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID C, bool IsVarArg, const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL, SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const override;

	SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::OutputArg> &Outs, const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL, SelectionDAG &DAG) const override;

	SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI, SmallVectorImpl<SDValue> &InVals) const override;
};
}