#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>


class IRGenerator{
public:
    IRGenerator();
    llvm::Module* module;
    llvm::LLVMContext Context;
    llvm::IRBuilder<> Builder;
};
