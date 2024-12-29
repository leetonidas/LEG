#include "LEGISelLowering.h"
#include "LEG.h"
#include "LEGMachineFunctionInfo.h"
#include "LEGRegisterInfo.h"
#include "LEGSubtarget.h"
#include "LEGTargetMachine.h"
#include "MCTargetDesc/LEGMCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"

using namespace llvm;

#define DEBUG_TYPE "leg-lower"

LEGTargetLowering::LEGTargetLowering(const TargetMachine &TM, const LEGSubtarget &STI)
	: TargetLowering(TM), Subtarget(STI) {

	addRegisterClass(MVT::i64, &LEG::GPRRegClass);
	computeRegisterProperties(STI.getRegisterInfo());

	setStackPointerRegisterToSaveRestore(LEG::SP);
	setSupportsUnalignedAtomics(false);

	setLoadExtAction({ISD::EXTLOAD, ISD::SEXTLOAD, ISD::ZEXTLOAD}, MVT::i64,  MVT::i1, Promote);
	setLoadExtAction({ISD::SEXTLOAD, ISD::ZEXTLOAD}, MVT::i64, MVT::i8, Expand);
	setLoadExtAction({ISD::SEXTLOAD, ISD::ZEXTLOAD}, MVT::i64, MVT::i16, Expand);
	setLoadExtAction({ISD::SEXTLOAD, ISD::ZEXTLOAD}, MVT::i64, MVT::i32, Expand);

	// this seams to not work without in-register-truncation
	// using this instead of the truncstore pattern will crash in LegalizeStoreOps in LegalizeDAG.cpp
	// in case TargetLowering::Expand - Result will be the same as SDValue(Node, 0) and thus ReplaceNode asserts
	//setTruncStoreAction(MVT::i64, MVT::i32, Expand);
	//setTruncStoreAction(MVT::i64, MVT::i16, Expand);
	//setTruncStoreAction(MVT::i64, MVT::i8, Expand);
	//setTruncStoreAction(MVT::i64, MVT::i1, Expand);

	// sign extend is emulated via shl, sar
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);

	setOperationAction(ISD::BR_CC, MVT::i64, Expand);
	setOperationAction(ISD::SELECT_CC, MVT::i64, Expand);

	setCondCodeAction(ISD::SETGE, MVT::i64, Expand);
	setCondCodeAction(ISD::SETGT, MVT::i64, Expand);
	setCondCodeAction(ISD::SETUGE, MVT::i64, Expand);
	setCondCodeAction(ISD::SETUGT, MVT::i64, Expand);

	setOperationAction({ISD::GlobalAddress, ISD::BlockAddress, ISD::ConstantPool, ISD::JumpTable}, MVT::i64, Custom);
//	setOperationAction({ISD::STACKSAVE, ISD::STACKRESTORE}, MVT::Other, Expand);
	errs() << "LEGTargetLowering constructor done\n";
}

#include "LEGGenCallingConv.inc"

SDValue LEGTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI, SmallVectorImpl<SDValue> &InVals) const {
	SelectionDAG &DAG = CLI.DAG;
	SDLoc &DL = CLI.DL;
	SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
	SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
	SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
	SDValue Chain = CLI.Chain;
	SDValue Callee = CLI.Callee;
	bool &isTailCall = CLI.IsTailCall;
	CallingConv::ID CallConv = CLI.CallConv;
	EVT PtrVT = getPointerTy(DAG.getDataLayout());
	bool isVarArg = CLI.IsVarArg;

	MachineFunction &MF = DAG.getMachineFunction();

	isTailCall = false;

	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());

	CCInfo.AnalyzeCallOperands(Outs, CC_LEG);

	unsigned NumBytes = CCInfo.getNextStackOffset();

	Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);
	SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
	SmallVector<SDValue, 8> MemOpChains;

	unsigned AI, AE;
	SDValue StackPtr;
	for (AI = 0, AE = ArgLocs.size(); AI != AE; ++AI) {
		CCValAssign &VA = ArgLocs[AI];
		EVT RegVT = VA.getLocVT();
		SDValue Arg = OutVals[AI];

		assert(VA.getLocInfo() == CCValAssign::Full && "value promotion necessary");

		if (VA.isRegLoc()) {
			RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
		} else {
			assert(VA.isMemLoc() && "Argument not register or memory");

			if (!StackPtr.getNode())
				StackPtr = DAG.getCopyFromReg(Chain, DL, LEG::SP, PtrVT);
			SDValue Address = DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr, DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));

			MemOpChains.push_back(DAG.getStore(Chain, DL, Arg, Address, MachinePointerInfo::getStack(MF, VA.getLocMemOffset())));
		}
	}

	if (!MemOpChains.empty())
		Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

	SDValue Glue;

	for (auto &Reg : RegsToPass) {
		Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
		Glue = Chain.getValue(1);
	}

	// maybe we don't need the GlobalAddress -> TargetGlobalAddress
	// thing from RISCV

	if (GlobalAddressSDNode *S = dyn_cast<GlobalAddressSDNode>(Callee)) {
		const GlobalValue *GV = S->getGlobal();
		Callee = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0, 0);
	} else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
		Callee = DAG.getTargetExternalSymbol(S->getSymbol(), PtrVT, 0);
	}

	SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
	SmallVector<SDValue, 8> Ops;
	Ops.push_back(Chain);
	Ops.push_back(Callee);

	// also push the number of arguments
	Ops.push_back(DAG.getConstant(ArgLocs.size(), DL, MVT::i64));

	for (auto &Reg : RegsToPass)
		Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

	const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
	const uint32_t *Mask = TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);
	assert(Mask && "Missing call preserved mask");
	Ops.push_back(DAG.getRegisterMask(Mask));

	if (Glue.getNode())
		Ops.push_back(Glue);

	Chain = DAG.getNode(LEGISD::CALL, DL, NodeTys, Ops);
	DAG.addNoMergeSiteInfo(Chain.getNode(), CLI.NoMerge);
	Glue = Chain.getValue(1);

	Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
	if (!Ins.empty())
		Glue = Chain.getValue(1);


	SmallVector<CCValAssign, 16> RVLocs;
	CCState RetCCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs, *DAG.getContext());
	RetCCInfo.AnalyzeCallResult(Ins, RetCC_LEG);

	for (CCValAssign const &RVLoc : RVLocs) {
		SDValue RetValue = DAG.getCopyFromReg(Chain, DL, RVLoc.getLocReg(), RVLoc.getLocVT(), Glue);
		Chain = RetValue.getValue(1);
		Glue = Chain.getValue(2);

		InVals.push_back(Chain.getValue(0));
	}

	return Chain;
}

