project(EventFlow CXX ASM)

add_library(evfl OBJECT
  include/evfl/param.h
  include/evfl/query.h
  include/ore/BinaryFileHeader.h
  include/ore/BitUtils.h
  include/ore/StringView.h
  include/ore/Types.h
  src/ore/BitUtils.cpp
  src/ore/BinaryFileHeader.cpp
)

target_compile_options(evfl PRIVATE -fno-exceptions)
target_compile_options(evfl PRIVATE -fno-strict-aliasing)
target_compile_options(evfl PRIVATE -Wno-invalid-offsetof)
target_include_directories(evfl PUBLIC include/)
