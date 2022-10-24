add_library(brainfuck
            brainfuck/ast.cpp
            brainfuck/codegen.cpp
            brainfuck/lexer.cpp
            brainfuck/objcode.cpp
            brainfuck/optimizer.cpp
            brainfuck/parser.cpp
            brainfuck/source_location.cpp
)
target_include_directories(brainfuck SYSTEM BEFORE PUBLIC /usr/lib/llvm-${USE_LLVM_VERSION}/include)
target_link_libraries(brainfuck LLVM-${USE_LLVM_VERSION})

add_executable(bfcompile cli.cpp)
target_link_libraries(bfcompile brainfuck)
