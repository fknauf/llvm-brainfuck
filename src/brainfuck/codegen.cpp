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

        zero_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(8, 0));
        one_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(8, 1));
        memsize_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(32, 30000));
        byteType_ = llvm::Type::getInt8Ty(*llvmContext_);
        intType_ = llvm::Type::getInt32Ty(*llvmContext_);
        ptrType_ = llvm::Type::getInt8PtrTy(*llvmContext_);

        llvm::FunctionType *putcharType = llvm::FunctionType::get(intType_, {intType_}, false);
        llvm::FunctionType *getcharType = llvm::FunctionType::get(intType_, false);
        llvm::FunctionType *mainType = llvm::FunctionType::get(llvm::Type::getVoidTy(*llvmContext_), false);

        putcharFunc_ = llvm::Function::Create(putcharType, llvm::Function::ExternalLinkage, "putchar", *module_);
        getcharFunc_ = llvm::Function::Create(getcharType, llvm::Function::ExternalLinkage, "getchar", *module_);
        mainFunc_ = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", *module_);

        auto entryBlock = llvm::BasicBlock::Create(*llvmContext_, "entry", mainFunc_);
        irBuilder_->SetInsertPoint(entryBlock);

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
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_, "incrPos");
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue, "incrVal");
        auto newValue = irBuilder_->CreateAdd(oldValue, one_, "incrAdd");
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(DecrAST const &)
    {
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_, "decrPos");
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue, "decrVal");
        auto newValue = irBuilder_->CreateSub(oldValue, one_, "decrSub");
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(LeftAST const &)
    {
        auto oldPosValue = irBuilder_->CreateLoad(ptrType_, posMem_, "leftPos");
        auto newPosValue = irBuilder_->CreateSub(oldPosValue, one_, "leftMove");
        irBuilder_->CreateStore(newPosValue, posMem_);
    }

    void CodeGenerator::operator()(RightAST const &)
    {
        auto oldPosValue = irBuilder_->CreateLoad(ptrType_, posMem_, "rightPos");
        auto newPosValue = irBuilder_->CreateAdd(oldPosValue, one_, "rightMove");
        irBuilder_->CreateStore(newPosValue, posMem_);
    }

    void CodeGenerator::operator()(WriteAST const &)
    {
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_, "writePos");
        auto dataValue = irBuilder_->CreateLoad(byteType_, posValue, "writeVal");
        auto dataInt = irBuilder_->CreateCast(llvm::CastInst::ZExt, dataValue, intType_, "writeCast");
        irBuilder_->CreateCall(putcharFunc_, dataInt, "writeCall");
    }

    void CodeGenerator::operator()(ReadAST const &)
    {
        auto readValue = irBuilder_->CreateCall(getcharFunc_, llvm::None, "readCall");
        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_, "readPos");
        irBuilder_->CreateStore(readValue, posValue);
    }

    void CodeGenerator::operator()(LoopAST const &ast)
    {
        auto headBB = llvm::BasicBlock::Create(*llvmContext_, "headBlock", mainFunc_);
        auto bodyBB = llvm::BasicBlock::Create(*llvmContext_, "bodyBlock", mainFunc_);
        auto afterBB = llvm::BasicBlock::Create(*llvmContext_, "contBlock", mainFunc_);

        irBuilder_->CreateBr(headBB);
        irBuilder_->SetInsertPoint(headBB);

        auto posValue = irBuilder_->CreateLoad(ptrType_, posMem_, "loopPos");
        auto dataValue = irBuilder_->CreateLoad(byteType_, posValue, "loopData");
        auto loopCondition = irBuilder_->CreateICmpEQ(dataValue, zero_, "loopCond");

        irBuilder_->CreateCondBr(loopCondition, afterBB, bodyBB);
        irBuilder_->SetInsertPoint(bodyBB);

        (*this)(ast.loopBody());

        irBuilder_->CreateBr(headBB);
        irBuilder_->SetInsertPoint(afterBB);
    }

    llvm::orc::ThreadSafeModule CodeGenerator::finalizeModule()
    {
        irBuilder_->CreateRetVoid();

        return {std::move(module_), std::move(llvmContext_)};
    }
}