#include "ast.hpp"

namespace brainfuck
{
    ASTBase::ASTBase(SourceLocation const &loc)
        : loc_(loc)
    {
    }

    ASTBase::~ASTBase() {}

    LoopAST::LoopAST(SourceLocation const &loc, std::vector<AST> loopBody)
        : ASTBase(loc),
          loopBody_(std::move(loopBody))
    {
    }

    SourceLocation astLocation(AST const &ast)
    {
        return std::visit([](auto &a)
                          { return a.location(); },
                          ast);
    }
}
