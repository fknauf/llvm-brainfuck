#ifndef INCLUDED_LLVM_BRAINFUCK_AST_HPP
#define INCLUDED_LLVM_BRAINFUCK_AST_HPP

#include "source_location.hpp"

#include <variant>
#include <vector>

namespace brainfuck
{
    class ASTBase
    {
    public:
        ASTBase(SourceLocation const &loc);
        virtual ~ASTBase() = 0;

        SourceLocation const &location() const { return loc_; }

    private:
        SourceLocation loc_;
    };

    struct LeftAST : public ASTBase
    {
        using ASTBase::ASTBase;
    };

    struct RightAST : public ASTBase
    {
        using ASTBase::ASTBase;
    };

    struct IncrAST : public ASTBase
    {
        using ASTBase::ASTBase;
    };

    struct DecrAST : public ASTBase
    {
        using ASTBase::ASTBase;
    };

    struct WriteAST : public ASTBase
    {
        using ASTBase::ASTBase;
    };

    struct ReadAST : public ASTBase
    {
        using ASTBase::ASTBase;
    };

    class LoopAST;

    using AST = std::variant<LeftAST, RightAST, IncrAST, DecrAST, WriteAST, ReadAST, LoopAST>;

    class LoopAST : public ASTBase
    {
    public:
        LoopAST(SourceLocation const &loc, std::vector<AST> loopBody);

        std::vector<AST> const &loopBody() const { return loopBody_; }

    private:
        std::vector<AST> loopBody_;
    };

    SourceLocation astLocation(AST const &ast);
}

#endif
