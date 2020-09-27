# ATClangObfuscator
基于Clang进行代码混淆


### 下载LLVM源码，生成LLVM.xcodeproj

* Apple's fork of llvm-project
    * Code：https://github.com/apple/llvm-project.git
    * Branch apple/stable/20200714
    
* 下载源码并生成XCode工程（需安装CMake）

```
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build
cd build
cmake -G Xcode -DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra' ../llvm
```

### 新建CLang工具，加入LLVM.xcodeproj（CLang libTooling）

* 假设新建名字为CodeStyleRefactor：
* 进入目录 ```llvm-project/clang-tools-extra```
* CMakeLists.txt 添加 ```add_subdirectory(CodeStyleRefactor)```
* 新建目录 ```CodeStyleRefactor```
* 进入目录 ```CodeStyleRefactor```
* 新建 CMakeList.txt 添加以下内容

```
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
```
* 进入llvm-project/build重新cmake
* 打开LLVM.xcodeproj，即可在Clang executables分组下看到 CodeStyleRefactor
* 打开LLVM.xcodeproj时会提示自动创建Scheme，建议不自动创建，后续手动创建clang和CodeStyleRefactor两个即可



