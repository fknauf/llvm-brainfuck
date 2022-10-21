#include "codegen.hpp"

#include <llvm/IR/Verifier.h>

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

        byteZero_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(8, 0));
        byteOne_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(8, 1));
        memsize_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(32, 30000));
        ptrIntOne_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(dataLayout.getPointerSizeInBits(), 1));

        byteType_ = llvm::Type::getInt8Ty(*llvmContext_);
        intType_ = llvm::Type::getInt32Ty(*llvmContext_);
        bytePtrType_ = llvm::Type::getInt8PtrTy(*llvmContext_);
        ptrIntType_ = ptrIntOne_->getType();

        llvm::FunctionType *putcharType = llvm::FunctionType::get(intType_, {intType_}, false);
        llvm::FunctionType *getcharType = llvm::FunctionType::get(intType_, false);
        llvm::FunctionType *mainType = llvm::FunctionType::get(intType_, false);

        putcharFunc_ = llvm::Function::Create(putcharType, llvm::Function::ExternalLinkage, "putchar", *module_);
        getcharFunc_ = llvm::Function::Create(getcharType, llvm::Function::ExternalLinkage, "getchar", *module_);
        mainFunc_ = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", *module_);

        auto entryBlock = llvm::BasicBlock::Create(*llvmContext_, "entry", mainFunc_);
        irBuilder_->SetInsertPoint(entryBlock);

        posMem_ = irBuilder_->CreateAlloca(bytePtrType_, nullptr, "posMem");
        globalMem_ = irBuilder_->CreateAlloca(byteType_, memsize_, "globalMem");
        irBuilder_->CreateIntrinsic(llvm::Intrinsic::memset,
                                    {bytePtrType_, byteType_, intType_, llvm::Type::getInt1Ty(*llvmContext_)},
                                    {globalMem_, byteZero_, memsize_, llvm::ConstantInt::get(*llvmContext_, llvm::APInt(1, 0))});

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
        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "incrPos");
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue, "incrOld");
        auto newValue = irBuilder_->CreateAdd(oldValue, byteOne_, "incrNew");
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(DecrAST const &)
    {
        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "decrPos");
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue, "decrOld");
        auto newValue = irBuilder_->CreateSub(oldValue, byteOne_, "decrNew");
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(LeftAST const &)
    {
        auto oldPosPtr = irBuilder_->CreateLoad(bytePtrType_, posMem_, "leftOldPtr");
        auto oldPosInt = irBuilder_->CreatePtrToInt(oldPosPtr, ptrIntType_, "leftOldInt");
        auto newPosInt = irBuilder_->CreateSub(oldPosInt, ptrIntOne_, "leftNewInt");
        auto newPosPtr = irBuilder_->CreateIntToPtr(newPosInt, bytePtrType_, "leftNewPtr");
        irBuilder_->CreateStore(newPosPtr, posMem_);
    }

    void CodeGenerator::operator()(RightAST const &)
    {
        auto oldPosPtr = irBuilder_->CreateLoad(bytePtrType_, posMem_, "rightOldPtr");
        auto oldPosInt = irBuilder_->CreatePtrToInt(oldPosPtr, ptrIntType_, "rightOldInt");
        auto newPosInt = irBuilder_->CreateAdd(oldPosInt, ptrIntOne_, "rightNewInt");
        auto newPosPtr = irBuilder_->CreateIntToPtr(newPosInt, bytePtrType_, "rightNewPtr");
        irBuilder_->CreateStore(newPosPtr, posMem_);
    }

    void CodeGenerator::operator()(WriteAST const &)
    {
        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "writePos");
        auto dataValue = irBuilder_->CreateLoad(byteType_, posValue, "writeVal");
        auto dataInt = irBuilder_->CreateCast(llvm::CastInst::ZExt, dataValue, intType_, "writeCast");
        irBuilder_->CreateCall(putcharFunc_, dataInt, "writeCall");
    }

    void CodeGenerator::operator()(ReadAST const &)
    {
        auto readValue = irBuilder_->CreateCall(getcharFunc_, llvm::None, "readCall");
        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "readPos");
        irBuilder_->CreateStore(readValue, posValue);
    }

    void CodeGenerator::operator()(LoopAST const &ast)
    {
        auto headBB = llvm::BasicBlock::Create(*llvmContext_, "headBlock", mainFunc_);
        auto bodyBB = llvm::BasicBlock::Create(*llvmContext_, "bodyBlock", mainFunc_);
        auto afterBB = llvm::BasicBlock::Create(*llvmContext_, "afterBlock", mainFunc_);

        irBuilder_->CreateBr(headBB);
        irBuilder_->SetInsertPoint(headBB);

        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "loopPos");
        auto dataValue = irBuilder_->CreateLoad(byteType_, posValue, "loopVal");
        auto loopCondition = irBuilder_->CreateICmpEQ(dataValue, byteZero_, "loopCond");

        irBuilder_->CreateCondBr(loopCondition, afterBB, bodyBB);
        irBuilder_->SetInsertPoint(bodyBB);

        (*this)(ast.loopBody());

        irBuilder_->CreateBr(headBB);
        irBuilder_->SetInsertPoint(afterBB);
    }

    llvm::orc::ThreadSafeModule CodeGenerator::finalizeModule()
    {
        irBuilder_->CreateRet(llvm::ConstantInt::get(*llvmContext_, llvm::APInt(32, 0)));

        llvm::verifyFunction(*mainFunc_);
        llvm::verifyModule(*module_);

        return {std::move(module_), std::move(llvmContext_)};
    }
}