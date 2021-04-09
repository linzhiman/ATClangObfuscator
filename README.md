# ATClangObfuscator
基于Clang进行代码混淆，支持黑、白名单，忽略目录，实际工程可用。


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

* 假设新建名字为CodeStyleRefactor

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

### 具体实现CodeStyleRefactor

* ```什么也没有```


### 配置目标工程

* 以ATClangObfuscatorTest工程为例

* 打开ATClangObfuscatorTest.xcodeproj，新建Analyze配置（PROJECT->Configurations->Duplicate "Debug" Configuration）

* 修改Build Settings中Analyze配置

   * Enable Modules (C and Object-C) = NO
   ```
   fatal error: no handler registered for module format 'obj'
   LLVM ERROR: unknown module format
   ```
   
   * Enable Index-While-Building Functionality = NO
   ```
   Xcode 9.0 adds -index-store-path to the build command. It's not supported in clang yet. See this explanation.
   You can remove it by disabling the build option Index-While-Building Functionality in Xcode.
   ```
   
   * [可选] User-Defined 添加CC、CXX，值为本地编译的clang可执行文件路径（如果没有使用预编译，此处可省略）
   ```
   error: PCH file built from a different branch ((clang-1100.0.33.17)) than the compiler ()
   
   /Users/linzhiman/llvm-project/build/Release/bin/clang
   ```
   
   * [可选] Other C++ Flags 添加 -isystem "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/"
   ```
   fatal error: 'string' file not found
   #include <string>
   ```

* 命令行进入ATClangObfuscatorTest目录，使用xcodebuild命令生成目标工程的compile_commands.json
```
xcodebuild clean build -project ATClangObfuscatorTest.xcodeproj -scheme ATClangObfuscatorTest -configuration Analyze -arch x86_64 -sdk iphonesimulator13.2  | tee xcodebuild.log | xcpretty --report json-compilation-database --output compile_commands.json
```

* 命令行进入llvm-project/build/Release/bin，执行CodeStyleRefactor工具
```
./CodeStyleRefactor /Users/linzhiman/ATClangObfuscator/ATClangObfuscatorTest/
```

* 如果目标工程使用了预编译，当用xcodebuild后又修改了预编译文件，执行工具时会提示一下错误。此时避免重新编译整个工程，可如下处理：   

   * 打开xcodebuild.log（目标工程根目录下）
   * 搜索ProcessPCH，并将完整命令拷贝到命令行重新执行
```
error: PCH file built from a different branch ((clang-1100.0.33.17)) than the compiler ()
```

* 如果提示类似‘fatal error: 'string' file not found’的错误，将以下目录拷贝到llvm-project/build/Release目录
```
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/
```

* 如果你移动了工具到其他目录，执行时报错类似‘fatal error: 'string' file not found’的错误，请在目标工程Other C++ Flags中添加相应路径：
```
-isystem
"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/"
```

* 注：工具使用Release编译可以极大的提高工具的执行效率，特别是目标工程有大量文件时

* 注：如果你不想编译clang，可以直接使用我编译的release包，参考clangObfuscator.py来混淆你的代码

### 参考资料

* [Clang 12 documentation](https://clang.llvm.org/docs/)
* [Clang Tutorial](http://swtv.kaist.ac.kr/courses/cs453-fall13/Clang%20tutorial%20v4.pdf)





