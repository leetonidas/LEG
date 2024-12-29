#include "TargetInfo/LEGTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

namespace llvm {

Target &getTheLEGTarget() {
  static Target TheLEGTarget;
  return TheLEGTarget;
}

} // namespace llvm

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLEGTargetInfo() {
  llvm::RegisterTarget<llvm::Triple::leg> X(llvm::getTheLEGTarget(), "leg",
                                            "Imaginary LEG Architecture", "LEG");
}