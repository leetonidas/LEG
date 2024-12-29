add_llvm_component_group(LEG)

set(LLVM_TARGET_DEFINITIONS LEG.td)

#tablegen(LLVM LEGGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM LEGGenAsmWriter.inc -gen-asm-writer)
#tablegen(LLVM LEGGenCompressInstEmitter.inc -gen-compress-inst-emitter)
tablegen(LLVM LEGGenCallingConv.inc -gen-callingconv)
tablegen(LLVM LEGGenDAGISel.inc -gen-dag-isel)
#tablegen(LLVM LEGGenDisassemblerTables.inc -gen-disassembler)
#tablegen(LLVM LEGGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM LEGGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM LEGGenMCCodeEmitter.inc -gen-emitter)
#tablegen(LLVM LEGGenMCPseudoLowering.inc -gen-pseudo-lowering)
#tablegen(LLVM LEGGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM LEGGenRegisterInfo.inc -gen-register-info)
#tablegen(LLVM LEGGenSearchableTables.inc -gen-searchable-tables)
tablegen(LLVM LEGGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(LEGCommonTableGen)

add_llvm_target(LEGCodeGen
  LEGAsmPrinter.cpp
  LEGExpandPseudoInsts.cpp
  LEGFrameLowering.cpp
  LEGInstrInfo.cpp
  LEGISelDAGToDAG.cpp
  LEGISelLowering.cpp
  LEGMCInstLower.cpp
  LEGRegisterInfo.cpp
  LEGSubtarget.cpp
  LEGTargetMachine.cpp
  LEGTargetObjectFile.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  Core
  MC
  LEGDesc
  LEGInfo
  SelectionDAG
  Support
  Target

  ADD_TO_COMPONENT
  LEG
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
