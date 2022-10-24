#include "codegen.hpp"

#include <llvm/IR/Verifier.h>

#include <algorithm>

namespace brainfuck
{
    namespace
    {
        int const BRAINFUCK_MEMSIZE = 30000;
    }

    CodeGenerator::CodeGenerator(llvm::DataLayout dataLayout,
                                 std::filesystem::path const &sourceFilePath,
                                 bool shouldEmitDebugInfo)
    {
        initLlvmInfrastructure(dataLayout, sourceFilePath, shouldEmitDebugInfo);
        initConstantsAndTypes();
        initDeclareFunctions();
        initMainEntry();
    }

    void CodeGenerator::initLlvmInfrastructure(llvm::DataLayout const &dataLayout, std::filesystem::path const &sourceFilePath, bool shouldEmitDebugInfo)
    {
        llvmContext_ = std::make_unique<llvm::LLVMContext>();

        std::string moduleName = sourceFilePath.empty() ? "anonymousModule" : sourceFilePath.stem();
        module_ = std::make_unique<llvm::Module>(sourceFilePath.stem().string(), *llvmContext_);
        module_->setDataLayout(dataLayout);
        irBuilder_ = std::make_unique<llvm::IRBuilder<>>(*llvmContext_);

        if (shouldEmitDebugInfo)
        {
            module_->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

            std::string sourceFileName = sourceFilePath.empty() ? "<stdin>" : sourceFilePath.filename();
            std::string sourceFileDir = sourceFilePath.empty() ? "." : sourceFilePath.parent_path();

            debugInfoBuilder_ = std::make_unique<llvm::DIBuilder>(*module_);
            debugInfoFile_ = debugInfoBuilder_->createFile(sourceFileName, sourceFileDir);
            debugInfoCompileUnit_ = debugInfoBuilder_->createCompileUnit(llvm::dwarf::DW_LANG_C, debugInfoFile_, "bfcompile", true, "", 0);
        }
    }

