#include "AST.h"
#define VISIBLE 1

// print error message in unified format
void ErrorMessage(std::string Message, int ExitCode){
    std::cout << "[ERROR] : " + Message << std::endl;
    exit(ExitCode); 
}

std::string Node::graphVizRelation;
int Node::nodeCount = 1;

void Node::setNodeName(std::string name){
    mNodeName = name;
}

Program::Program(std::vector<Definition*>* _Definitions):Definitions(_Definitions){
    if(VISIBLE){
        for(auto i : *Definitions)
            addChildren(i);
    }
    setNodeName("Program");
}

llvm::Value* Program::CodeGen(IRGenerator& gen){
    for(auto i : *Definitions)
        i->CodeGen(gen);
    return NULL;
}

FunName::FunName(std::string* _Name, int _PointerDim):Name(_Name),PointerDim(_PointerDim){
    if(VISIBLE)
        setNodeName("FunName : " + *_Name);
}

std::string* FunName::GetName(){
    return Name;
}

int FunName::GetPointerDim(){
    return PointerDim;
}

void Star::Add(){
    Dim++;
}

int Star::GetDim(){
    return Dim;
}

FunDefinition::FunDefinition(VarType* _ReturnType, FunName* _FunNam, ArgList* _Args, Block* _Statements):ReturnType(_ReturnType),Name(_FunNam),Args(_Args),Statements(_Statements){
    // set type of each formal parameters in function
    Args->SetTypeForVar();

    if(VISIBLE){
        addChildren(ReturnType);
        addChildren(Name);
        addChildren(Args);
        Node* VNode = new Node();
        if(_Statements!=NULL){
            VNode->setNodeName("FunBody");
            addChildren(VNode);
            addChildren(_Statements);
        }
        setNodeName("FunDefinition");
    }
}

llvm::Value* FunDefinition::CodeGen(IRGenerator& gen){
    std::vector<llvm::Type*> ArgsType;
    // in C, when there is an array type parameter, only a pointer pointing to its elements will be passed
    // however, we make no distinction between types
    Args->PushType(ArgsType, gen);
    // wrap the basic type with pointer
    llvm::Type* basic = ReturnType->GetType(gen);
    for(int i = 0; i < Name->GetPointerDim(); i++)
        basic = llvm::PointerType::get(basic, 0);
    // create function type
    llvm::FunctionType* FunType = llvm::FunctionType::get(basic, ArgsType, false);
    llvm::Function* fun = llvm::Function::Create(FunType, llvm::Function::ExternalLinkage, *Name->GetName(), gen.module);
    // if there is a confliction, the latter will be renamed, so we can check confliction by it
    if(fun->getName() != *Name->GetName()){
        //delete the function we declared just now, and get the former declared function
        fun->eraseFromParent();
        fun = gen.module->getFunction(*Name->GetName());
    }
    if(Statements != NULL){
        if(!fun->empty())
            ErrorMessage("redefinition of function", 2);
        if(fun->arg_size() != ArgsType.size())
            ErrorMessage("redefinition of function with different args", 93);
        int idx = 0;
        gen.NamedValues.clear();
        for(llvm::Function::arg_iterator AI = fun->arg_begin(); idx != ArgsType.size(); idx++, AI++){
            AI->setName(Args->GetNameByIndex(idx));
            gen.NamedValues[Args->GetNameByIndex(idx)] = AI;
        }
        llvm::BasicBlock* BB = llvm::BasicBlock::Create(gen.Context, "entry", fun);
        gen.Builder.SetInsertPoint(BB);
        if(llvm::Value* RetVal = Statements->CodeGen(gen)){
            // Validate the generated code, checking for consistency.
            llvm::verifyFunction(*fun);
            return fun;
        }
        // Error reading body, remove function. Add this code when CodeGen of return statment complete
        //fun->eraseFromParent();
        return nullptr;
    }
    return NULL;
}

