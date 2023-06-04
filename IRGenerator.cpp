#include <iostream>
#include "IRGenerator.h"

IRGenerator::IRGenerator():Builder(llvm::IRBuilder<>(Context)), IsInFunction(false){
    module = new llvm::Module("Program", Context);
}

// cast value to bool
llvm::Value* Cast2Bool(llvm::Value* value, IRGenerator& gen){
    if(value->getType() == llvm::Type::getInt1Ty(gen.Context))
        return value;
    else if(value->getType()->isIntegerTy())
        return gen.Builder.CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0, false));
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
        // if there is a bool type among two operand, we should use unsigned casting
        if(Bit1 < Bit2)
            A = gen.Builder.CreateIntCast(A, typeB, Bit1 != 1);
        else
            B = gen.Builder.CreateIntCast(B, typeA, Bit2 != 1);
        return true;
    }
    else if(typeA->isFloatingPointTy() && typeB->isFloatingPointTy()){
        if(typeA->isFloatTy() && typeB->isDoubleTy())
            A = gen.Builder.CreateFPCast(A, typeB);
        else if(typeA->isDoubleTy() && typeB->isFloatTy())
            B = gen.Builder.CreateFPCast(B, typeA);
        return true;
    }
    // careful about the sign, too
    else if(typeA->isIntegerTy() && typeB->isFloatingPointTy()){
        A = typeA->isIntegerTy(1) ? gen.Builder.CreateUIToFP(A, typeB) : gen.Builder.CreateSIToFP(A, typeB);
        return true;
    }
    else if(typeA->isFloatingPointTy() && typeB->isIntegerTy()){
        B = typeB->isIntegerTy(1) ? gen.Builder.CreateUIToFP(B, typeA) : gen.Builder.CreateSIToFP(B, typeA);
        return true;
    }
    else return false;
}

llvm::Value* TypeCastTo(llvm::Value* A, llvm::Type* type, IRGenerator& gen){
    llvm::Type* typeA = A->getType();
    if(typeA == type)
        return A;
    else if(type == llvm::Type::getInt1Ty(gen.Context))
        return Cast2Bool(A, gen);
    else if(typeA->isIntegerTy() && type->isIntegerTy())
        return gen.Builder.CreateIntCast(A, type, !typeA->isIntegerTy(1));
    else if(typeA->isIntegerTy() && type->isFloatingPointTy())
        return typeA->isIntegerTy(1) ? gen.Builder.CreateUIToFP(A, type) : gen.Builder.CreateSIToFP(A, type);
    else if(typeA->isFloatingPointTy() && type->isIntegerTy())
        return gen.Builder.CreateFPToSI(A, type);
    else if(typeA->isFloatingPointTy() && type->isFloatingPointTy())
        return gen.Builder.CreateFPCast(A, type);
    else if(typeA->isPointerTy() && type->isIntegerTy())
        return gen.Builder.CreatePtrToInt(A, type);
    else if(typeA->isIntegerTy() && type->isPointerTy())
        return gen.Builder.CreateIntToPtr(A, type);
    else if(typeA->isPointerTy() && type->isPointerTy())
        return gen.Builder.CreatePointerCast(A, type);
    else return NULL;
}

// when we pass the array type variable, what we want is the address of the first element instead of the whole array
llvm::Value* SuperLoad(llvm::Value* Operand, IRGenerator& gen){
    if(Operand->getType()->getPointerElementType()->isArrayTy())
        return gen.Builder.CreatePointerCast(Operand, Operand->getType()->getPointerElementType()->getArrayElementType()->getPointerTo());
    else
        return gen.Builder.CreateLoad(Operand->getType()->getPointerElementType(), Operand);
}