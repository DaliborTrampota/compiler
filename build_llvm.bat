cmake -S LLVM/llvm -B LLVM/build `
  -A x64 `
  -DLLVM_ENABLE_PROJECTS="llvm" `
  -DLLVM_TARGETS_TO_BUILD="host" `
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build LLVM/build --config RelWithDebInfo --parallel 8
