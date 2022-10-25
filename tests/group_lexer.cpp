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

    BOOST_CHECK_EQUAL(brainfuck::Token::INCR, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::DECR, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::LEFT, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::RIGHT, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::LOOP_START, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::LOOP_END, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::WRITE, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK_EQUAL(brainfuck::Token::READ, lexer.currentToken());
    lexer.advance();
    BOOST_CHECK(brainfuck::Token::END_OF_FILE == lexer.currentToken());
}

BOOST_AUTO_TEST_SUITE_END()
