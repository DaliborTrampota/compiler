cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project(my_compiler)

add_subdirectory(json)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(my_compiler src/main.cpp)
# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

# Include both source and build directories
target_include_directories(my_compiler PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_BINARY_DIR}/include
)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Now build our tools

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    IRReader
    Analysis
    Target
    AsmParser
    ExecutionEngine

    # Target-specific libraries
    X86AsmParser
    X86CodeGen
    X86Desc
    X86Disassembler
    X86Info
    X86TargetMCA
)

# Link against LLVM libraries and nlohmann_json
target_link_libraries(my_compiler 
    ${LLVM_LIBS}
    nlohmann_json::nlohmann_json
)