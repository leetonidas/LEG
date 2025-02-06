#include "LEGMCAsmInfo.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/MC/MCStreamer.h"

using namespace llvm;

void LEGMCAsmInfo::anchor() {}

LEGMCAsmInfo::LEGMCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
	CodePointerSize = 8;
	CommentString = ";";
	Data16bitsDirective = "\t.quater\t";
	Data32bitsDirective = "\t.half\t";
}