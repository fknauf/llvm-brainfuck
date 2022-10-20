#include "objcode.hpp"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>

namespace brainfuck
{
    ObjCodeWriter::ObjCodeWriter(llvm::TargetOptions options,
                                 llvm::Optional<llvm::Reloc::Model> relocationModel,
                                 std::string const &cpu,
                                 std::string const &features)
        : ObjCodeWriter(llvm::sys::getDefaultTargetTriple(), options, relocationModel, cpu, features)
    {
    }

    ObjCodeWriter::ObjCodeWriter(std::string const &targetTriple,
                                 llvm::TargetOptions options,
                                 llvm::Optional<llvm::Reloc::Model> relocationModel,
                                 std::string const &cpu,
                                 std::string const &features)
    {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        std::string errMsg;
        auto target = llvm::TargetRegistry::lookupTarget(targetTriple, errMsg);

        if (target == nullptr)
        {
            throw std::runtime_error(errMsg);
        }

        targetMachine_.reset(target->createTargetMachine(targetTriple, cpu, features, options, relocationModel));

        if (targetMachine_.get() == nullptr)
        {
            throw std::runtime_error("could not create target machine description");
        }
    }

    void ObjCodeWriter::writeModuleToFile(std::string const &fileName,
                                          llvm::Module &module,
                                          llvm::CodeGenFileType fileType)
    {
        std::error_code ec;
        llvm::raw_fd_ostream dest(fileName, ec);

        if (ec)
        {
            throw std::runtime_error("Couldn't open file " + fileName);
        }

        writeModuleToStream(dest, module, fileType);
    }

    void ObjCodeWriter::writeModuleToStream(llvm::raw_pwrite_stream &dest,
                                            llvm::Module &module,
                                            llvm::CodeGenFileType fileType)
    {
        llvm::legacy::PassManager passManager;

        if (targetMachine_->addPassesToEmitFile(passManager, dest, nullptr, fileType))
        {
            throw std::runtime_error("Target Machine can't emit a file of this type");
        }

        module.setDataLayout(targetMachine_->createDataLayout());
        passManager.run(module);
        dest.flush();
    }
}
