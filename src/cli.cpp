#include "brainfuck/lexer.hpp"
#include "brainfuck/parser.hpp"
#include "brainfuck/objcode.hpp"
#include "brainfuck/codegen.hpp"
#include "brainfuck/optimizer.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{
    void dumpModule(llvm::Module &module, brainfuck::ObjCodeWriter &objWriter, std::filesystem::path const &fileNameStem)
    {
        std::error_code ec;
        llvm::raw_fd_ostream llOut(fileNameStem.string() + ".ll", ec);
        if (!ec)
        {
            module.print(llOut, nullptr);
        }

        objWriter.writeModuleToFile(fileNameStem.string() + ".o", module);
        objWriter.writeModuleToFile(fileNameStem.string() + ".asm", module, llvm::CGFT_AssemblyFile);
    }

    void do_compile(std::istream &in, std::filesystem::path const &sourcePath)
    {
        brainfuck::Lexer lexer(in);
        brainfuck::ObjCodeWriter objWriter;
        brainfuck::CodeGenerator codegen(objWriter.getDataLayout(), sourcePath, true);

        auto ast = brainfuck::parse(lexer);
        codegen(ast);

        auto tsModule = codegen.finalizeModule();
        auto &module = *tsModule.getModuleUnlocked();

        auto pathStem = sourcePath.parent_path() / sourcePath.stem();
        auto pathStemUnoptimized = pathStem;
        pathStemUnoptimized += "_unoptimized";

        dumpModule(module, objWriter, pathStemUnoptimized);

        brainfuck::optimizeModule(module);

        dumpModule(module, objWriter, pathStem);
    }
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string fileName = argv[i];
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
