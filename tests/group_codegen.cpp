#include <boost/test/unit_test.hpp>

#include "brainfuck/codegen.hpp"
#include "brainfuck/objcode.hpp"
#include "brainfuck/optimizer.hpp"
#include "brainfuck/parser.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/process.hpp>
#include <boost/process/async.hpp>

#include <unistd.h>

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(codegen)

namespace
{
    struct TestCommunication
    {
        std::string prompt;
        std::string expectedOutput;
    };

    void testRunModule(llvm::Module &module,
                       brainfuck::ObjCodeWriter &writer,
                       TestCommunication const &io,
                       std::string const &tag)
    {
        namespace bp = boost::process;

        auto tempPath = std::filesystem::temp_directory_path();
        auto pidStr = std::to_string(getpid());
        auto tempModule = tempPath / (pidStr + "_" + tag + "_module.o");
        auto tempAout = tempPath / (pidStr + "_" + tag + "_a.out");

        writer.writeModuleToFile(tempModule.string(), module);

        bp::child linkerProcess(bp::search_path("cc"), tempModule.string(), "-o", tempAout.string());
        linkerProcess.wait();

        bp::opstream toChild;
        bp::ipstream fromChild;
        std::ostringstream bufferStream;
        bp::child runnerProcess(tempAout.string(),
                                (bp::std_in < toChild),
                                (bp::std_out > fromChild));

        toChild << io.prompt << std::flush;
        toChild.pipe().close();

        runnerProcess.wait_for(std::chrono::milliseconds(100));
        BOOST_CHECK(!runnerProcess.running());
        runnerProcess.terminate();

        bufferStream << fromChild.rdbuf();

        BOOST_CHECK_EQUAL(io.expectedOutput, bufferStream.str());

        std::error_code ec;

        std::filesystem::remove(tempAout, ec);
        BOOST_CHECK(!ec);
        std::filesystem::remove(tempModule, ec);
        BOOST_CHECK(!ec);
    }
}

BOOST_AUTO_TEST_CASE(helloworld)
{
    std::string source = ">++++++++[<+++++++++>-]<.>++++[<+++++++>-"
                         "]<+.+++++++..+++.>>++++++[<+++++++>-]<++."
                         "------------.>++++++[<+++++++++>-]<+.<.++"
                         "+.------.--------.>>>++++[<++++++++>-]<+.";
    std::istringstream sourceStream(source);

    brainfuck::Lexer lexer(sourceStream);
    auto ast = brainfuck::parse(lexer);

    brainfuck::ObjCodeWriter writer;
    brainfuck::CodeGenerator codegen(writer.getDataLayout());

    codegen(ast);

    auto tsafeModule = codegen.finalizeModule();
    auto module = tsafeModule.getModuleUnlocked();

    TestCommunication io{"", "Hello, World!"};

    testRunModule(*module, writer, io, "hello_unoptimized");
    brainfuck::optimizeModule(*module);
    testRunModule(*module, writer, io, "hello_optimized");
}

BOOST_AUTO_TEST_CASE(rot13)
{
    std::string source = "-,+[-[>>++++[>+++++"
                         "+++<-]<+<-[>+>+>-[>"
                         ">>]<[[>+<-]>>+>]<<<"
                         "<<-]]>>>[-]+>--[-[<"
                         "->+++[-]]]<[+++++++"
                         "+++++<[>-[>+>>]>[+["
                         "<+>-]>+>>]<<<<<-]>>"
                         "[<+>-]>[-[-<<[-]>>]"
                         "<<[<<->>-]>>]<<[<<+"
                         ">>-]]<[-]<.[-]<-,+]";

    std::istringstream sourceStream(source);

    brainfuck::Lexer lexer(sourceStream);
    auto ast = brainfuck::parse(lexer);

    brainfuck::ObjCodeWriter writer;
    brainfuck::CodeGenerator codegen(writer.getDataLayout());

    codegen(ast);

    auto tsafeModule = codegen.finalizeModule();
    auto module = tsafeModule.getModuleUnlocked();

    TestCommunication io{"Hello\n", "Uryyb\n"};

    testRunModule(*module, writer, io, "rot13_unoptimized");
    brainfuck::optimizeModule(*module);
    testRunModule(*module, writer, io, "rot13_optimized");
}

BOOST_AUTO_TEST_SUITE_END()
