#ifndef INCLUDED_LLVM_BRAINFUCK_OBJCODE_HPP
#define INCLUDED_LLVM_BRAINFUCK_OBJCODE_HPP

#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

#include <memory>
#include <stdexcept>

namespace brainfuck
{
    class ObjCodeWriter
    {
    public:
        ObjCodeWriter(llvm::TargetOptions options = {},
                      llvm::Optional<llvm::Reloc::Model> relocationModel = {},
                      std::string const &cpu = "generic",
                      std::string const &features = "");

        ObjCodeWriter(std::string const &targetTriple,
                      llvm::TargetOptions options = {},
                      llvm::Optional<llvm::Reloc::Model> relocationModel = {},
                      std::string const &cpu = "generic",
                      std::string const &features = "");

        auto getDataLayout() const { return targetMachine_->createDataLayout(); }

        void writeModuleToFile(std::string const &filename,
                               llvm::Module &module,
                               llvm::CodeGenFileType fileType = llvm::CGFT_ObjectFile);

        void writeModuleToStream(llvm::raw_pwrite_stream &dest,
                                 llvm::Module &module,
                                 llvm::CodeGenFileType fileType = llvm::CGFT_ObjectFile);

    private:
        std::unique_ptr<llvm::TargetMachine> targetMachine_;
    };
}

#endif
