#include <boost/test/unit_test.hpp>

#include "brainfuck/parser.hpp"

#include <sstream>
#include <string>

namespace
{
    template <typename T>
    struct ExpectType
    {
        bool operator()(T const &) const
        {
            return true;
        }

        bool operator()(brainfuck::ASTBase const &) const
        {
            return false;
        }
    };

    template <typename T, int change>
    struct ExpectChange
    {
        bool operator()(T const &t) const
        {
            return t.change() == change;
        }

        bool operator()(brainfuck::ASTBase const &) const
        {
            return false;
        }
    };
}

BOOST_AUTO_TEST_SUITE(parser)

BOOST_AUTO_TEST_CASE(normalcode)
{
    std::string source = "+-<>[].,";
    std::istringstream sourceStream(source);
    brainfuck::Lexer lexer(sourceStream);

    std::vector<brainfuck::AST> ast = brainfuck::parse(lexer);

    BOOST_CHECK_EQUAL(7, ast.size());

    BOOST_CHECK(std::visit(ExpectChange<brainfuck::DataChangeAST, 1>(), ast[0]));
    BOOST_CHECK(std::visit(ExpectChange<brainfuck::DataChangeAST, -1>(), ast[1]));
    BOOST_CHECK(std::visit(ExpectChange<brainfuck::PositionChangeAST, -1>(), ast[2]));
    BOOST_CHECK(std::visit(ExpectChange<brainfuck::PositionChangeAST, 1>(), ast[3]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::LoopAST>(), ast[4]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::WriteAST>(), ast[5]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::ReadAST>(), ast[6]));

    brainfuck::LoopAST *loop = std::get_if<brainfuck::LoopAST>(&ast[4]);
    BOOST_CHECK(loop);

    BOOST_CHECK_EQUAL(0, loop->loopBody().size());

    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 1), astLocation(ast[0]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 2), astLocation(ast[1]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 3), astLocation(ast[2]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 4), astLocation(ast[3]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 5), astLocation(ast[4]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 7), astLocation(ast[5]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 8), astLocation(ast[6]));
}

BOOST_AUTO_TEST_CASE(comments_lines)
{
    std::string source =
        "daten +-\n"
        "pos < x >\n"
        "[loop]\n"
        "io ., ";
    std::istringstream sourceStream(source);
    brainfuck::Lexer lexer(sourceStream);

    std::vector<brainfuck::AST> ast = brainfuck::parse(lexer);

    BOOST_CHECK_EQUAL(7, ast.size());

    BOOST_CHECK(std::visit(ExpectChange<brainfuck::DataChangeAST, 1>(), ast[0]));
    BOOST_CHECK(std::visit(ExpectChange<brainfuck::DataChangeAST, -1>(), ast[1]));
    BOOST_CHECK(std::visit(ExpectChange<brainfuck::PositionChangeAST, -1>(), ast[2]));
    BOOST_CHECK(std::visit(ExpectChange<brainfuck::PositionChangeAST, 1>(), ast[3]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::LoopAST>(), ast[4]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::WriteAST>(), ast[5]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::ReadAST>(), ast[6]));

    brainfuck::LoopAST *loop = std::get_if<brainfuck::LoopAST>(&ast[4]);
    BOOST_CHECK(loop);

    BOOST_CHECK_EQUAL(0, loop->loopBody().size());

    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 7), astLocation(ast[0]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(1, 8), astLocation(ast[1]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(2, 5), astLocation(ast[2]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(2, 9), astLocation(ast[3]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(3, 1), astLocation(ast[4]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(4, 4), astLocation(ast[5]));
    BOOST_CHECK_EQUAL(brainfuck::SourceLocation(4, 5), astLocation(ast[6]));
}

BOOST_AUTO_TEST_CASE(nested_loops)
{
    std::string source = "[.[.].[[]].]";
    std::istringstream sourceStream(source);
    brainfuck::Lexer lexer(sourceStream);

    std::vector<brainfuck::AST> ast = brainfuck::parse(lexer);

    BOOST_CHECK_EQUAL(1, ast.size());

    BOOST_CHECK(std::visit(ExpectType<brainfuck::LoopAST>(), ast[0]));

    auto outerLoop = std::get_if<brainfuck::LoopAST>(&ast[0]);
    BOOST_CHECK(outerLoop);

    BOOST_CHECK_EQUAL(5, outerLoop->loopBody().size());

    BOOST_CHECK(std::visit(ExpectType<brainfuck::WriteAST>(), outerLoop->loopBody()[0]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::LoopAST>(), outerLoop->loopBody()[1]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::WriteAST>(), outerLoop->loopBody()[2]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::LoopAST>(), outerLoop->loopBody()[3]));
    BOOST_CHECK(std::visit(ExpectType<brainfuck::WriteAST>(), outerLoop->loopBody()[4]));

    auto firstInnerLoop = std::get_if<brainfuck::LoopAST>(&outerLoop->loopBody()[1]);
    BOOST_CHECK(firstInnerLoop);

    BOOST_CHECK_EQUAL(1, firstInnerLoop->loopBody().size());
    BOOST_CHECK(std::visit(ExpectType<brainfuck::WriteAST>(), firstInnerLoop->loopBody()[0]));

    auto secondInnerLoop = std::get_if<brainfuck::LoopAST>(&outerLoop->loopBody()[3]);
    BOOST_CHECK(secondInnerLoop);

    BOOST_CHECK_EQUAL(1, secondInnerLoop->loopBody().size());
    BOOST_CHECK(std::visit(ExpectType<brainfuck::LoopAST>(), secondInnerLoop->loopBody()[0]));

    auto innerInnerLoop = std::get_if<brainfuck::LoopAST>(&secondInnerLoop->loopBody()[0]);
    BOOST_CHECK(innerInnerLoop);
    BOOST_CHECK(innerInnerLoop->loopBody().empty());
}

BOOST_AUTO_TEST_SUITE_END()