    void CodeGenerator::initConstantsAndTypes()
    {
        byteZero_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(8, 0));
        byteOne_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(8, 1));
        memsize_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(64, BRAINFUCK_MEMSIZE));
        ptrIntOne_ = llvm::ConstantInt::get(*llvmContext_, llvm::APInt(module_->getDataLayout().getPointerSizeInBits(), 1));

        byteType_ = llvm::Type::getInt8Ty(*llvmContext_);
        intType_ = llvm::Type::getInt32Ty(*llvmContext_);
        bytePtrType_ = llvm::Type::getInt8PtrTy(*llvmContext_);
        ptrIntType_ = ptrIntOne_->getType();
    }

    void CodeGenerator::initDeclareFunctions()
    {
        llvm::FunctionType *putcharType = llvm::FunctionType::get(intType_, {intType_}, false);
        llvm::FunctionType *getcharType = llvm::FunctionType::get(intType_, false);
        llvm::FunctionType *mainType = llvm::FunctionType::get(intType_, false);

        putcharFunc_ = llvm::Function::Create(putcharType, llvm::Function::ExternalLinkage, "putchar", *module_);
        getcharFunc_ = llvm::Function::Create(getcharType, llvm::Function::ExternalLinkage, "getchar", *module_);
        mainFunc_ = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", *module_);

        if (debugInfoBuilder_)
        {
            auto debugIntType = debugInfoBuilder_->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
            auto debugMainArgv = debugInfoBuilder_->getOrCreateTypeArray({debugIntType});
            auto debugMainType = debugInfoBuilder_->createSubroutineType(debugMainArgv);

            debugMain_ = debugInfoBuilder_->createFunction(debugInfoFile_,
                                                           mainFunc_->getName(),
                                                           llvm::StringRef(),
                                                           debugInfoFile_,
                                                           0,
                                                           debugMainType,
                                                           0,
                                                           llvm::DINode::FlagPrototyped,
                                                           llvm::DISubprogram::SPFlagDefinition);
            mainFunc_->setSubprogram(debugMain_);
            irBuilder_->SetCurrentDebugLocation(llvm::DebugLoc());
        }
    }

    void CodeGenerator::initMainEntry()
    {
        auto entryBlock = llvm::BasicBlock::Create(*llvmContext_, "entry", mainFunc_);
        irBuilder_->SetInsertPoint(entryBlock);

        posMem_ = irBuilder_->CreateAlloca(bytePtrType_, nullptr, "posMem");
        globalMem_ = irBuilder_->CreateAlloca(byteType_, memsize_, "globalMem");
        irBuilder_->CreateIntrinsic(llvm::Intrinsic::memset,
                                    {bytePtrType_, byteType_, intType_, llvm::Type::getInt1Ty(*llvmContext_)},
                                    {globalMem_, byteZero_, memsize_, llvm::ConstantInt::get(*llvmContext_, llvm::APInt(1, 0))});
        irBuilder_->CreateStore(globalMem_, posMem_);

        if (debugInfoBuilder_)
        {
            auto debugByteType = debugInfoBuilder_->createBasicType("unsigned char", 8, llvm::dwarf::DW_ATE_unsigned_char);
            auto debugBytePtrType = debugInfoBuilder_->createPointerType(debugByteType, 64);
            auto memsizeMD = llvm::ConstantAsMetadata::get(llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(*llvmContext_), BRAINFUCK_MEMSIZE));
            auto subscripts = debugInfoBuilder_->getOrCreateSubrange(memsizeMD, nullptr, nullptr, nullptr);
            auto subscriptsArray = debugInfoBuilder_->getOrCreateArray({subscripts});
            auto debugByteArrayType = debugInfoBuilder_->createArrayType(BRAINFUCK_MEMSIZE, 1, debugByteType, subscriptsArray);

            auto debugPos = debugInfoBuilder_->createAutoVariable(debugMain_, "pos", debugInfoFile_, 1, debugBytePtrType, true);
            auto debugMem = debugInfoBuilder_->createAutoVariable(debugMain_, "mem", debugInfoFile_, 1, debugByteArrayType, true);
            auto debugLoc = llvm::DILocation::get(debugMain_->getContext(), 1, 0, debugMain_);

            debugInfoBuilder_->insertDeclare(posMem_, debugPos, debugInfoBuilder_->createExpression(), debugLoc, irBuilder_->GetInsertBlock());
            debugInfoBuilder_->insertDeclare(globalMem_, debugMem, debugInfoBuilder_->createExpression(), debugLoc, irBuilder_->GetInsertBlock());

            irBuilder_->SetCurrentDebugLocation(debugLoc);
        }
    }

    void CodeGenerator::operator()(AST const &ast)
    {
        std::visit(*this, ast);
    }

    void CodeGenerator::operator()(std::vector<AST> const &block)
    {
        std::for_each(begin(block), end(block), std::ref(*this));
    }

    void CodeGenerator::operator()(IncrAST const &ast)
    {
        emitDebugLocation(ast.location());

        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "incrPos");
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue, "incrOld");
        auto newValue = irBuilder_->CreateAdd(oldValue, byteOne_, "incrNew");
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(DecrAST const &ast)
    {
        emitDebugLocation(ast.location());

        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "decrPos");
        auto oldValue = irBuilder_->CreateLoad(byteType_, posValue, "decrOld");
        auto newValue = irBuilder_->CreateSub(oldValue, byteOne_, "decrNew");
        irBuilder_->CreateStore(newValue, posValue);
    }

    void CodeGenerator::operator()(LeftAST const &ast)
    {
        emitDebugLocation(ast.location());

        auto oldPosPtr = irBuilder_->CreateLoad(bytePtrType_, posMem_, "leftOldPtr");
        auto oldPosInt = irBuilder_->CreatePtrToInt(oldPosPtr, ptrIntType_, "leftOldInt");
        auto newPosInt = irBuilder_->CreateSub(oldPosInt, ptrIntOne_, "leftNewInt");
        auto newPosPtr = irBuilder_->CreateIntToPtr(newPosInt, bytePtrType_, "leftNewPtr");
        irBuilder_->CreateStore(newPosPtr, posMem_);
    }

    void CodeGenerator::operator()(RightAST const &ast)
    {
        emitDebugLocation(ast.location());

        auto oldPosPtr = irBuilder_->CreateLoad(bytePtrType_, posMem_, "rightOldPtr");
        auto oldPosInt = irBuilder_->CreatePtrToInt(oldPosPtr, ptrIntType_, "rightOldInt");
        auto newPosInt = irBuilder_->CreateAdd(oldPosInt, ptrIntOne_, "rightNewInt");
        auto newPosPtr = irBuilder_->CreateIntToPtr(newPosInt, bytePtrType_, "rightNewPtr");
        irBuilder_->CreateStore(newPosPtr, posMem_);
    }

    void CodeGenerator::operator()(WriteAST const &ast)
    {
        emitDebugLocation(ast.location());

        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "writePos");
        auto dataValue = irBuilder_->CreateLoad(byteType_, posValue, "writeVal");
        auto dataInt = irBuilder_->CreateCast(llvm::CastInst::ZExt, dataValue, intType_, "writeCast");
        irBuilder_->CreateCall(putcharFunc_, dataInt, "writeCall");
    }

    void CodeGenerator::operator()(ReadAST const &ast)
    {
        emitDebugLocation(ast.location());

        auto readValue = irBuilder_->CreateCall(getcharFunc_, llvm::None, "readCall");
        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "readPos");
        irBuilder_->CreateStore(readValue, posValue);
    }

    void CodeGenerator::operator()(LoopAST const &ast)
    {
        emitDebugLocation(ast.location());

        auto headBB = llvm::BasicBlock::Create(*llvmContext_, "headBlock", mainFunc_);
        auto bodyBB = llvm::BasicBlock::Create(*llvmContext_, "bodyBlock", mainFunc_);
        auto afterBB = llvm::BasicBlock::Create(*llvmContext_, "afterBlock");

        irBuilder_->CreateBr(headBB);
        irBuilder_->SetInsertPoint(headBB);

        auto posValue = irBuilder_->CreateLoad(bytePtrType_, posMem_, "loopPos");
        auto dataValue = irBuilder_->CreateLoad(byteType_, posValue, "loopVal");
        auto loopCondition = irBuilder_->CreateICmpEQ(dataValue, byteZero_, "loopCond");

        irBuilder_->CreateCondBr(loopCondition, afterBB, bodyBB);
        irBuilder_->SetInsertPoint(bodyBB);

        (*this)(ast.loopBody());

        irBuilder_->CreateBr(headBB);

        mainFunc_->getBasicBlockList().push_back(afterBB);
        irBuilder_->SetInsertPoint(afterBB);
    }

    llvm::orc::ThreadSafeModule CodeGenerator::finalizeModule()
    {
        irBuilder_->CreateRet(llvm::ConstantInt::get(*llvmContext_, llvm::APInt(32, 0)));
        if (debugInfoBuilder_)
        {
            debugInfoBuilder_->finalize();
        }

        llvm::verifyFunction(*mainFunc_);
        llvm::verifyModule(*module_);

        return {std::move(module_), std::move(llvmContext_)};
    }

    void CodeGenerator::emitDebugLocation(SourceLocation const &loc)
    {
        if (debugInfoBuilder_)
        {
            auto debugLoc = llvm::DILocation::get(debugMain_->getContext(), loc.line(), loc.column(), debugMain_);
            irBuilder_->SetCurrentDebugLocation(debugLoc);
        }
    }
}
