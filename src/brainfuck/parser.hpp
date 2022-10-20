#ifndef INCLUDED_LLVM_BRAINFUCK_PARSER_HPP
#define INCLUDED_LLVM_BRAINFUCK_PARSER_HPP

#include "ast.hpp"
#include "lexer.hpp"

#include <stdexcept>

namespace brainfuck
{
    class ParseError : public std::runtime_error
    {
    public:
        ParseError(SourceLocation const &loc, std::string const &errMsg);
    };

    std::vector<AST> parse(Lexer &lexer);
}

#endif
