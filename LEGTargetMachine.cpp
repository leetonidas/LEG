#include "LEGTargetMachine.h"
#include "LEG.h"
#include "TargetInfo/LEGTargetInfo.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassRegistry.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"

using namespace llvm;

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                           std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

LEGTargetMachine::LEGTargetMachine(const Target &T, const Triple &TT, StringRef CPU, StringRef FS, const TargetOptions &Options,
	                               std::optional<Reloc::Model> RM, std::optional<CodeModel::Model> CM, CodeGenOptLevel Ol, bool JIT)
	: LLVMTargetMachine(T, "e-m:e-p:64:64-i64:64-i128:128-n64-S128", TT, CPU, FS, Options, getEffectiveRelocModel(TT, RM),
		                getEffectiveCodeModel(CM, CodeModel::Small), Ol)
	, TLOF(std::make_unique<TargetLoweringObjectFileELF>())
	, SubTarget(TT, "generic", FS, *this) {
	initAsmInfo();
}

const LEGSubtarget *LEGTargetMachine::getSubtargetImpl(const Function &F) const {
	return &SubTarget;
}

namespace {
class LEGPassConfig : public TargetPassConfig {
public:
	LEGPassConfig(LEGTargetMachine &TM, PassManagerBase &PM)
		: TargetPassConfig(TM, PM) {}

	LEGTargetMachine &getLEGTargetMachine() const {
		return getTM<LEGTargetMachine>();
	}

	bool addInstSelector() override;
	void addPreEmitPass2() override;
};
}

TargetPassConfig *LEGTargetMachine::createPassConfig(PassManagerBase &PM) {
	return new LEGPassConfig(*this, PM);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLEGTarget() {
	RegisterTargetMachine<LEGTargetMachine> X(getTheLEGTarget());
	auto *PR = PassRegistry::getPassRegistry();
	initializeLEGExpandPseudoPass(*PR);
}

bool LEGPassConfig::addInstSelector() {
	addPass(createLEGISelDag(getLEGTargetMachine(), getOptLevel()));
	return false;
}

void LEGPassConfig::addPreEmitPass2() {
	addPass(createLEGExpandPseudoPass());
}
