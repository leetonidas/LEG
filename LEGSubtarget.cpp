#include "LEGSubtarget.h"

#include "llvm/BinaryFormat/ELF.h"

#include "LEG.h"
#include "LEGTargetMachine.h"
#include "MCTargetDesc/LEGMCTargetDesc.h"

#define DEBUG_TYPE "leg-subtraget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "LEGGenSubtargetInfo.inc"


namespace llvm {
LEGSubtarget::LEGSubtarget(const Triple &TT, StringRef CPU, StringRef FS, const LEGTargetMachine &TM)
	: LEGGenSubtargetInfo(TT, CPU, CPU, FS)
	, FrameLowering(initializeSubtargetDependencies(CPU, FS, TM))
	, RegInfo(getHwMode())
	, TLInfo(TM, *this) {
	ParseSubtargetFeatures(CPU, CPU, FS);
}

LEGSubtarget &LEGSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS, const TargetMachine &TM) {
	ParseSubtargetFeatures(CPU, CPU, FS);
	return *this;
}
}