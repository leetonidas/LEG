#pragma once

#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Target/TargetMachine.h"

#include "LEGRegisterInfo.h"
#include "LEGFrameLowering.h"
#include "LEGISelLowering.h"
#include "LEGInstrInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "LEGGenSubtargetInfo.inc"
#undef GET_SUBTARGETINFO_HEADER

namespace llvm {
class LEGSubtarget : public LEGGenSubtargetInfo {
public:
	LEGSubtarget(const Triple &TT, StringRef CPU, StringRef FS, const LEGTargetMachine &TM);

	void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

	bool hasInsEncryption() const { return m_hasInsEncryption; }

	// will SEGFAULT in SelectCodeCommon -> mayRaiseFPException if not defined
	const LEGInstrInfo *getInstrInfo() const override { return &InstrInfo; }
	const LEGFrameLowering *getFrameLowering() const override { return &FrameLowering; }
	const LEGTargetLowering *getTargetLowering() const override { return &TLInfo; }
	const LEGRegisterInfo *getRegisterInfo() const override { return &RegInfo; }


	LEGSubtarget &initializeSubtargetDependencies(StringRef CPU, StringRef TuneCPU, const TargetMachine &TM);

private:
	bool m_isBigEndian = false;
	bool m_hasInsEncryption = false;

	LEGFrameLowering FrameLowering;
	LEGInstrInfo InstrInfo;
	LEGRegisterInfo RegInfo;
	LEGTargetLowering TLInfo;
};
}