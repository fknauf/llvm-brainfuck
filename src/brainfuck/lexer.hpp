#ifndef INCLUDED_LLVM_BRAINFUCK_LEXER_HPP
#define INCLUDED_LLVM_BRAINFUCK_LEXER_HPP

#include "source_location.hpp"

#include <istream>
#include <unordered_set>

namespace brainfuck
{
    class Lexer
    {
    public:
        enum
        {
            END_OF_FILE = '\0',
            LEFT = '<',
            RIGHT = '>',
            INCR = '+',
            DECR = '-',
            WRITE = '.',
            READ = ',',
            LOOP_START = '[',
            LOOP_END = ']'
        };

        Lexer(std::istream &in);

        char currentToken() const { return currentToken_; }
        auto const &currentLocation() const { return currentLocation_; }

        void advance();

    private:
        std::istream &in_;
        std::unordered_set<char> const meaningfulTokens;

        char currentToken_;
        SourceLocation currentLocation_;
    };
}

#endif
