#include <iostream>
#include "IRGenerator.h"

IRGenerator::IRGenerator():Builder(llvm::IRBuilder<>(Context)){
    module = new llvm::Module("Program", Context);
}

llvm::Value* Cast2Bool(llvm::Value* value, IRGenerator& gen){
    if(value->getType() == llvm::Type::getInt1Ty(gen.Context))
        return value;
    else if(value->getType()->isIntegerTy())
        return gen.Builder.CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0, true));
    else if(value->getType()->isFloatTy())
        return gen.Builder.CreateFCmpONE(value, llvm::ConstantFP::get(value->getType(), 0.0));
    else if(value->getType()->isPointerTy())
        return gen.Builder.CreateICmpNE(gen.Builder.CreatePtrToInt(value, gen.Builder.getInt64Ty()), llvm::ConstantInt::get(llvm::Type::getInt64Ty(gen.Context), 0));
    else{
        std::cout << "[ERROR] : cast unsupport type to bool" << std::endl;
        exit(30);
    }
}

bool TypeUpgrading(llvm::Value*& A, llvm::Value*& B, IRGenerator& gen){
    llvm::Type* typeA = A->getType();
    llvm::Type* typeB = B->getType();
    if(typeA->isIntegerTy() && typeB->isIntegerTy()){
        size_t Bit1 = typeA->getIntegerBitWidth();
        size_t Bit2 = typeB->getIntegerBitWidth();
        if(Bit1 < Bit2)
            A = gen.Builder.CreateIntCast(A, typeB, true);
        else
            B = gen.Builder.CreateIntCast(B, typeA, true);
        return true;
    }
    else if(typeA->isFloatingPointTy() && typeB->isFloatingPointTy()){
        if(typeA->isFloatTy() && typeB->isDoubleTy())
            A = gen.Builder.CreateFPCast(A, typeB);
        else if(typeA->isDoubleTy() && typeB->isFloatTy())
            B = gen.Builder.CreateFPCast(B, A->getType());
        return true;
    }
    else if(typeA->isIntegerTy() && typeB->isFloatingPointTy()){
        A = gen.Builder.CreateSIToFP(A, typeB);
        return true;
    }
    else if(typeA->isFloatingPointTy() && typeB->isIntegerTy()){
        B = gen.Builder.CreateSIToFP(B, typeA);
        return true;
    }
    else return false;
}