
include "llvm/Target/Target.td"

include "LEGDevices.td"

include "LEGRegisterInfo.td"

include "LEGCallingConv.td"

include "LEGInstrInfo.td"

def LEGInstrInfo : InstrInfo;

def LEGAsmParser : AsmParser;

def LEGAsmWriter : AsmWriter {
	// printInstruciton and alike will take "MCSubtargetInfo &STI" as additional argument
	int PassSubtarget = 1;
}

def : Processor<"generic", NoItineraries, []>;

def LEG : Target {
	let InstructionSet = LEGInstrInfo;
	let AssemblyParsers = [LEGAsmParser];
	let AssemblyWriters = [LEGAsmWriter];
}
