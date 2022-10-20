#include <boost/test/unit_test.hpp>

#include "brainfuck/lexer.hpp"

#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(lexer)

BOOST_AUTO_TEST_CASE(normalcode)
{
    std::string source = "+-<>[].,";
    std::istringstream sourceStream(source);

    brainfuck::Lexer lexer(sourceStream);

    for (char c : source)
    {
        BOOST_CHECK_EQUAL(c, lexer.currentToken());

        lexer.advance();
    }

    BOOST_CHECK_EQUAL(brainfuck::Lexer::END_OF_FILE, lexer.currentToken());
}

BOOST_AUTO_TEST_SUITE_END()
