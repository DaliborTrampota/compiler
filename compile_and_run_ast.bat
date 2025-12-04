@echo off

rm test\program.exe
rm test\runtime.o
rm %2

echo [1/4] Compiling runtime library...
clang -c test\runtime.c -o test\runtime.o || (echo ERROR: Failed to compile runtime.c & exit /b 1)

echo [2/4] Running compiler on %1 %2...
.\build\RelWithDebInfo\my_compiler.exe %1 %2 || (echo ERROR: Compiler failed & exit /b 1)

echo [3/4] Compiling generated IR...
clang %2 test\runtime.o -o test\program.exe || (echo ERROR: Failed to compile IR & exit /b 1)

echo [4/4] Running program:
echo ================================
test\program.exe
set EXIT_CODE=%ERRORLEVEL%
echo ================================
if %EXIT_CODE% neq 0 (
    echo ERROR: Program exited with code %EXIT_CODE%
    exit /b %EXIT_CODE%
)
echo Program completed successfully!