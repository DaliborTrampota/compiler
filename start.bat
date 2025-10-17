@echo off
:: Arguments:
:: -r: rebuild LLVM
:: -vs: start Visual Studio solution

if "%1"=="-vs" (
    echo VS flag detected
    start build/my_compiler.sln
    exit /b
)

set REBUILD=0
if "%1"=="-r" (
    echo Rebuild flag detected
    set REBUILD=1
)

if not exist LLVM/build (
    set REBUILD=1
)

if not exist build (
    mkdir build
)

if %REBUILD% equ 1 (
    echo Building LLVM...
    call build_llvm.bat
)

echo Building my_compiler...
cmake -S . -B build -DLLVM_DIR="./LLVM/build/lib/cmake/llvm"

echo Opening my_compiler.sln...
start build/my_compiler.sln