VarDefinition::VarDefinition(VarType* _Type, std::vector<Var*>* _List):Type(_Type),List(_List){
    if(VISIBLE){
        addChildren(Type);
        Node* VNode = new Node();
        VNode->setNodeName("VarList");
        for(auto i : *List)
            VNode->addChildren(i);
        addChildren(VNode);
        setNodeName("VarDefinition");
    }
    for(auto i : *List)
        i->SetType(Type);
}

void VarDefinition::PushType(std::vector<llvm::Type*>& elements, IRGenerator& gen){
    for(auto i : *List)
        elements.push_back(i->GetType(gen));
}

llvm::Value* VarDefinition::CodeGen(IRGenerator& gen){
    for(auto i : *List){
        llvm::Type* RealType = i->GetType(gen);
        // function CreateAlloca always return a PointerTy instead of RealType
        llvm::Value* temp = gen.Builder.CreateAlloca(RealType);
        auto iter = gen.NamedValues.find(i->GetName());
        if(iter != gen.NamedValues.end())
            ErrorMessage("redefinition of variable", 3);
        gen.NamedValues[i->GetName()] = temp;
    }
    return NULL;
}

ArgList::ArgList(){
    if(VISIBLE)
        setNodeName("ArgList");
}

void ArgList::Add(VarType* _Type, Var* _Name){
    Types.push_back(_Type);
    Names.push_back(_Name);
    if(VISIBLE){
        Node* VNode = new Node();
        VNode->addChildren(_Type);
        VNode->addChildren(_Name);
        VNode->setNodeName("Arg");
        addChildren(VNode);
    }
}

void ArgList::SetTypeForVar(){
    int len = Types.size();
    if(len != Names.size())
        ErrorMessage("ArgList has inconsistent size", 162);
    for(int i = 0; i < len; i++){
        Names[i]->SetType(Types[i]);
    }
}

void ArgList::PushType(std::vector<llvm::Type*>& elements, IRGenerator& gen){
    for(auto i : Names)
        elements.push_back(i->GetType(gen));
}

std::string ArgList::GetNameByIndex(int idx){
    return Names[idx]->GetName();
}

Var::Var(std::string* _Name):PointerDim(0),Name(_Name),ASTType(NULL),LLVMType(NULL){
    if(VISIBLE)
        setNodeName(*Name);
}

void Var::SetPointer(int dim){
    PointerDim = dim;
    if(VISIBLE){
        std::string star="";
        for(int i = 0; i < dim; i++)
            star += "*";
        setNodeName(star+getNodeName());
    }
}

void Var::AddArray(int size){
    ArrayDim.push_back(size);
    if(VISIBLE)
        setNodeName(getNodeName()+"[]");
}

void Var::SetType(VarType* _Type){
    ASTType = _Type;
}

llvm::Type* Var::GetType(IRGenerator& gen){
    if(LLVMType != NULL)
        return LLVMType;
    llvm::Type* basic = ASTType->GetType(gen);
    for(int i = 0; i < PointerDim; i++)
        // I don't know the exactly usage of the second parameter,
        // if something bad happens, we should check here
        basic = llvm::PointerType::get(basic, 0);
    for(int i = 0; i < ArrayDim.size(); i++)
        basic = llvm::ArrayType::get(basic, ArrayDim[i]);
    LLVMType = basic;
    return LLVMType;
}

std::string Var::GetName(){
    return *Name;
}

BuildInType::BuildInType(enum TypeIndex _Index):Index(_Index){
    if(VISIBLE){
        switch(Index){
            case _INT_: setNodeName("INT");break;
            case _SHORT_: setNodeName("SHORT");break;
            case _LONG_: setNodeName("LONG");break;
            case _FLOAT_: setNodeName("FLOAT");break;
            case _DOUBLE_: setNodeName("DOUBLE");break;
            case _CHAR_: setNodeName("CHAR");break;
            case _BOOL_: setNodeName("BOOL");break;
            case _VOID_: setNodeName("VOID");break;
        }
    }
}

