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

    BOOST_CHECK_EQUAL(brainfuck::Token::incr, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::decr, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::left, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::right, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::loop_start, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::loop_end, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::write, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::read, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK(brainfuck::Token::end_of_file == lexer.currentToken());
}

BOOST_AUTO_TEST_SUITE_END()
