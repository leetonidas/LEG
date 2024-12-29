#pragma once

#include "LEGSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {
class LEGMachineFunctionInfo : public MachineFunctionInfo {
private:

public:
	LEGMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}

};
}