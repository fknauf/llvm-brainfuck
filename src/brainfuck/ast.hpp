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

    using LeftAST = SimpleAST<Token::LEFT>;
    using RightAST = SimpleAST<Token::RIGHT>;
    using IncrAST = SimpleAST<Token::INCR>;
    using DecrAST = SimpleAST<Token::DECR>;
    using WriteAST = SimpleAST<Token::WRITE>;
    using ReadAST = SimpleAST<Token::READ>;

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
