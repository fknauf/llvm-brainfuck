#include <boost/test/unit_test.hpp>

#include "brainfuck/codegen.hpp"
#include "brainfuck/objcode.hpp"
#include "brainfuck/parser.hpp"

#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(codegen)

BOOST_AUTO_TEST_CASE(normalcode)
{
    std::string source = "[+-<>.,]";
    std::istringstream sourceStream(source);

    brainfuck::Lexer lexer(sourceStream);
    auto ast = brainfuck::parse(lexer);

    brainfuck::ObjCodeWriter writer;
    brainfuck::CodeGenerator codegen(writer.getDataLayout());

    codegen(ast);

    auto tsafeModule = codegen.finalizeModule();
    auto module = tsafeModule.getModuleUnlocked();

    module->print(llvm::errs(), nullptr);

    writer.writeModuleToFile("module.o", *module);
    writer.writeModuleToFile("module.asm", *module, llvm::CGFT_AssemblyFile);
}

BOOST_AUTO_TEST_SUITE_END()
