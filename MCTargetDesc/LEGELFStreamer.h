#pragma once;

#include "LEGTargetStreamer.h"
#include "llvm/MC/MCELFStreamer.h"

namespace llvm {
class LEGELFStreamer : public MCELFStreamer {
};
}