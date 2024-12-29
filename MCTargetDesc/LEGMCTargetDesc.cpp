#include "LEGMCTargetDesc.h"
//#include "LEGELFStreamer.h"
#include "LEGInstPrinter.h"
#include "LEGMCAsmInfo.h"
//#include "LEGTargetStreamer.h"
#include "TargetInfo/LEGTargetInfo.h"

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "LEGGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LEGGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "LEGGenRegisterInfo.inc"

using namespace llvm;

MCInstrInfo *createLEGMCInstrInfo() {
	MCInstrInfo *X = new MCInstrInfo();
	InitLEGMCInstrInfo(X);

	return X;
}

static MCRegisterInfo *createLEGMCRegisterInfo(const Triple &TT) {
	MCRegisterInfo *X = new MCRegisterInfo();
	InitLEGMCRegisterInfo(X, 0);

	return X;
}

static MCSubtargetInfo *createLEGMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
	if (CPU.empty())
		CPU = "generic";

	return createLEGMCSubtargetInfoImpl(TT, CPU, /*Tune*/ CPU, FS);
}

static MCInstPrinter *createLEGMCInstPrinter(const Triple &TT,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
	if (SyntaxVariant == 0) {
		return new LEGInstPrinter(MAI, MII, MRI);
	}

	return nullptr;
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter,
                                    bool RelaxAll) {
	return createELFStreamer(Context, std::move(MAB), std::move(OW), std::move(Emitter), RelaxAll);
}

/*
static MCTargetStreamer *createLEGObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
	return new LEGELFStreamer(S, STI);
}

static MCTargetStreamer *createMCAsmTargetStreamer(MCStreamer &S, formatted_raw_ostream &OS, MCInstPrinter *InstPrint, bool isVerboseAsm) {
	return new LEGTargetAsmStreamer(S);
}
*/

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLEGTargetMC() {
	RegisterMCAsmInfo<LEGMCAsmInfo> X(getTheLEGTarget());

	TargetRegistry::RegisterMCInstrInfo(getTheLEGTarget(), createLEGMCInstrInfo);

	TargetRegistry::RegisterMCRegInfo(getTheLEGTarget(), createLEGMCRegisterInfo);

	TargetRegistry::RegisterMCSubtargetInfo(getTheLEGTarget(), createLEGMCSubtargetInfo);

	TargetRegistry::RegisterMCInstPrinter(getTheLEGTarget(), createLEGMCInstPrinter);

	TargetRegistry::RegisterMCCodeEmitter(getTheLEGTarget(), createLEGMCCodeEmitter);

	TargetRegistry::RegisterELFStreamer(getTheLEGTarget(), createMCStreamer);

	//TargetRegistry::RegisterObjectTargetStreamer(getTheLEGTarget(), createLEGObjectTargetStreamer);

	//TargetRegistry::RegisterAsmTargetStreamer(getTheLEGTarget(), createMCAsmTargetStreamer);

	TargetRegistry::RegisterMCAsmBackend(getTheLEGTarget(), createLEGAsmBackend);
}