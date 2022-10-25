#include <boost/test/unit_test.hpp>

#include "brainfuck/codegen.hpp"
#include "brainfuck/objcode.hpp"
#include "brainfuck/optimizer.hpp"
#include "brainfuck/parser.hpp"

#include <boost/process.hpp>

#include <unistd.h>

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(codegen)

namespace
{
    void testRunModule(llvm::Module &module, brainfuck::ObjCodeWriter &writer, std::string const &expectedOutput, std::string const &tag)
    {
        auto tempPath = std::filesystem::temp_directory_path();
        auto pidStr = std::to_string(getpid());
        auto tempModule = tempPath / (pidStr + "_" + tag + "_module.o");
        auto tempAout = tempPath / (pidStr + "_" + tag + "_a.out");

        writer.writeModuleToFile(tempModule.string(), module);

        boost::process::child linkerProcess(boost::process::search_path("cc"), tempModule.string(), "-o", tempAout.string());
        linkerProcess.wait();

        boost::process::ipstream readerStream;
        std::ostringstream bufferStream;
        boost::process::child runnerProcess(tempAout.string(), boost::process::std_out > readerStream);
        runnerProcess.wait();

        bufferStream << readerStream.rdbuf();
        BOOST_CHECK_EQUAL(expectedOutput, bufferStream.str());

        std::error_code ec;

        std::filesystem::remove(tempAout, ec);
        BOOST_CHECK(!ec);
        std::filesystem::remove(tempModule, ec);
        BOOST_CHECK(!ec);
    }
}

BOOST_AUTO_TEST_CASE(normalcode)
{
    std::string source = ">++++++++[<+++++++++>-]<.>++++[<+++++++>-]<+.+++++++..+++.>>++++++[<+++++++>-]<+"
                         "+.------------.>++++++[<+++++++++>-]<+.<.+++.------.--------.>>>++++[<++++++++>-"
                         "]<+.";
    std::istringstream sourceStream(source);

    brainfuck::Lexer lexer(sourceStream);
    auto ast = brainfuck::parse(lexer);

    brainfuck::ObjCodeWriter writer;
    brainfuck::CodeGenerator codegen(writer.getDataLayout());

    codegen(ast);

    auto tsafeModule = codegen.finalizeModule();
    auto module = tsafeModule.getModuleUnlocked();

    testRunModule(*module, writer, "Hello, World!", "unoptimized");
    brainfuck::optimizeModule(*module);
    testRunModule(*module, writer, "Hello, World!", "optimized");
}

BOOST_AUTO_TEST_SUITE_END()
