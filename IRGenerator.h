#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/BasicBlock.h>

class IRGenerator{
public:
    IRGenerator();
    llvm::Module* module;
    llvm::LLVMContext Context;
    llvm::IRBuilder<> Builder;
    std::map<std::string, llvm::Value*> NamedValues;
    
};

llvm::Value* Cast2Bool(llvm::Value* value, IRGenerator& gen);

// make two values have the same type
bool TypeUpgrading(llvm::Value*& A, llvm::Value*& B, IRGenerator& gen);

// try to convert A from typeA to typeB
llvm::Value* TypeCastTo(llvm::Value* A, llvm::Type* type, IRGenerator& gen);