Get-ChildItem -Path . -Include *.cpp,*.h -Recurse |
Where-Object { -not $_.FullName.Contains("LLVM") } |
Where-Object { -not $_.FullName.Contains("json") } |
ForEach-Object -Parallel { clang-format-20 -i $_.FullName }