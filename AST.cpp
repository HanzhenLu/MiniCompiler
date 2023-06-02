#include "AST.h"
#define VISIBLE 1

// print error message in unified format
void ErrorMessage(std::string Message, int ExitCode){
    std::cout << "[ERROR] : " + Message << std::endl;
    exit(ExitCode); 
}

std::string Node::graphVizRelation;
int Node::nodeCount = 1;
std::map<std::string, llvm::Value*> NamedValues;

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
            ErrorMessage("redefinition of function with different args", 2);
        int idx = 0;
        NamedValues.clear();
        for(llvm::Function::arg_iterator AI = fun->arg_begin(); idx != ArgsType.size(); idx++, AI++){
            AI->setName(Args->GetNameByIndex(idx));
            NamedValues[Args->GetNameByIndex(idx)] = AI;
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
        llvm::Value* temp = gen.Builder.CreateAlloca(RealType);
        auto iter = NamedValues.find(i->GetName());
        if(iter != NamedValues.end())
            ErrorMessage("redefinition of variable", 3);
        NamedValues[i->GetName()] = temp;
    }
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
        ErrorMessage("ArgList has inconsistent size", 1);
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

FunctionCall::FunctionCall(std::string* _FunName, std::vector<Expression*>* _Args):FunName(_FunName),Args(_Args){
    if(VISIBLE){
        setNodeName("FunCall : " + *FunName);
        for(auto i : *Args)
            addChildren(i);
    }
}

Component::Component(Expression* _Structure, std::string* _ComponentName):Structure(_Structure), ComponentName(_ComponentName){
    if(VISIBLE){
        setNodeName("Component : " + *ComponentName);
        addChildren(Structure);
    }
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
    if(!(value->getType()->isIntegerTy() || value->getType()->isFloatingPointTy()))
        ErrorMessage("Only integers and floating point numbers should be used as operand of '+'", 5);
    return value;
}

NegativeSign::NegativeSign(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Negative");
        addChildren(Operand);
    }
}

llvm::Value* NegativeSign::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGen(gen);
    if(!(value->getType()->isIntegerTy() || value->getType()->isFloatingPointTy()))
        ErrorMessage("Only integers and floating point numbers should be used as operand of '-'", 5);
    if(value->getType()->isIntegerTy())
        return gen.Builder.CreateNeg(value);
    else
        return gen.Builder.CreateFNeg(value);
}

Increment::Increment(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Increment");
        addChildren(Operand);
    }
}

llvm::Value* Increment::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGen(gen);
    // temporarily, only int is allowed to increment
    if(!(value->getType()->isIntegerTy()))
        ErrorMessage("Only integers are allowed to increment", 6);
    llvm::Value* puls = gen.Builder.CreateAdd(value, llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.Context), 1));
    gen.Builder.CreateStore(puls, value);
    return value;
}

Decrement::Decrement(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Decrement");
        addChildren(Operand);
    }
}

llvm::Value* Decrement::CodeGen(IRGenerator& gen){
    llvm::Value* value = Operand->CodeGen(gen);
    // temporarily, only int is allowed to increment
    if(!(value->getType()->isIntegerTy()))
        ErrorMessage("Only integers are allowed to decrement", 6);
    llvm::Value* puls = gen.Builder.CreateAdd(value, llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.Context), -1));
    gen.Builder.CreateStore(puls, value);
    return value;
}

ValueOf::ValueOf(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("ValueOf");
        addChildren(Operand);
    }
}

AddressOf::AddressOf(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("AddressOf");
        addChildren(Operand);
    }
}

LogicNot::LogicNot(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("LogicNot");
        addChildren(Operand);
    }
}

BitWiseNot::BitWiseNot(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("BitWiseNot");
        addChildren(Operand);
    }
}

LogicAnd::LogicAnd(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("LogicAnd");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

BitWiseAnd::BitWiseAnd(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("BitWiseAnd");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

LogicOr::LogicOr(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("LogicOr");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

BitWiseOr::BitWiseOr(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("BitWiseOr");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

LogicXor::LogicXor(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("LogicXor");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

BitWiseXor::BitWiseXor(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("BitWiseXor");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Add::Add(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("+");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Sub::Sub(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("-");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Div::Div(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("/");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Mul::Mul(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("*");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Mod::Mod(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("%");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Gt::Gt(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName(">");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Ge::Ge(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName(">=");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Lt::Lt(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("<");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Le::Le(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("<=");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Eq::Eq(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("==");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Neq::Neq(Expression* A, Expression*B):OperandA(A),OperandB(B){
    if(VISIBLE){
        setNodeName("!=");
        addChildren(OperandA);
        addChildren(OperandB);
    }
}

Conditional::Conditional(Expression* _Condition, Expression* _ValueTrue, Expression* _ValueFalse):Condition(_Condition),ValueFalse(_ValueFalse),ValueTrue(_ValueTrue){
    if(VISIBLE){
        setNodeName("?");
        addChildren(Condition);
        addChildren(ValueFalse);
        addChildren(ValueTrue);
    }
}

Assign::Assign(Expression* _Target, Expression* _Object):Target(_Target),Object(_Object){
    if(VISIBLE){
        setNodeName("=");
        addChildren(Target);
        addChildren(Object);
    }
}

llvm::Value* Assign::CodeGen(IRGenerator& gen){
    gen.Builder.CreateStore(Object->CodeGen(gen), Target->CodeGen(gen));
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
    ErrorMessage("unknown type", 4);
}

Variable::Variable(std::string* _Name):Name(_Name){
    if(VISIBLE)
        setNodeName(*Name);
}

StrVar::StrVar(std::string* _Value):Value(_Value){
    if(VISIBLE)
        setNodeName("Constant string");
}