#include "MCTargetDesc/LEGAsmBackend.h"
#include "MCTargetDesc/LEGFixupKinds.h"
#include "MCTargetDesc/LEGMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringSwitch.h"

using namespace llvm;

std::optional<MCFixupKind> LEGAsmBackend::getFixupKind(StringRef Name) const {
	if (!STI.getTargetTriple().isOSBinFormatELF())
		return std::nullopt;
	unsigned Type = llvm::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X,Y) .Case(#X, Y)
#include "llvm/BinaryFormat/ELFRelocs/LEG.def"
#undef ELF_RELOC
		;
	if (Type != -1u)
		return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
	return std::nullopt;
}

const MCFixupKindInfo &LEGAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
	// Table must be in the same order as in LEGFixupKinds
	const static MCFixupKindInfo Infos[] = {
		{"fixup_leg_imm64", 32, 64, 0},
		{"fixup_leg_pcrel_jmp22", 0, 20, MCFixupKindInfo::FKF_IsPCRel},
	    {"fixup_leg_pcrel_jmp27",  0, 25, MCFixupKindInfo::FKF_IsPCRel},
	    {"fixup_leg_pcrel_dat16", }
	};

	if (Kind < FirstTargetFixupKind)
		return MCAsmBackend::getFixupKindInfo(Kind);
	if (Kind >= FirstLiteralRelocationKind)
		return MCAsmBackend::getFixupKindInfo(FK_NONE);

	return Infos[Kind - FirstTargetFixupKind];
}

bool LEGAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count, const MCSubtargetInfo *STI) const {
	OS.write_zeros(Count);
	return true;
}

static uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value, MCContext &Ctx) {
	switch (Fixup.getTargetKind()) {
	case LEG::fixup_leg_pcrel_jmp22:
		return (Value >> 2) & 0xfffff;
	case LEG::fixup_leg_pcrel_jmp27:
		return (Value >> 2) & 0x1ffffff;
	default:
		llvm_unreachable("unknown fixup kind");
	}
}

void LEGAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup, const MCValue &Target, MutableArrayRef<char> Data,
	                           uint64_t Value, bool IsResolved, const MCSubtargetInfo *STI) const {
	outs() << "apply fixup\n";
	MCFixupKind Kind = Fixup.getKind();
	if (Kind >= FirstLiteralRelocationKind)
		return;
	MCContext &Ctx = Asm.getContext();
	MCFixupKindInfo Info = getFixupKindInfo(Kind);
	if (!Value)
		return;

	if (Info.TargetOffset + Info.TargetSize > 64)
		llvm_unreachable("adjustFixupValue not correclty implemented yet");

	Value = adjustFixupValue(Fixup, Value, Ctx);
	Value <<= Info.TargetOffset;

	//errs() << Fixup.getKind() << " - " << Fixup.getTargetKind() << " - " << Fixup.getOffset() << " - " << Fixup.getValue() << "\n";

	unsigned Offset = Fixup.getOffset();
	unsigned NumBytes = alignTo(Info.TargetSize + Info.TargetOffset, 8) / 8;
	assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

	for (unsigned i = 0; i != NumBytes; ++i) {
		unsigned idx = STI->getFeatureBits()[LEG::FeatureBIGENDIAN] ? (3 - i) : i;
		Data[Offset + idx] |= uint8_t(Value >> (i * 8));
	}
}

bool LEGAsmBackend::shouldForceRelocation(const MCAssembler &Asm, const MCFixup &Fixup, const MCValue &Target, const MCSubtargetInfo *STI) {
	return false;
}

std::unique_ptr<MCObjectTargetWriter>
LEGAsmBackend::createObjectTargetWriter() const {
	return createLEGELFObjectWriter(OSABI);
}

MCAsmBackend *llvm::createLEGAsmBackend(const Target &T, const MCSubtargetInfo &STI, const MCRegisterInfo &MRI, const MCTargetOptions &Options) {
	const Triple &TT = STI.getTargetTriple();
	uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
	return new LEGAsmBackend(STI, OSABI, Options);
}