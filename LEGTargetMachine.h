#pragma once

#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#include "LEGSubtarget.h"

namespace llvm {

class LEGTargetMachine : public LLVMTargetMachine {
public:
	LEGTargetMachine(const Target &T, const Triple &TT, StringRef CPU, StringRef FS, const TargetOptions &Options, std::optional<Reloc::Model> RM, std::optional<CodeModel::Model> CM, CodeGenOptLevel OptLevel, bool JIT);

	const LEGSubtarget *getSubtargetImpl(const Function &F) const override;
	const LEGSubtarget *getSubtargetImpl() const = delete;

	TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

	TargetLoweringObjectFile *getObjFileLowering() const override {
		return TLOF.get();
	}

private:
	std::unique_ptr<TargetLoweringObjectFile> TLOF;
	LEGSubtarget SubTarget;
};
}