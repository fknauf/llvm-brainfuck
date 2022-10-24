#include "brainfuck/lexer.hpp"
#include "brainfuck/parser.hpp"
#include "brainfuck/objcode.hpp"
#include "brainfuck/codegen.hpp"
#include "brainfuck/optimizer.hpp"

#include <fstream>
#include <iostream>

namespace
{
    void do_compile(std::istream &in, std::string const &outFileName)
    {
        brainfuck::Lexer lexer(in);
        brainfuck::ObjCodeWriter objWriter;
        brainfuck::CodeGenerator codegen(objWriter.getDataLayout());

        auto ast = brainfuck::parse(lexer);
        codegen(ast);

        auto tsModule = codegen.finalizeModule();
        auto &module = *tsModule.getModuleUnlocked();
        brainfuck::optimizeModule(module);

        objWriter.writeModuleToFile(outFileName, module);
    }
}

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        do_compile(std::cin, "module.o");
    }
    else
    {
        std::string fileName = argv[1];
        std::ifstream in(fileName);

        if (in)
        {
            do_compile(in, fileName + ".o");
        }
        else
        {
            std::cerr << "Could not open " << fileName << std::endl;
        }
    }
}
