#include <boost/test/unit_test.hpp>

#include "brainfuck/source_location.hpp"

#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(source_location)

BOOST_AUTO_TEST_CASE(basics)
{
    brainfuck::SourceLocation loc;

    BOOST_CHECK_EQUAL(1, loc.line());
    BOOST_CHECK_EQUAL(0, loc.column());

    loc.advance('+');

    BOOST_CHECK_EQUAL(1, loc.line());
    BOOST_CHECK_EQUAL(1, loc.column());

    loc.advance('\n');

    BOOST_CHECK_EQUAL(2, loc.line());
    BOOST_CHECK_EQUAL(0, loc.column());

    loc.advance('a');
    loc.advance('b');
    loc.advance('c');

    BOOST_CHECK_EQUAL(2, loc.line());
    BOOST_CHECK_EQUAL(3, loc.column());

    loc.advanceLine();

    BOOST_CHECK_EQUAL(3, loc.line());
    BOOST_CHECK_EQUAL(0, loc.column());

    brainfuck::SourceLocation loc2(10, 20);

    BOOST_CHECK_EQUAL(10, loc2.line());
    BOOST_CHECK_EQUAL(20, loc2.column());
}

BOOST_AUTO_TEST_CASE(comparisons)
{
    brainfuck::SourceLocation start(1, 0);
    brainfuck::SourceLocation mid(1, 10);
    brainfuck::SourceLocation end(2, 0);

    BOOST_CHECK_EQUAL(start, start);
    BOOST_CHECK_NE(start, end);

    BOOST_CHECK_LT(start, mid);
    BOOST_CHECK_LT(start, end);
    BOOST_CHECK_LT(mid, end);

    BOOST_CHECK_GT(end, mid);
    BOOST_CHECK_GT(end, start);
    BOOST_CHECK_GT(mid, start);

    BOOST_CHECK_GE(start, start);
    BOOST_CHECK_GE(mid, start);

    BOOST_CHECK_LE(mid, mid);
    BOOST_CHECK_LE(mid, end);
}

BOOST_AUTO_TEST_SUITE_END()