SDValue LEGTargetLowering::LowerFormalArguments(SDValue Chain, CallingConv::ID C, bool IsVarArg
		, const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DLoc
		, SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo &MFI = MF.getFrameInfo();
	auto DL = DAG.getDataLayout();

	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(C, IsVarArg, MF, ArgLocs, *DAG.getContext());
	CCInfo.AnalyzeFormalArguments(Ins, CC_LEG);

	SDValue ArgValue;

	for (CCValAssign &VA : ArgLocs) {

		if (VA.isRegLoc()) {
			EVT RegVT = VA.getLocVT();
			assert(RegVT == MVT::i64);

			Register Reg = MF.addLiveIn(VA.getLocReg(), &LEG::GPRRegClass);
			ArgValue = DAG.getCopyFromReg(Chain, DLoc, Reg, RegVT);

			// TODO check VA.getLocInfo()
			InVals.push_back(ArgValue);
		} else {
			assert(VA.isMemLoc());
			EVT LocVT = VA.getLocVT();

			errs() << "lowering stack arg\n";

			int FI = MFI.CreateFixedObject(LocVT.getSizeInBits() / 8, VA.getLocMemOffset(), true);
			SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DL));
			InVals.push_back(DAG.getLoad(LocVT, DLoc, Chain, FIN, MachinePointerInfo::getFixedStack(MF, FI)));
		}
	}

	return Chain;
}

SDValue LEGTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg
		, const SmallVectorImpl<ISD::OutputArg> &Outs, const SmallVectorImpl<SDValue> &OutVals
		, const SDLoc &DL, SelectionDAG &DAG) const {
	MachineFunction &MF = DAG.getMachineFunction();

	SmallVector<CCValAssign, 16> RVLocs;

	CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

	CCInfo.AnalyzeReturn(Outs, RetCC_LEG);

	SDValue Glue;
	SmallVector<SDValue, 4> RetOps(1, Chain);
	for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
		CCValAssign &VA = RVLocs[i];
		assert(VA.isRegLoc() && "Can only return in registers!");
		Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Glue);
		Glue = Chain.getValue(1);
		RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
	}

	RetOps[0] = Chain;
	if (Glue.getNode()) {
		RetOps.push_back(Glue);
	}

	auto ret = DAG.getNode(LEGISD::RET_FLAG, DL, MVT::Other, RetOps);
	return ret;
}


SDValue LEGTargetLowering::LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
	auto DL = DAG.getDataLayout();

	const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
	int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

	SDValue Result = DAG.getTargetGlobalAddress(GV, SDLoc(Op), getPointerTy(DL), Offset);
	return DAG.getNode(LEGISD::WRAPPER, SDLoc(Op), getPointerTy(DL), Result);
}

SDValue LEGTargetLowering::LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const {
	auto DL = DAG.getDataLayout();

	const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();

	SDValue Result = DAG.getTargetBlockAddress(BA, getPointerTy(DL));
	return DAG.getNode(LEGISD::WRAPPER, SDLoc(Op), getPointerTy(DL), Result);
}


SDValue LEGTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
	switch (Op.getOpcode()) {
	default:
		llvm_unreachable("Unable to custom lower SDValue");
	case ISD::GlobalAddress:
		return LowerGlobalAddress(Op, DAG);
	case ISD::BlockAddress:
		return LowerBlockAddress(Op, DAG);
	}

	return SDValue();
}