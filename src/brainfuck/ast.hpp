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

    template <int>
    class TaggedChangeAST : public ASTBase
    {
    public:
        TaggedChangeAST(SourceLocation const &loc, int change)
            : ASTBase(loc), change_(change) {}

        int change() const noexcept { return change_; }

    private:
        int change_;
    };

    using PositionChangeAST = TaggedChangeAST<0>;
    using DataChangeAST = TaggedChangeAST<1>;

    template <char>
    class TaggedEmptyAST : public ASTBase
    {
    public:
        using ASTBase::ASTBase;
    };

    using WriteAST = TaggedEmptyAST<'.'>;
    using ReadAST = TaggedEmptyAST<','>;

    class LoopAST;

    using AST = std::variant<PositionChangeAST, DataChangeAST, WriteAST, ReadAST, LoopAST>;

    class LoopAST : public ASTBase
    {
    public:
        LoopAST(SourceLocation const &loc, std::vector<AST> loopBody);

        LoopAST(LoopAST const &) = delete;
        LoopAST(LoopAST &&) = default;
        LoopAST &operator=(LoopAST const &) = delete;
        LoopAST &operator=(LoopAST &&) = default;

        std::vector<AST> const &loopBody() const { return loopBody_; }

    private:
        std::vector<AST> loopBody_;
    };

    SourceLocation astLocation(AST const &ast);
}

#endif
