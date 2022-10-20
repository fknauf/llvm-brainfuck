#include "parser.hpp"
#include "source_location.hpp"

#include <cassert>

namespace brainfuck
{
    namespace
    {
        void expectToken(Lexer &lexer, char expected, std::string const &errMsg)
        {
            if (lexer.currentToken() != expected)
            {
                throw ParseError(lexer.currentLocation(), errMsg);
            }

            lexer.advance();
        }

        std::vector<AST> parseLoopBody(Lexer &lexer);

        LoopAST parseLoop(Lexer &lexer)
        {
            SourceLocation loopStart = lexer.currentLocation();

            expectToken(lexer, Lexer::LOOP_START, "Expected [ at start of loop");

            auto loopBody = parseLoopBody(lexer);

            if (lexer.currentToken() != Lexer::LOOP_END)
            {
                throw ParseError(lexer.currentLocation(), "Expected ] at end of loop");
            }

            return {loopStart, std::move(loopBody)};
        }

        std::vector<AST> parseLoopBody(Lexer &lexer)
        {
            std::vector<AST> body;

            while (lexer.currentToken() != Lexer::END_OF_FILE && lexer.currentToken() != ']')
            {
                switch (lexer.currentToken())
                {
                case Lexer::RIGHT:
                    body.emplace_back(PositionChangeAST(lexer.currentLocation(), 1));
                    break;
                case Lexer::LEFT:
                    body.emplace_back(PositionChangeAST(lexer.currentLocation(), -1));
                    break;
                case Lexer::INCR:
                    body.emplace_back(DataChangeAST(lexer.currentLocation(), 1));
                    break;
                case Lexer::DECR:
                    body.emplace_back(DataChangeAST(lexer.currentLocation(), -1));
                    break;
                case Lexer::WRITE:
                    body.emplace_back(WriteAST(lexer.currentLocation()));
                    break;
                case Lexer::READ:
                    body.emplace_back(ReadAST(lexer.currentLocation()));
                    break;
                case Lexer::LOOP_START:
                    body.emplace_back(parseLoop(lexer));
                    break;
                default:
                    throw ParseError(lexer.currentLocation(), std::string("unexpected in loop body: ") + lexer.currentToken());
                }

                lexer.advance();
            }

            return body;
        }
    }

    ParseError::ParseError(SourceLocation const &loc, std::string const &errMsg)
        : runtime_error(to_string(loc) + " " + errMsg)
    {
    }

    std::vector<AST> parse(Lexer &lexer)
    {
        std::vector<AST> mainProgram = parseLoopBody(lexer);

        expectToken(lexer, Lexer::END_OF_FILE, "unmatched ]");

        return mainProgram;
    }
}
