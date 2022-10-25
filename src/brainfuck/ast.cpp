#include "ast.hpp"

namespace brainfuck
{
    LoopAST::LoopAST(SourceLocation loc, std::vector<AST> loopBody)
        : loc_(loc),
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