llvm::Type* BuildInType::GetType(IRGenerator& gen){
    if(Type != NULL)
        return Type;
    switch(Index){
        case _INT_: Type = llvm::Type::getInt32Ty(gen.Context);break;
        case _SHORT_: Type = llvm::Type::getInt16Ty(gen.Context);break;
        case _LONG_: Type = llvm::Type::getInt64Ty(gen.Context);break;
        case _FLOAT_: Type = llvm::Type::getFloatTy(gen.Context);break;
        case _DOUBLE_: Type = llvm::Type::getDoubleTy(gen.Context);break;
        case _CHAR_: Type = llvm::Type::getInt8Ty(gen.Context);break;
        case _BOOL_: Type = llvm::Type::getInt1Ty(gen.Context);break;
        case _VOID_: Type = llvm::Type::getVoidTy(gen.Context);break;
    }
    return Type;
}

StructType::StructType(){
    if(VISIBLE)
        setNodeName("StructType : ");
}

void StructType::Add(VarDefinition* _VarDefinition){
    VarDefinitions.push_back(_VarDefinition);
    if(VISIBLE)
        addChildren(_VarDefinition);
}

llvm::Type* StructType::GetType(IRGenerator& gen){
    if(Type != NULL)
        return Type;
    // determine if the name alread exists
    // this step helps when struct declaration and define is seperate
    llvm::StructType* ST = gen.module->getTypeByName(*Name);
    if(!ST)
        ST = llvm::StructType::create(gen.Context, *Name);
    std::vector<llvm::Type*> elements;
    for(auto i : VarDefinitions)
        i->PushType(elements, gen);
    ST->setBody(elements);
    Type = ST;
    return Type;
}

void StructType::SetName(std::string* _Name){
    Name = _Name;
    if(VISIBLE)
        setNodeName("StructType : " + *_Name);
}

EnumDefinition::EnumDefinition(std::string* _Name, int _Value):Name(_Name),Value(_Value){
    if(VISIBLE)
        setNodeName(*_Name);
}

void EnumType::Add(EnumDefinition* _EnumDefinition){
    EnumDefinitions.push_back(_EnumDefinition);
    if(VISIBLE)
        addChildren(_EnumDefinition);
}

Block::Block(std::vector<Statement*>* _statements):statements(_statements){
    if(VISIBLE){
        setNodeName("Block");
        for(auto i : *statements)
            addChildren(i);
    }
}

llvm::Value* Block::CodeGen(IRGenerator& gen){
    llvm::Value *RetVal, *StaVal;
    // There is a little problem here, when multiple return statements exist, how to decide for returning value
    for(auto i : *statements){
        StaVal = i->CodeGen(gen);
        // if(StaVal)
        //     return StaVal;
    }
    return NULL;
}

IfStatement::IfStatement(Expression* _Condition, Statement* _True, Statement* _False):Condition(_Condition),True(_True),False(_False){
    if(VISIBLE){
        setNodeName("IF");
        addChildren(Condition);
        addChildren(True);
        if(False != NULL)
            addChildren(False);
    }
}

WhileStatement::WhileStatement(Expression* _Condition, Statement* _Loop):Condition(_Condition),Loop(_Loop){
    if(VISIBLE){
        setNodeName("While");
        addChildren(Condition);
        addChildren(Loop);
    }
}

DoWhileStatement::DoWhileStatement(Expression* _Condition, Statement* _Loop):Condition(_Condition),Loop(_Loop){
    if(VISIBLE){
        setNodeName("DoWhile");
        addChildren(Loop);
        addChildren(Condition);
    }
}

ForStatement::ForStatement(Expression* _Initialization, Expression* _Condition, Statement* _Collection, Statement* _Loop):Initialization(_Initialization), Condition(_Condition),Collection(_Collection),Loop(_Loop){
    if(VISIBLE){
        setNodeName("For");
        addChildren(Initialization);
        addChildren(Condition);
        addChildren(Collection);
        addChildren(Loop);
    }
}

ReturnStatement::ReturnStatement(Expression* _ReturnValue): ReturnValue(_ReturnValue){
    if(VISIBLE){
        setNodeName("return");
        addChildren(ReturnValue);
    }    
}

