#include "IRGenerator.h"

IRGenerator::IRGenerator():Builder(llvm::IRBuilder<>(Context)){
    module = new llvm::Module("Program", Context);
}