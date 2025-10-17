cmake_minimum_required(VERSION 3.20)
project(my_compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Always use the dynamic (DLL) runtime (/MD) even in Debug
# This avoids CRT mismatches when linking against Release LLVM
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()


add_subdirectory(json)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)
SOURCE_GROUP(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCES})
add_executable(my_compiler ${SOURCES})




# Include both source and build directories
target_include_directories(my_compiler PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_BINARY_DIR}/include
)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})


# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    IRReader
    Analysis
    Target
    AsmParser
    # AsmPrinter
    ExecutionEngine
    # MC
    # MCParser

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

# If no build type is specified, use RelWithDebInfo (optimized + debug info)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()