GetItem::GetItem(Expression* _Array, Expression* _Index):Array(_Array), Index(_Index){
    if(VISIBLE){
        setNodeName("GetItem");
        addChildren(Array);
        addChildren(Index);
    }
}

llvm::Value* GetItem::CodeGen(IRGenerator& gen){
    return SuperLoad(CodeGenPtr(gen), gen);
}

llvm::Value* GetItem::CodeGenPtr(IRGenerator& gen){
    llvm::Value* ArrayPtr = Array->CodeGen(gen);
    if(!ArrayPtr->getType()->isPointerTy())
        ErrorMessage("a[], a must be a pointers or arrays", 375);
    llvm::Value* Idx = Index->CodeGen(gen);
    if(!Idx->getType()->isIntegerTy())
        ErrorMessage("a[idx], idx must be an integer", 379);
    return gen.Builder.CreateGEP(ArrayPtr->getType()->getPointerElementType(), ArrayPtr, Idx);
}

FunctionCall::FunctionCall(std::string* _FunName, std::vector<Expression*>* _Args):FunName(_FunName),Args(_Args){
    if(VISIBLE){
        setNodeName("FunCall : " + *FunName);
        for(auto i : *Args)
            addChildren(i);
    }
}

llvm::Value* FunctionCall::CodeGen(IRGenerator& gen){
    llvm::Function *Call = gen.module->getFunction(*FunName);
    if(Call == NULL)
        ErrorMessage("unknown function", 393);
    if(Call->arg_size() != Args->size())
        ErrorMessage("unmatched args", 395);
    
    std::vector<llvm::Value*> _Args;
    for(int i = 0; i < Call->arg_size(); i++){
        _Args.push_back((*Args)[i]->CodeGen(gen));
    }
    return gen.Builder.CreateCall(Call, _Args, *FunName);
}

llvm::Value* FunctionCall::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Function calling was used as left value", 396);
    return NULL;
}

Component::Component(Expression* _Structure, std::string* _ComponentName):Structure(_Structure), ComponentName(_ComponentName){
    if(VISIBLE){
        setNodeName("Component : " + *ComponentName);
        addChildren(Structure);
    }
}

llvm::Value* Component::CodeGen(IRGenerator& gen){
    llvm::Value* result = CodeGenPtr(gen);
    return gen.Builder.CreateLoad(result->getType()->getPointerElementType(), result);
}

llvm::Value* Component::CodeGenPtr(IRGenerator& gen){
    
}

PtrComponent::PtrComponent(Expression* _PtrStructure, std::string* _ComponentName):PtrStructure(_PtrStructure), ComponentName(_ComponentName){
    if(VISIBLE){
        setNodeName("PtrComponent : " + *ComponentName);
        addChildren(PtrStructure);
    }
}

PositiveSign::PositiveSign(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Positive");
        addChildren(Operand);
    }
}

llvm::Value* PositiveSign::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGen(gen);
    llvm::Type* _type = value->getType();
    if(!(_type->isIntegerTy() || _type->isIntegerTy()))
        ErrorMessage("Only integers and floating point numbers should be used as operand of '+'", 5);
    return value;
}

llvm::Value* PositiveSign::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("+ was used as left value", 458);
    return NULL;
}

NegativeSign::NegativeSign(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Negative");
        addChildren(Operand);
    }
}

llvm::Value* NegativeSign::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGen(gen);
    llvm::Type* _type = value->getType();
    if(!(_type->isIntegerTy() || _type->isFloatingPointTy()))
        ErrorMessage("Only integers and floating point numbers should be used as operand of '-'", 5);
    if(value->getType()->isIntegerTy())
        return gen.Builder.CreateNeg(value);
    else
        return gen.Builder.CreateFNeg(value);
}

llvm::Value* NegativeSign::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("- was used as left value", 458);
    return NULL;
}

Increment::Increment(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Increment");
        addChildren(Operand);
    }
}

