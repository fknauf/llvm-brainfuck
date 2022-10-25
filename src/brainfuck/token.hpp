#ifndef INCLUDED_LLVM_BRAINFUCK_TOKEN_HPP
#define INCLUDED_LLVM_BRAINFUCK_TOKEN_HPP

#include <ostream>
#include <string>

namespace brainfuck
{
    enum class Token
    {
#define TOKEN(tok) tok,
#include "token.list"
    };

    std::string to_string(Token tok);
    std::ostream &operator<<(std::ostream &out, Token tok);
};

#endif
