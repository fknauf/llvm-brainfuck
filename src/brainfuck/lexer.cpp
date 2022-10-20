#include "lexer.hpp"

#include <unordered_set>

namespace brainfuck
{
    Lexer::Lexer(std::istream &in)
        : in_{in},
          meaningfulTokens{LEFT, RIGHT, INCR, DECR, WRITE, READ, LOOP_START, LOOP_END}
    {
        advance();
    }

    void Lexer::advance()
    {
        char c;

        while (in_.get(c) && !meaningfulTokens.contains(c))
        {
            currentLocation_.advance(c);
        }

        if (in_)
        {
            currentLocation_.advance(c);
            currentToken_ = c;
        }
        else
        {
            currentToken_ = END_OF_FILE;
        }
    }
}
