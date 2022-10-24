link_libraries(c++abi LLVM-14)
add_library(brainfuck
            brainfuck/ast.cpp
            brainfuck/codegen.cpp
            brainfuck/lexer.cpp
            brainfuck/objcode.cpp
            brainfuck/optimizer.cpp
            brainfuck/parser.cpp
            brainfuck/source_location.cpp
)

add_executable(bfcompile cli.cpp)
target_link_libraries(bfcompile brainfuck)
