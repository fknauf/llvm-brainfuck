include_directories(BEFORE ../src)
add_executable(test
               test_main.cpp
               group_codegen.cpp
               group_lexer.cpp
               group_parser.cpp
               group_source_location.cpp
)
target_link_libraries(test brainfuck boost_unit_test_framework boost_filesystem)
