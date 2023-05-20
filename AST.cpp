#include <iostream>
#include <vector>
#include "AST.h"

Program::Program(std::vector<Definition*>* _Definitions):Definitions(_Definitions){}

FunName::FunName(std::string* _Name, int _PointerDim):Name(_Name),PointerDim(_PointerDim){}

void Star::Add(){
    Dim++;
}

int Star::GetDim(){
    return Dim;
}

FunDefinition::FunDefinition(VarType* _ReturnType, FunName* _FunNam, ArgList* _Args, std::vector<Statement*>* _Statements):ReturnType(_ReturnType),Name(_FunNam),Args(_Args),Statements(_Statements){}

VarDefinition::VarDefinition(VarType* _Type, std::vector<Var*>* _List):Type(_Type),List(_List){}

void ArgList::Add(VarType* _Type, std::string* _Name){
    Types.push_back(_Type);
    Names.push_back(_Name);
}

Var::Var(std::string* _Name):PointerDim(0),Name(_Name){}

void Var::SetPointer(int dim){
    PointerDim = dim;
}

void Var::AddArray(int size){
    ArrayDim.push_back(size);
}

BuildInType::BuildInType(enum TypeIndex _Index):Index(_Index){}

void StructType::Add(VarDefinition* _VarDefinition){
    VarDefinitions.push_back(_VarDefinition);
}

EnumDefinition::EnumDefinition(std::string* _Name, int _Value):Name(_Name),Value(_Value){}

void EnumType::Add(EnumDefinition* _EnumDefinition){
    EnumDefinitions.push_back(_EnumDefinition);
}

Block::Block(std::vector<Statement*>* _statements):statements(_statements){}

IfStatement::IfStatement(Expression* _Condition, Statement* _True, Statement* _False):Condition(_Condition),True(_True),False(_False){}

WhileStatement::WhileStatement(Expression* _Condition, Statement* _Loop):Condition(_Condition),Loop(_Loop){}

DoWhileStatement::DoWhileStatement(Expression* _Condition, Statement* _Loop):Condition(_Condition),Loop(_Loop){}

ForStatement::ForStatement(std::vector<Statement*>* _Initialization, Expression* _Condition, Statement* _Collection, Statement* _Loop):Initialization(_Initialization), Condition(_Condition),Collection(_Collection),Loop(_Loop){}

ReturnStatement::ReturnStatement(Expression* _ReturnValue): ReturnValue(_ReturnValue){}

GetItem::GetItem(Expression* _Array, Expression* _Index):Array(_Array), Index(_Index){}

FunctionCall::FunctionCall(std::string* _FunName, std::vector<Expression*>* _Args):FunName(_FunName),Args(_Args){}

Component::Component(Expression* _Structure, std::string* _ComponentName):Structure(_Structure), ComponentName(_ComponentName){}

PtrComponent::PtrComponent(Expression* _PtrStructure, std::string* _ComponentName):PtrStructure(_PtrStructure), ComponentName(_ComponentName){}

PositiveSign::PositiveSign(Expression* _Operand):Operand(_Operand){}

NegativeSign::NegativeSign(Expression* _Operand):Operand(_Operand){}

Increment::Increment(Expression* _Operand):Operand(_Operand){}

Decrement::Decrement(Expression* _Operand):Operand(_Operand){}

ValueOf::ValueOf(Expression* _Operand):Operand(_Operand){}

AddressOf::AddressOf(Expression* _Operand):Operand(_Operand){}

LogicNot::LogicNot(Expression* _Operand):Operand(_Operand){}

BitWiseNot::BitWiseNot(Expression* _Operand):Operand(_Operand){}

LogicAnd::LogicAnd(Expression* A, Expression*B):OperandA(A),OperandB(B){}

BitWiseAnd::BitWiseAnd(Expression* A, Expression*B):OperandA(A),OperandB(B){}

LogicOr::LogicOr(Expression* A, Expression*B):OperandA(A),OperandB(B){}

BitWiseOr::BitWiseOr(Expression* A, Expression*B):OperandA(A),OperandB(B){}

LogicXor::LogicXor(Expression* A, Expression*B):OperandA(A),OperandB(B){}

BitWiseXor::BitWiseXor(Expression* A, Expression*B):OperandA(A),OperandB(B){}

Add::Add(Expression* A, Expression*B):OperandA(A),OperandB(B){}

Sub::Sub(Expression* A, Expression*B):OperandA(A),OperandB(B){}

Div::Div(Expression* A, Expression*B):OperandA(A),OperandB(B){}

Mul::Mul(Expression* A, Expression*B):OperandA(A),OperandB(B){}

Mod::Mod(Expression* A, Expression*B):OperandA(A),OperandB(B){}

GT::GT(Expression* A, Expression*B):OperandA(A),OperandB(B){}

GE::GE(Expression* A, Expression*B):OperandA(A),OperandB(B){}

LT::LT(Expression* A, Expression*B):OperandA(A),OperandB(B){}

LE::LE(Expression* A, Expression*B):OperandA(A),OperandB(B){}

EQ::EQ(Expression* A, Expression*B):OperandA(A),OperandB(B){}

NEQ::NEQ(Expression* A, Expression*B):OperandA(A),OperandB(B){}

Conditional::Conditional(Expression* _Condition, Expression* _ValueTrue, Expression* _ValueFalse):Condition(_Condition),ValueFalse(_ValueFalse),ValueTrue(_ValueTrue){}

Assign::Assign(Expression* _Target, Expression* _Object):Target(_Target),Object(_Object){}

Constant::Constant(bool b):Type(_BOOL_){
    Value.b = b;
}

Constant::Constant(int i):Type(_INT_){
    Value.i = i;
}

Constant::Constant(double d):Type(_DOUBLE_){
    Value.d = d;
}

Constant::Constant(char c):Type(_CHAR_){
    Value.c = c;
}

Variable::Variable(std::string* _Name):Name(_Name){}

StrVar::StrVar(std::string* _Value):Value(_Value){}