llvm::Value* Increment::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGenPtr(gen);
    llvm::Value* RetVal = gen.Builder.CreateLoad(value->getType()->getPointerElementType(), value);
    // temporarily, only int is allowed to increment
    llvm::Type* _type = value->getType()->getPointerElementType();
    if(!(_type->isIntegerTy()))
        ErrorMessage("Only integers are allowed to increment", 429);
    llvm::Value* puls = gen.Builder.CreateAdd(RetVal, llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen.Context), 1));
    gen.Builder.CreateStore(puls, value);
    return RetVal;
}

llvm::Value* Increment::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("++ was used as left value", 458);
    return NULL;
}

Decrement::Decrement(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Decrement");
        addChildren(Operand);
    }
}

llvm::Value* Decrement::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGen(gen);
    llvm::Value* RetVal = gen.Builder.CreateLoad(value->getType()->getPointerElementType(), value);
    llvm::Type* _type = value->getType()->getPointerElementType();
    // temporarily, only int is allowed to decrement
    if(!(_type->isIntegerTy()))
        ErrorMessage("Only integers are allowed to decrement", 446);
    llvm::Value* sub = gen.Builder.CreateSub(RetVal, llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen.Context), -1));
    gen.Builder.CreateStore(sub, value);
    return RetVal;
}

llvm::Value* Decrement::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("-- wsa used as left value", 511);
    return NULL;
}

ValueOf::ValueOf(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("ValueOf");
        addChildren(Operand);
    }
}

llvm::Value* ValueOf::CodeGen(IRGenerator& gen){
    return SuperLoad(CodeGenPtr(gen), gen);
}

llvm::Value* ValueOf::CodeGenPtr(IRGenerator& gen){
    llvm::Value* result = Operand->CodeGen(gen);
    if(!result->getType()->isPointerTy())
        ErrorMessage("* was applied to a non-pointer value", 530);
    return result;
}

AddressOf::AddressOf(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("AddressOf");
        addChildren(Operand);
    }
}

llvm::Value* AddressOf::CodeGen(IRGenerator& gen){
    return Operand->CodeGenPtr(gen);
}

llvm::Value* AddressOf::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("& was used as left value", 546);
    return NULL;
}

LogicNot::LogicNot(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("LogicNot");
        addChildren(Operand);
    }
}

llvm::Value* LogicNot::CodeGen(IRGenerator& gen){
    return gen.Builder.CreateICmpNE(Cast2Bool(Operand->CodeGen(gen), gen), llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen.Context), 1));
}

llvm::Value* LogicNot::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("LogicNot was used as left value", 485);
    return NULL;
}

BitWiseNot::BitWiseNot(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("BitWiseNot");
        addChildren(Operand);
    }
}

llvm::Value* BitWiseNot::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGen(gen);
    if(!value->getType()->isIntegerTy()){
        ErrorMessage("Bitwise Not must be applied to integers", 499);
    }
    return gen.Builder.CreateNot(value);
}

llvm::Value* BitWiseNot::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("BitWiseNot was used as left value", 506);
    return NULL;
}

LogicAnd::LogicAnd(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("LogicAnd");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* LogicAnd::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    A = Cast2Bool(A, gen);
    B = Cast2Bool(B, gen);
    return gen.Builder.CreateAnd(A, B);
}

llvm::Value* LogicAnd::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("LogicAnd was used as left value", 526);
    return NULL;
}

BitWiseAnd::BitWiseAnd(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("BitWiseAnd");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* BitWiseAnd::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(!(A->getType()->isIntegerTy() && B->getType()->isIntegerTy()))
        TypeUpgrading(A, B, gen);
    else
        ErrorMessage("BitWiseAnd must be applied to integers", 546);
    return gen.Builder.CreateAnd(A, B);
}

llvm::Value* BitWiseAnd::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("BitWiseAnd was used as left value", 547);
    return NULL;
}

