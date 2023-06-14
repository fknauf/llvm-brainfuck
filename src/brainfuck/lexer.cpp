#include "lexer.hpp"

#include <unordered_set>

namespace brainfuck
{
    Lexer::Lexer(std::istream &in)
        : in_{in},
          meaningfulTokens{{{'<', Token::left},
                            {'>', Token::right},
                            {'+', Token::incr},
                            {'-', Token::decr},
                            {'.', Token::write},
                            {',', Token::read},
                            {'[', Token::loop_start},
                            {']', Token::loop_end}}}
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

        currentToken_ = Token::end_of_file;
    }
}
