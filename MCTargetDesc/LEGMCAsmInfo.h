#pragma once

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class LEGMCAsmInfo : public MCAsmInfoELF {
	void anchor() override;

public:
	explicit LEGMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
};
}