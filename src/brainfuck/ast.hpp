#ifndef INCLUDED_LLVM_BRAINFUCK_AST_HPP
#define INCLUDED_LLVM_BRAINFUCK_AST_HPP

#include "token.hpp"
#include "source_location.hpp"

#include <variant>
#include <vector>

namespace brainfuck
{
    template <Token opcode>
    class SimpleAST
    {
    public:
        SimpleAST(SourceLocation loc = {}) : loc_(loc) {}

        auto location() const { return loc_; }

    private:
        SourceLocation loc_;
    };

    using LeftAST = SimpleAST<Token::left>;
    using RightAST = SimpleAST<Token::right>;
    using IncrAST = SimpleAST<Token::incr>;
    using DecrAST = SimpleAST<Token::decr>;
    using WriteAST = SimpleAST<Token::write>;
    using ReadAST = SimpleAST<Token::read>;

    class LoopAST;

    using AST = std::variant<LeftAST, RightAST, IncrAST, DecrAST, WriteAST, ReadAST, LoopAST>;

    class LoopAST
    {
    public:
        LoopAST(SourceLocation loc, std::vector<AST> loopBody);

        auto const &loopBody() const { return loopBody_; }
        auto location() const { return loc_; }

    private:
        SourceLocation loc_;
        std::vector<AST> loopBody_;
    };

    SourceLocation astLocation(AST const &ast);
}

#endif
