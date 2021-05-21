project(EventFlow CXX ASM)

add_library(evfl OBJECT
  include/evfl/param.h
  include/evfl/query.h
  include/ore/BinaryFile.h
  include/ore/BitUtils.h
  include/ore/EnumUtil.h
  include/ore/IterRange.h
  include/ore/ResDic.h
  include/ore/ResEndian.h
  include/ore/ResMetaData.h
  include/ore/StringPool.h
  include/ore/StringView.h
  include/ore/Types.h
  src/ore/BitUtils.cpp
  src/ore/EnumUtil.cpp
  src/ore/BinaryFile.cpp
  src/ore/ResDic.cpp
  src/ore/StringPool.cpp
)

target_compile_options(evfl PRIVATE -fno-exceptions)
target_compile_options(evfl PRIVATE -fno-strict-aliasing)
target_compile_options(evfl PRIVATE -Wno-invalid-offsetof)
target_include_directories(evfl PUBLIC include/)
