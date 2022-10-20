link_libraries(c++abi LLVM-14)
add_library(brainfuck
            brainfuck/ast.cpp
            brainfuck/lexer.cpp
            brainfuck/parser.cpp
            brainfuck/source_location.cpp
)