LogicOr::LogicOr(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("LogicOr");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* LogicOr::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    A = Cast2Bool(A, gen);
    B = Cast2Bool(B, gen);
    return gen.Builder.CreateOr(A, B);
}

llvm::Value* LogicOr::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("LogicOr was used as left value", 547);
    return NULL;
}

BitWiseOr::BitWiseOr(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("BitWiseOr");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* BitWiseOr::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(!(A->getType()->isIntegerTy() && B->getType()->isIntegerTy()))
        TypeUpgrading(A, B, gen);
    else
        ErrorMessage("BitWiseOr must be applied to integers", 590);
    return gen.Builder.CreateOr(A, B);
}

llvm::Value* BitWiseOr::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("BitWiseOr was used as left value", 590);
    return NULL;
}

LogicXor::LogicXor(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("LogicXor");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* LogicXor::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    A = Cast2Bool(A, gen);
    B = Cast2Bool(B, gen);
    return gen.Builder.CreateXor(A, B);
}

llvm::Value* LogicXor::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("LogicXor was used as left value", 590);
    return NULL;
}

BitWiseXor::BitWiseXor(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("BitWiseXor");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* BitWiseXor::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(!(A->getType()->isIntegerTy() && B->getType()->isIntegerTy()))
        TypeUpgrading(A, B, gen);
    else
        ErrorMessage("BitWiseXor must be applied to integers", 634);
    return gen.Builder.CreateXor(A,B);
}

llvm::Value* BitWiseXor::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("BitWiseXor was used as left value", 638);
    return NULL;
}

Add::Add(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("+");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Add::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateAdd(A, B);
        else
            return gen.Builder.CreateFAdd(A, B);
    }
    else if(A->getType()->isPointerTy() && B->getType()->isIntegerTy())
        return gen.Builder.CreateGEP(A->getType()->getPointerElementType(), A, B);
    else if(A->getType()->isIntegerTy() && B->getType()->isPointerTy())
        return gen.Builder.CreateGEP(B->getType()->getPointerElementType(), B, A);
    ErrorMessage("An unsupport type was used in addition", 658);
    return NULL;
}

llvm::Value* Add::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Addition was used as left value", 662);
    return NULL;
}

Sub::Sub(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("-");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Sub::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateSub(A, B);
        else
            return gen.Builder.CreateFSub(A, B);
    }
    else if(A->getType()->isPointerTy() && B->getType()->isIntegerTy())
        return gen.Builder.CreateGEP(A->getType()->getPointerElementType(), A, gen.Builder.CreateNeg(B));
    else if(A->getType()->isIntegerTy() && B->getType()->isPointerTy())
        return gen.Builder.CreateGEP(B->getType()->getPointerElementType(), B, gen.Builder.CreateNeg(A));
    ErrorMessage("An unsupport type was used in subtraction", 688);
    return NULL;
}

llvm::Value* Sub::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Subtraction was used as left value", 693);
    return NULL;
}

Div::Div(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("/");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Div::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateSDiv(A, B);
        else
            return gen.Builder.CreateFDiv(A, B);
    }
    ErrorMessage("Unsupported type was used in division", 719);
    return NULL;
}

llvm::Value* Div::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Division was used as left value", 693);
    return NULL;
}

Mul::Mul(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("*");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Mul::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateMul(A, B);
        else
            return gen.Builder.CreateFMul(A, B);
    }
    ErrorMessage("Unsupported type was used in multiplication", 745);
    return NULL;
}

llvm::Value* Mul::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Multiplication was used as left value", 693);
    return NULL;
}

Mod::Mod(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("%");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Mod::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(A->getType()->isIntegerTy() && B->getType()->isIntegerTy()){
        TypeUpgrading(A, B, gen);
        return gen.Builder.CreateSRem(A, B);
    }
    ErrorMessage("Unsupport type was used in Modulo", 769);
    return NULL;
}

llvm::Value* Mod::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Module was used as left value", 693);
    return NULL;
}

