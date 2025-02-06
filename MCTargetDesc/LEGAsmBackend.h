#pragma once

#include "MCTargetDesc/LEGFixupKinds.h"

#include "llvm/ADT/bit.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"

namespace llvm {
class MCAssembler;
class MCObjectTargetWriter;
class raw_ostream;

class LEGAsmBackend : public MCAsmBackend {
	const MCSubtargetInfo &STI;
	uint8_t OSABI;
	const MCTargetOptions &TargetOptions;
public:
	LEGAsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI, const MCTargetOptions &Options)
		: MCAsmBackend(llvm::endianness::little), STI(STI), OSABI(OSABI), TargetOptions(Options) {}
	~LEGAsmBackend() override = default;

	std::unique_ptr<MCObjectTargetWriter>
	createObjectTargetWriter() const override;

	void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup, const MCValue &Target, MutableArrayRef<char> Data,
		            uint64_t Value, bool IsResolved, const MCSubtargetInfo *STI) const override;

	std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;
	const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

	unsigned getNumFixupKinds() const override {
		return LEG::NumTargetFixupKinds;
	}

	bool writeNopData(raw_ostream &OS, uint64_t Count, const MCSubtargetInfo *STI) const override;

	bool shouldForceRelocation(const MCAssembler &Asm, const MCFixup &Fixup, const MCValue &Target, const MCSubtargetInfo *STI) override;
};
}