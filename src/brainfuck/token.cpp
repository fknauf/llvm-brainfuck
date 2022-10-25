#include "token.hpp"

#include <sstream>
#include <type_traits>

namespace brainfuck
{
    std::string to_string(Token tok)
    {
        switch (tok)
        {
#define TOKEN(tok)   \
    case Token::tok: \
        return #tok;
#include "token.list"
        }

        std::ostringstream unknownFmt;
        unknownFmt << "UNKNOWN (" << static_cast<std::underlying_type_t<Token>>(tok) << ")";
        return unknownFmt.str();
    }

    std::ostream &operator<<(std::ostream &out, Token tok)
    {
        return out << to_string(tok);
    }
}
