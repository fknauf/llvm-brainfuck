#ifndef INCLUDED_LLVM_BRAINFUCK_CODEGEN_HPP
#define INCLUDED_LLVM_BRAINFUCK_CODEGEN_HPP

#include "ast.hpp"

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

#include <filesystem>
#include <memory>

namespace brainfuck
{
    class CodeGenerator
    {
    public:
        CodeGenerator(llvm::DataLayout dataLayout = llvm::DataLayout(""),
                      std::filesystem::path const &sourceFilePath = {},
                      bool shouldEmitDebugInfo_ = false);

        void operator()(AST const &ast);
        void operator()(std::vector<AST> const &block);
        void operator()(IncrAST const &);
        void operator()(DecrAST const &);
        void operator()(LeftAST const &);
        void operator()(RightAST const &);
        void operator()(WriteAST const &);
        void operator()(ReadAST const &);
        void operator()(LoopAST const &loop);

        llvm::orc::ThreadSafeModule finalizeModule();

    private:
        void emitDebugLocation(SourceLocation const &loc);

        std::unique_ptr<llvm::LLVMContext> llvmContext_;
        std::unique_ptr<llvm::Module> module_;
        std::unique_ptr<llvm::IRBuilder<>> irBuilder_;
        std::unique_ptr<llvm::DIBuilder> debugInfoBuilder_;

        llvm::DIFile *debugInfoFile_ = nullptr;
        llvm::DICompileUnit *debugInfoCompileUnit_ = nullptr;

        llvm::Value *byteZero_ = nullptr;
        llvm::Value *byteOne_ = nullptr;
        llvm::Value *memsize_ = nullptr;
        llvm::Value *ptrIntOne_ = nullptr;

        llvm::Type *byteType_ = nullptr;
        llvm::Type *intType_ = nullptr;
        llvm::Type *bytePtrType_ = nullptr;
        llvm::Type *ptrIntType_ = nullptr;

        llvm::Function *putcharFunc_;
        llvm::Function *getcharFunc_;
        llvm::Function *mainFunc_;

        llvm::DISubprogram *debugMain_;

        llvm::AllocaInst *posMem_ = nullptr;
        llvm::AllocaInst *globalMem_ = nullptr;
    };
}

#endif
