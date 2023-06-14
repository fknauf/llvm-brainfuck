#include "parser.hpp"
#include "source_location.hpp"

#include <cassert>
#include <sstream>

namespace brainfuck
{
    namespace
    {
        void expectToken(Lexer &lexer, Token expected, std::string_view errMsg)
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

            expectToken(lexer, Token::loop_start, "Expected [ at start of loop");

            auto loopBody = parseLoopBody(lexer);

            if (lexer.currentToken() != Token::loop_end)
            {
                throw ParseError(lexer.currentLocation(), "Expected ] at end of loop");
            }

            return {loopStart, std::move(loopBody)};
        }

        std::vector<AST> parseLoopBody(Lexer &lexer)
        {
            std::vector<AST> body;

            while (lexer.currentToken() != Token::end_of_file && lexer.currentToken() != Token::loop_end)
            {
                switch (lexer.currentToken())
                {
                case Token::right:
                    body.emplace_back(RightAST(lexer.currentLocation()));
                    break;
                case Token::left:
                    body.emplace_back(LeftAST(lexer.currentLocation()));
                    break;
                case Token::incr:
                    body.emplace_back(IncrAST(lexer.currentLocation()));
                    break;
                case Token::decr:
                    body.emplace_back(DecrAST(lexer.currentLocation()));
                    break;
                case Token::write:
                    body.emplace_back(WriteAST(lexer.currentLocation()));
                    break;
                case Token::read:
                    body.emplace_back(ReadAST(lexer.currentLocation()));
                    break;
                case Token::loop_start:
                    body.emplace_back(parseLoop(lexer));
                    break;
                default:
                    // Purely defensive programming; this should be impossible to reach
                    throw ParseError(lexer.currentLocation(), "unexpected token in loop body: " + to_string(lexer.currentToken()));
                }

                lexer.advance();
            }

            return body;
        }
    }

    ParseError::ParseError(SourceLocation loc, std::string_view errMsg)
        : runtime_error((std::ostringstream{} << loc << " " << errMsg).str())
    {
    }

    std::vector<AST> parse(Lexer &lexer)
    {
        std::vector<AST> mainProgram = parseLoopBody(lexer);

        expectToken(lexer, Token::end_of_file, "unmatched ]");

        return mainProgram;
    }
}
