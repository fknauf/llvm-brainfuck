#ifndef INCLUDED_LLVM_BRAINFUCK_OPTIMIZER_HPP
#define INCLUDED_LLVM_BRAINFUCK_OPTIMIZER_HPP

#include <llvm/IR/Module.h>

namespace brainfuck
{
    void optimizeModule(llvm::Module &module);
}

#endif
