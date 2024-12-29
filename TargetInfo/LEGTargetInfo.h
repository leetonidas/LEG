#pragma once

namespace llvm {
class Target;

Target &getTheLEGTarget();

namespace LEG {
	int getPredOpcode(unsigned short Opcode);
}

}

