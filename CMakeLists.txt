project(llvm-brainfuck)

cmake_minimum_required(VERSION 3.24)

set(USE_LLVM_VERSION "16")
option(USE_ASAN "use adress sanitizer" on)
option(USE_STACK_PROTECTOR "use stack protector" on)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_COMPILER clang++-${USE_LLVM_VERSION})
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wextra -Werror)

if(USE_STACK_PROTECTOR)
    add_compile_options(-fstack-protector -fstack-protector-all)
endif(USE_STACK_PROTECTOR)

if(USE_ASAN) 
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif(USE_ASAN)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(tests)