Gt::Gt(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName(">");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Gt::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateICmpSGT(A, B);
        else
            return gen.Builder.CreateFCmpOGT(A, B);
    }
    else{
        ErrorMessage("unsupport type was used for compariation", 797);
    }
}

llvm::Value* Gt::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("> was used as left value", 802);
    return NULL;
}

Ge::Ge(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName(">=");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Ge::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateICmpSGE(A, B);
        else
            return gen.Builder.CreateFCmpOGE(A, B);
    }
    else{
        ErrorMessage("unsupport type was used for compariation", 824);
    }
}

llvm::Value* Ge::CodeGenPtr(IRGenerator& gen){
    ErrorMessage(">= was used as left value", 829);
    return NULL;
}

Lt::Lt(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("<");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Lt::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateICmpSLT(A, B);
        else
            return gen.Builder.CreateFCmpOLT(A, B);
    }
    else{
        ErrorMessage("unsupport type was used for compariation", 851);
    }
}

llvm::Value* Lt::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("< was used as left value", 856);
    return NULL;
}

Le::Le(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("<=");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Le::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateICmpSLE(A, B);
        else
            return gen.Builder.CreateFCmpOLE(A, B);
    }
    else{
        ErrorMessage("unsupport type was used for compariation", 878);
    }
}

llvm::Value* Le::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("<= was used as left value", 883);
    return NULL;
}

Eq::Eq(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("==");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Eq::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateICmpEQ(A, B);
        else
            return gen.Builder.CreateFCmpOEQ(A, B);
    }
    else if(A->getType()->isPointerTy() && B->getType()->isPointerTy())
        return gen.Builder.CreateICmpEQ(
            gen.Builder.CreatePtrToInt(A, llvm::Type::getInt32Ty(gen.Context)),
            gen.Builder.CreatePtrToInt(B, llvm::Type::getInt32Ty(gen.Context))
        );
    else{
        ErrorMessage("unsupport type was used for compariation", 910);
    }
}

llvm::Value* Eq::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("== was used as left value", 915);
    return NULL;
}

Neq::Neq(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("!=");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

llvm::Value* Neq::CodeGen(IRGenerator& gen){
    llvm::Value* A = OperandA->CodeGen(gen);
    llvm::Value* B = OperandB->CodeGen(gen);
    if(TypeUpgrading(A, B, gen)){
        if(A->getType()->isIntegerTy())
            return gen.Builder.CreateICmpNE(A, B);
        else
            return gen.Builder.CreateFCmpONE(A, B);
    }
    else if(A->getType()->isPointerTy() && B->getType()->isPointerTy())
        return gen.Builder.CreateICmpNE(
            gen.Builder.CreatePtrToInt(A, llvm::Type::getInt32Ty(gen.Context)),
            gen.Builder.CreatePtrToInt(B, llvm::Type::getInt32Ty(gen.Context))
        );
    else{
        ErrorMessage("unsupport type was used for compariation", 942);
    }
}

llvm::Value* Neq::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("!= was used as left value", 947);
    return NULL;
}

Conditional::Conditional(Expression* _Condition, Expression* _ValueTrue, Expression* _ValueFalse):Condition(_Condition),ValueFalse(_ValueFalse),ValueTrue(_ValueTrue){
    if(VISIBLE){
        setNodeName("?");
        addChildren(Condition);
        addChildren(ValueFalse);
        addChildren(ValueTrue);
    }
}

llvm::Value* Conditional::CodeGen(IRGenerator& gen){
    llvm::Value* _Condition = Cast2Bool(Condition->CodeGen(gen), gen);
    llvm::Value* _True = ValueTrue->CodeGen(gen);
    llvm::Value* _False = ValueFalse->CodeGen(gen);
    if(_True->getType() == _False->getType() || TypeUpgrading(_True, _False, gen))
        return gen.Builder.CreateSelect(_Condition, _True, _False);
    else{
        ErrorMessage("Two value has different type in ?", 967);
        return NULL;
    }
}

