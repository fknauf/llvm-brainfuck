#ifndef INCLUDED_LLVM_BRAINFUCK_SOURCE_LOCATION_HPP
#define INCLUDED_LLVM_BRAINFUCK_SOURCE_LOCATION_HPP

#include <compare>
#include <ostream>

namespace brainfuck
{
    class SourceLocation
    {
    public:
        SourceLocation(int line = 1, int column = 0);

        void advance(char c);
        void advanceLine();

        int line() const { return line_; }
        int column() const { return column_; }

        friend auto operator<=>(SourceLocation const &lhs, SourceLocation const &rhs) = default;

    private:
        int line_;
        int column_;
    };

    std::ostream &operator<<(std::ostream &out, SourceLocation const &loc);
}

#endif
