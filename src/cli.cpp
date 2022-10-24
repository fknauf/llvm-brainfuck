#include "brainfuck/lexer.hpp"
#include "brainfuck/parser.hpp"
#include "brainfuck/objcode.hpp"
#include "brainfuck/codegen.hpp"
#include "brainfuck/optimizer.hpp"

#include <fstream>
#include <iostream>

namespace
{
    void dumpModule(llvm::Module &module, brainfuck::ObjCodeWriter &objWriter, std::string const &fileNameStem)
    {
        std::error_code ec;
        llvm::raw_fd_ostream llOut(fileNameStem + ".ll", ec);
        if (!ec)
        {
            module.print(llOut, nullptr);
        }

        objWriter.writeModuleToFile(fileNameStem + ".o", module);
        objWriter.writeModuleToFile(fileNameStem + ".asm", module, llvm::CGFT_AssemblyFile);
    }

    void do_compile(std::istream &in, std::string const &moduleName)
    {
        brainfuck::Lexer lexer(in);
        brainfuck::ObjCodeWriter objWriter;
        brainfuck::CodeGenerator codegen(objWriter.getDataLayout());

        auto ast = brainfuck::parse(lexer);
        codegen(ast);

        auto tsModule = codegen.finalizeModule();
        auto &module = *tsModule.getModuleUnlocked();

        dumpModule(module, objWriter, moduleName + "_unoptimized");

        brainfuck::optimizeModule(module);

        dumpModule(module, objWriter, moduleName);
    }
}

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        do_compile(std::cin, "module");
    }
    else
    {
        std::string fileName = argv[1];
        std::ifstream in(fileName);

        if (in)
        {
            do_compile(in, fileName);
        }
        else
        {
            std::cerr << "Could not open " << fileName << std::endl;
        }
    }
}
