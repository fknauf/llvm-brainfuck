#include "codegen.hpp"

#include <algorithm>

namespace brainfuck
{

    CodeGenerator::CodeGenerator(llvm::DataLayout dataLayout,
                                 std::string const &moduleName)
    {
        llvmContext_ = std::make_unique<llvm::LLVMContext>();
        module_ = std::make_unique<llvm::Module>(moduleName, *llvmContext_);
        module_->setDataLayout(dataLayout);
        irBuilder_ = std::make_unique<llvm::IRBuilder<>>(*llvmContext_);

        one_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(8, 1));
        memsize_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(32, 30000));
        byteType_ = llvm::Type::getInt8Ty(*llvmContext_);
        intType_ = llvm::Type::getInt32Ty(*llvmContext_);
        ptrType_ = llvm::Type::getInt8PtrTy(*llvmContext_);

        llvm::FunctionType *putcharType = llvm::FunctionType::get(intType_, {intType_}, false);
        llvm::FunctionType *getcharType = llvm::FunctionType::get(intType_, false);

        putcharFunc_ = llvm::Function::Create(putcharType, llvm::Function::ExternalLinkage, "putchar", *module_);
        getcharFunc_ = llvm::Function::Create(getcharType, llvm::Function::ExternalLinkage, "getchar", *module_);

        posMem_ = irBuilder_->CreateAlloca(ptrType_, nullptr, "position");
        globalMem_ = irBuilder_->CreateAlloca(byteType_, memsize_, "memory");

        irBuilder_->CreateStore(globalMem_, posMem_);
    }

    void CodeGenerator::operator()(AST const &ast)
    {
        std::visit(*this, ast);
    }

    void CodeGenerator::operator()(std::vector<AST> const &block)
    {
        std::for_each(begin(block), end(block), std::ref(*this));
    }

    void CodeGenerator::operator()(IncrAST const &)
    {
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_);
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue);
        auto newValue = irBuilder_->CreateAdd(oldValue, one_);
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(DecrAST const &)
    {
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_);
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue);
        auto newValue = irBuilder_->CreateSub(oldValue, one_);
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(LeftAST const &)
    {
        auto oldPosValue = irBuilder_->CreateLoad(ptrType_, posMem_);
        auto newPosValue = irBuilder_->CreateSub(oldPosValue, one_);
        irBuilder_->CreateStore(newPosValue, posMem_);
    }

    void CodeGenerator::operator()(RightAST const &)
    {
        auto oldPosValue = irBuilder_->CreateLoad(ptrType_, posMem_);
        auto newPosValue = irBuilder_->CreateAdd(oldPosValue, one_);
        irBuilder_->CreateStore(newPosValue, posMem_);
    }

    void CodeGenerator::operator()(WriteAST const &)
    {
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_);
        auto dataValue = irBuilder_->CreateLoad(byteType_, posValue);
        auto dataInt = irBuilder_->CreateCast(llvm::CastInst::ZExt, dataValue, intType_);
        irBuilder_->CreateCall(putcharFunc_, dataInt);
    }

    void CodeGenerator::operator()(ReadAST const &)
    {
        auto readValue = irBuilder_->CreateCall(getcharFunc_);
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_);
        irBuilder_->CreateStore(readValue, posValue);
    }

    void CodeGenerator::operator()(LoopAST const &)
    {
    }

    llvm::orc::ThreadSafeModule CodeGenerator::finalizeModule()
    {
        return {std::move(module_), std::move(llvmContext_)};
    }
}