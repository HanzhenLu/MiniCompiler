cmake_minimum_required(VERSION 3.16)
project(MiniCompiler)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# flex and bison
execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                COMMAND flex Lexer.l 
                COMMAND bison Parser.ypp -d)

# Now build our tools
add_executable(Compiler Lexer.cpp AST.cpp Parser.tab.cpp visualize.cpp IRGenerator.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(Compiler ${llvm_libs})