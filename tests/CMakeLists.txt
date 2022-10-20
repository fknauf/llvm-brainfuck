link_libraries(brainfuck boost_unit_test_framework)

include_directories(BEFORE ../src)
add_executable(test
               test_main.cpp
               group_lexer.cpp
               group_parser.cpp
               group_source_location.cpp
)
