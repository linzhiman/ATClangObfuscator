set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_executable(CodeStyleRefactor
  CodeStyleRefactor.cpp
  CSUtils.cpp
  CSUtils.hpp
  )

clang_target_link_libraries(CodeStyleRefactor
  PRIVATE
  clangAST
  clangBasic
  clangFormat
  clangFrontend
  clangLex
  clangRewrite
  clangSerialization
  clangTooling
  clangToolingCore
  clangToolingRefactoring
  )
