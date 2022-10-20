#ifndef INCLUDED_LLVM_BRAINFUCK_CODEGEN_HPP
#define INCLUDED_LLVM_BRAINFUCK_CODEGEN_HPP

#include "ast.hpp"

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

#include <memory>

namespace brainfuck
{
    class CodeGenerator
    {
    public:
        CodeGenerator(llvm::DataLayout dataLayout = llvm::DataLayout(""),
                      std::string const &moduleName = "module");

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
        std::unique_ptr<llvm::LLVMContext> llvmContext_;
        std::unique_ptr<llvm::Module> module_;
        std::unique_ptr<llvm::IRBuilder<>> irBuilder_;

        llvm::Value *one_ = nullptr;
        llvm::Value *memsize_ = nullptr;
        llvm::Type *byteType_ = nullptr;
        llvm::Type *intType_ = nullptr;
        llvm::Type *ptrType_ = nullptr;

        llvm::Function *putcharFunc_;
        llvm::Function *getcharFunc_;

        llvm::AllocaInst *posMem_ = nullptr;
        llvm::AllocaInst *globalMem_ = nullptr;
    };
}

#endif
