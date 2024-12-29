#pragma once

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCCodeEmitter *createLEGMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx);

MCAsmBackend *createLEGAsmBackend(const Target &T, const MCSubtargetInfo &STI, const MCRegisterInfo &MRI, const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createLEGELFObjectWriter(uint8_t OSABI);

}

#define GET_REGINFO_ENUM
#include "LEGGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "LEGGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "LEGGenSubtargetInfo.inc"
