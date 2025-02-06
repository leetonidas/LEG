#include "llvm/Target/TargetMachine.h"

namespace llvm {

class PassRegistry;
class FunctionPass;
class LEGTargetMachine;
class MachineInstr;
class MCInst;
class AsmPrinter;
class MachineOperand;
class MCOperand;

// Declare Functions that declare FunctionPasses
FunctionPass *createLEGExpandPseudoPass();
void initializeLEGExpandPseudoPass(PassRegistry &);

FunctionPass *createLEGISelDag(LEGTargetMachine &TM, CodeGenOptLevel OptLevel);

void initializeLEGDAGToDAGISelLegacyPass(PassRegistry &);

void lowerLEGMachineInstrToMCInst(const MachineInstr &MI, MCInst &OutMI, const AsmPrinter &AP);

bool lowerLEGMachineOperandToMCOperand(const MachineOperand &MO, MCOperand &MCOp, const AsmPrinter &AP);
}