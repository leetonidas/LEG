add_llvm_component_library(LLVMLEGDesc
  LEGAsmBackend.cpp
  LEGELFObjectWriter.cpp
  LEGELFStreamer.cpp
  LEGInstPrinter.cpp
  LEGMCAsmInfo.cpp
  LEGMCCodeEmitter.cpp
  LEGMCELFStreamer.cpp
  LEGMCExpr.cpp
  LEGMCTargetDesc.cpp
  LEGTargetStreamer.cpp

  LINK_COMPONENTS
  MC
  LEGInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  LEG
)

add_dependencies(LLVMLEGDesc LEGCommonTableGen)

