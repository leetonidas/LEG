#include "MCTargetDesc/LEGFixupKinds.h"
#include "llvm/MC/MCELFObjectWriter.h"

namespace llvm {

class LEGELFObjectWriter : public MCELFObjectTargetWriter {
public:
	LEGELFObjectWriter(uint8_t OSABI);
	virtual ~LEGELFObjectWriter() = default;
	unsigned getRelocType(MCContext &Ctx, const MCValue &Target, const MCFixup &Fixup, bool IsPCRel) const override;
};

LEGELFObjectWriter::LEGELFObjectWriter(uint8_t OSABI)
	: MCELFObjectTargetWriter(/*elf64=*/true, OSABI, ELF::EM_LEG, true) {}

unsigned LEGELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target, const MCFixup &Fixup, bool IsPCRel) const {
	const unsigned Kind = Fixup.getTargetKind();
	if (Kind >= FirstLiteralRelocationKind)
		return Kind - FirstLiteralRelocationKind;
	switch (Kind) {
	case LEG::fixup_leg_imm64:
		return ELF::R_LEG_IMM64;
	case LEG::fixup_leg_pcrel_jmp22:
		return ELF::R_LEG_PCREL_BR22;
	case LEG::fixup_leg_pcrel_jmp27:
		return ELF::R_LEG_PCREL_BR27;
	}
	llvm_unreachable("invalid fixup kind!");
}

std::unique_ptr<MCObjectTargetWriter> createLEGELFObjectWriter(uint8_t OSABI) {
	return std::make_unique<LEGELFObjectWriter>(OSABI);
}

}