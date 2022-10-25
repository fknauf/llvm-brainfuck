#include "source_location.hpp"

#include <sstream>

namespace brainfuck
{
    SourceLocation::SourceLocation(int line, int column)
        : line_(line),
          column_(column)
    {
    }

    void SourceLocation::advance(char c)
    {
        if (c == '\n')
        {
            advanceLine();
        }
        else
        {
            ++column_;
        }
    }

    void SourceLocation::advanceLine()
    {
        column_ = 0;
        ++line_;
    }

    std::ostream &operator<<(std::ostream &out, SourceLocation const &loc)
    {
        return out << loc.line() << ":" << loc.column();
    }
}
