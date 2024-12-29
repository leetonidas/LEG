#pragma once

// only required for code emission

#include "llvm/MC/MCFixup.h"

#undef LEG

namespace llvm::LEG {
enum Fixups {
	fixup_leg_imm64 = FirstTargetFixupKind,
	fixup_leg_pcrel_jmp22,
	fixup_leg_pcrel_jmp27,
	fixup_leg_pcrel_dat16,
	NumTargetFixupKinds = fixup_leg_imm64 - FirstTargetFixupKind
};
}