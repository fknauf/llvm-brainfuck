#ifndef INCLUDED_LLVM_BRAINFUCK_LEXER_HPP
#define INCLUDED_LLVM_BRAINFUCK_LEXER_HPP

#include "source_location.hpp"
#include "token.hpp"

#include <istream>
#include <unordered_map>

namespace brainfuck
{
    class Lexer
    {
    public:
        Lexer(std::istream &in);
        Lexer(Lexer const &) = delete;
        Lexer(Lexer &&) = delete;
        Lexer &operator=(Lexer const &) = delete;
        Lexer &operator=(Lexer &&) = delete;

        Token currentToken() const { return currentToken_; }
        auto currentLocation() const { return currentLocation_; }

        void advance();

    private:
        std::istream &in_;
        std::unordered_map<char, Token> const meaningfulTokens;

        Token currentToken_;
        SourceLocation currentLocation_;
    };
}

#endif