llvm::Value* Conditional::CodeGenPtr(IRGenerator& gen){
    llvm::Value* _Condition = Cast2Bool(Condition->CodeGen(gen), gen);
    llvm::Value* _True = ValueTrue->CodeGenPtr(gen);
    llvm::Value* _False = ValueFalse->CodeGenPtr(gen);
    if(_True->getType() == _False->getType())
        return gen.Builder.CreateSelect(_Condition, _True, _False);
    ErrorMessage("When ? used as left value, two operands must have same type", 978);
    return NULL;
}

Assign::Assign(Expression* _Target, Expression* _Object):Target(_Target),Object(_Object){
    if(VISIBLE){
        setNodeName("=");
        addChildren(Target);
        addChildren(Object);
    }
}

llvm::Value* Assign::CodeGen(IRGenerator& gen){
    llvm::Value* _result = Object->CodeGen(gen);
    llvm::Value* _target = Target->CodeGenPtr(gen);
    _result = TypeCastTo(_result, _target->getType()->getPointerElementType(), gen);
    if(_result == NULL){
        ErrorMessage("unsupport cast", 849);
        return NULL;
    }
    gen.Builder.CreateStore(_result, _target);
    return _result;
}

llvm::Value* Assign::CodeGenPtr(IRGenerator& gen){
    // I don't think this function will be used
    // realize it when it's called
    ErrorMessage("Assign::CodeGenPtr", 1005);
    return NULL;
}

Constant::Constant(bool b):Type(_BOOL_){
    Value.b = b;
    if(VISIBLE)
        setNodeName("Constant bool");
}

Constant::Constant(int i):Type(_INT_){
    Value.i = i;
    if(VISIBLE)
        setNodeName("Constant int");
}

Constant::Constant(double d):Type(_DOUBLE_){
    Value.d = d;
    if(VISIBLE)
        setNodeName("Constant double");
}

Constant::Constant(char c):Type(_CHAR_){
    Value.c = c;
    if(VISIBLE)
        setNodeName("Constant char");
}

llvm::Value* Constant::CodeGen(IRGenerator& gen){
    switch(Type){
        case _INT_: return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.Context), Value.i);
        case _SHORT_: return llvm::ConstantInt::get(llvm::Type::getInt16Ty(gen.Context), Value.i);
        case _LONG_: return llvm::ConstantInt::get(llvm::Type::getInt64Ty(gen.Context), Value.i);
        case _FLOAT_: return llvm::ConstantFP::get(llvm::Type::getFloatTy(gen.Context), Value.d);
        case _DOUBLE_: return llvm::ConstantFP::get(llvm::Type::getDoubleTy(gen.Context), Value.d);
        case _BOOL_: return llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen.Context), Value.b);
        case _CHAR_: return llvm::ConstantInt::get(llvm::Type::getInt8Ty(gen.Context), Value.c);
    }
    ErrorMessage("unknown type", 1043);
}

llvm::Value* Constant::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Constant was used as left value", 1047);
    return NULL;
}

Variable::Variable(std::string* _Name):Name(_Name){
    if(VISIBLE)
        setNodeName(*Name);
}

llvm::Value* Variable::CodeGen(IRGenerator& gen){
    auto iter = gen.NamedValues.find(*Name);
    if(iter == gen.NamedValues.end())
        ErrorMessage("undefined variable is used", 682);
    return SuperLoad(iter->second, gen);
}

llvm::Value* Variable::CodeGenPtr(IRGenerator& gen){
    auto iter = gen.NamedValues.find(*Name);
    if(iter == gen.NamedValues.end())
        ErrorMessage("undefined variable is used", 689);
    return iter->second;
}

StrVar::StrVar(std::string* _Value):Value(_Value){
    if(VISIBLE)
        setNodeName("Constant string");
}

llvm::Value* StrVar::CodeGen(IRGenerator& gen){
    return gen.Builder.CreateGlobalStringPtr(Value->c_str());
}

llvm::Value* StrVar::CodeGenPtr(IRGenerator& gen){
    ErrorMessage("Constant string was used as left value", 1047);
    return NULL;
}