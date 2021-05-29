project(EventFlow CXX ASM)

add_library(evfl OBJECT
  include/evfl/EvflAllocator.h
  include/evfl/FlowchartContext.h
  include/evfl/MetaDataPack.h
  include/evfl/Param.h
  include/evfl/Query.h
  include/evfl/ResActor.h
  include/evfl/ResEventFlowFile.h
  include/evfl/ResFlowchart.h
  include/evfl/ResTimeline.h
  include/ore/Allocator.h
  include/ore/Array.h
  include/ore/BinaryFile.h
  include/ore/BitUtils.h
  include/ore/Buffer.h
  include/ore/EnumUtil.h
  include/ore/IterRange.h
  include/ore/RelocationTable.h
  include/ore/ResDic.h
  include/ore/ResEndian.h
  include/ore/ResMetaData.h
  include/ore/StringPool.h
  include/ore/StringView.h
  include/ore/Types.h
  src/evfl/MetaDataPack.cpp
  src/evfl/Param.cpp
  src/evfl/ResEventFlowFile.cpp
  src/evfl/ResTimeline.cpp
  src/evfl/ResActor.cpp
  src/evfl/ResFlowchart.cpp
  src/ore/BitUtils.cpp
  src/ore/EnumUtil.cpp
  src/ore/BinaryFile.cpp
  src/ore/RelocationTable.cpp
  src/ore/StringPool.cpp
  src/ore/ResDic.cpp
  src/ore/ResMetaData.cpp
)

target_compile_options(evfl PRIVATE -fno-exceptions)
target_compile_options(evfl PRIVATE -fno-strict-aliasing)
target_compile_options(evfl PRIVATE -Wno-invalid-offsetof)
target_include_directories(evfl PUBLIC include/)
