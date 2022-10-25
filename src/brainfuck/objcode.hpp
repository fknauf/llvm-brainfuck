#ifndef INCLUDED_LLVM_BRAINFUCK_OBJCODE_HPP
#define INCLUDED_LLVM_BRAINFUCK_OBJCODE_HPP

#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

#include <memory>
#include <stdexcept>
#include <string_view>

namespace brainfuck
{
    class ObjCodeWriter
    {
    public:
        ObjCodeWriter(llvm::TargetOptions options = {},
                      llvm::Optional<llvm::Reloc::Model> relocationModel = {},
                      std::string_view cpu = "generic",
                      std::string_view features = "");

        ObjCodeWriter(std::string const &targetTriple,
                      llvm::TargetOptions options = {},
                      llvm::Optional<llvm::Reloc::Model> relocationModel = {},
                      std::string_view cpu = "generic",
                      std::string_view features = "");

        auto getDataLayout() const { return targetMachine_->createDataLayout(); }

        void writeModuleToFile(std::string_view filename,
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
