#include "lexer.hpp"

#include <unordered_set>

namespace brainfuck
{
    Lexer::Lexer(std::istream &in)
        : in_{in},
          meaningfulTokens{{{'<', Token::LEFT},
                            {'>', Token::RIGHT},
                            {'+', Token::INCR},
                            {'-', Token::DECR},
                            {'.', Token::WRITE},
                            {',', Token::READ},
                            {'[', Token::LOOP_START},
                            {']', Token::LOOP_END}}}
    {
        advance();
    }

    void Lexer::advance()
    {
        char c;

        while (in_.get(c))
        {
            currentLocation_.advance(c);

            auto it = meaningfulTokens.find(c);
            if (it != meaningfulTokens.end())
            {
                currentToken_ = it->second;
                return;
            }

            continue;
        }

        currentToken_ = Token::END_OF_FILE;
    }
}
