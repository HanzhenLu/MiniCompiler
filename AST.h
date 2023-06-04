#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "IRGenerator.h"

class Definition;
class VarType;
class ArgList;
class Var;
class Program;
class Block;


/*
 *   usage:
 *   Mandatory: Create new node with new Node("str");
 *   Mandatory: add children node by using addChildren();
 *   Get 'AST.dot' file as output by using generateGraphVizOutput(); passing pointer to root node as parameter.
 */
class Node{
    std::string mNodeName;
public:
    int currentNodeNumber;
    static std::string graphVizRelation;
    void generateGraphVizOutput();
    static int nodeCount;
    std::vector<Node*> childrenList;
    Node(){currentNodeNumber = nodeCount;nodeCount++; mNodeName = "default";}
    Node(std::string name){currentNodeNumber = nodeCount;nodeCount++; mNodeName = name;}
    void addChildren(Node* nptr){
        childrenList.emplace_back(nptr);
    }
    void getGraphVizOutput(Node* root);
    const std::string& getNodeName() const{return mNodeName;}
    void setNodeName(std::string name);
    //virtual llvm::Value* CodeGen(IRGenerator& gen);
};


class Program: public Node{
    std::vector<Definition*>* Definitions;
public:
    Program(std::vector<Definition*>* _Definitions);
    llvm::Value* CodeGen(IRGenerator& gen);
};

class Statement: public Node{
public:
    // this function should be changed to Virtual function later
    virtual llvm::Value* CodeGen(IRGenerator& gen){
        std::cout << "Statement" <<std::endl;
        return NULL;
    }
};

class Definition: public Statement{
public:
    // this function should be changed to Virtual function later
    virtual llvm::Value* CodeGen(IRGenerator& gen){
        std::cout << "Definition" <<std::endl;
        return NULL;
    }
};

class FunName: public Node{
    std::string* Name;
    int PointerDim;
public:
    FunName(std::string* _Name, int _PointerDim);
    std::string* GetName();
    int GetPointerDim();
};

class Star: public Node{
    int Dim;
public:
    void Add();
    int GetDim();
};

class FunDefinition: public Definition{
    VarType* ReturnType;
    FunName* Name;
    ArgList* Args;
    Block* Statements;
public:
    FunDefinition(VarType* _ReturnType, FunName* _FunNam, ArgList* _Args, Block* _Statements);
    llvm::Value* CodeGen(IRGenerator& gen);
};

class VarDefinition: public Definition{
    VarType* Type;
    std::vector<Var*>* List;
public:
    VarDefinition(VarType* _Type, std::vector<Var*>* _List);
    void PushType(std::vector<llvm::Type*>& elements, IRGenerator& gen);
    llvm::Value* CodeGen(IRGenerator& gen);
};

class ArgList: public Node{
    std::vector<VarType*> Types;
    std::vector<Var*> Names;
public:
    ArgList();
    void Add(VarType* _Type, Var* _Name);
    void SetTypeForVar();
    void PushType(std::vector<llvm::Type*>& elements, IRGenerator& gen);
    std::string GetNameByIndex(int idx);
};

class Var: public Node{
    VarType* ASTType;
    llvm::Type* LLVMType;
    int PointerDim;
    std::vector<int> ArrayDim;
    std::string* Name;
public:
    Var(std::string* _Name);
    void SetPointer(int dim);
    void AddArray(int size);
    void SetType(VarType* _Type);
    // Wrap pointers and arrays on types from VarType
    llvm::Type* GetType(IRGenerator& gen);
    std::string GetName();
};

class VarType: public Node{
protected:
    llvm::Type* Type;
public:
    VarType():Type(NULL){}
    // generate type except pointer and array
    virtual llvm::Type* GetType(IRGenerator& gen) = 0;
};

enum TypeIndex{
    _INT_,
    _SHORT_,
    _LONG_,
    _FLOAT_,
    _DOUBLE_,
    _CHAR_,
    _BOOL_,
    _VOID_
};

class BuildInType: public VarType{
    enum TypeIndex Index;
public:
    BuildInType(enum TypeIndex _Index);
    llvm::Type* GetType(IRGenerator& gen);
};

class StructType: public VarType{
    std::string* Name;
    std::vector<VarDefinition*> VarDefinitions;
public:
    StructType();
    void Add(VarDefinition* _VarDefinition);
    llvm::Type* GetType(IRGenerator& gen);
    void SetName(std::string* _Name);
};

class EnumDefinition: public Definition{
    std::string* Name;
    int Value;
public:
    EnumDefinition(std::string* _Name, int _Value);
};

class EnumType: public VarType{
    std::vector<EnumDefinition*> EnumDefinitions;
public:
    void Add(EnumDefinition* _EnumDefinition);
    llvm::Type* GetType(IRGenerator& gen){
        std::cout << "EnumType" << std::endl; 
        return NULL;
    }
};

class Expression: public Statement{
public:
    // return the copy of the value as right value
    virtual llvm::Value* CodeGen(IRGenerator& gen) = 0;
    // return the pointer pointing to the address of the value, 
    // can be used as left value after loading
    virtual llvm::Value* CodeGenPtr(IRGenerator& gen) = 0;
};

class Block: public Statement{
    std::vector<Statement*>* statements;
public:
    Block(std::vector<Statement*>* _statements);
    llvm::Value* CodeGen(IRGenerator& gen);
};

class IfStatement: public Statement{
    Expression* Condition;
    Statement* True;
    Statement* False;
public:
    IfStatement(Expression* _Condition, Statement* _True, Statement* _False);
};

class WhileStatement: public Statement{
    Expression* Condition;
    Statement* Loop;
public:
    WhileStatement(Expression* _Condition, Statement* _Loop);
};

class DoWhileStatement: public Statement{
    Expression* Condition;
    Statement* Loop;
public:
    DoWhileStatement(Expression* _Condition, Statement* _Loop);
};

class ForStatement: public Statement{
    Expression* Initialization;
    Expression* Condition;
    Statement* Collection;
    Statement* Loop;
public:
    ForStatement(Expression* _Initialization, Expression* _Condition, Statement* _Collection, Statement* _Loop);
};

class ContinueStatement: public Statement{
};

class BreakStatement: public Statement{

};

class ReturnStatement: public Statement{
    Expression* ReturnValue;
public:
    ReturnStatement(Expression* _ReturnValue);
};

class GetItem: public Expression{
    Expression* Array;
    Expression* Index;
public:
    GetItem(Expression* _Array, Expression* _Index);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class FunctionCall: public Expression{
    std::string* FunName;
    std::vector<Expression*>* Args;
public:
    FunctionCall(std::string* _FunName, std::vector<Expression*>* _Args);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Component: public Expression{
    Expression* Structure;
    std::string* ComponentName;
public:
    Component(Expression* _Structure, std::string* _ComponentName);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class PtrComponent: public Expression{
    Expression* PtrStructure;
    std::string* ComponentName;
public:
    PtrComponent(Expression* _PtrStructure, std::string* _ComponentName);
    llvm::Value* CodeGen(IRGenerator& gen){}
    llvm::Value* CodeGenPtr(IRGenerator& gen){}
};

class PositiveSign: public Expression{
    Expression* Operand;
public:
    PositiveSign(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class NegativeSign: public Expression{
    Expression* Operand;
public:
    NegativeSign(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Increment: public Expression{
    Expression* Operand;
public:
    Increment(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Decrement: public Expression{
    Expression* Operand;
public:
    Decrement(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class ValueOf: public Expression{
    Expression* Operand;
public:
    ValueOf(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class AddressOf: public Expression{
    Expression* Operand;
public:
    AddressOf(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class LogicNot: public Expression{
    Expression* Operand;
public:
    LogicNot(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class BitWiseNot: public Expression{
    Expression* Operand;
public:
    BitWiseNot(Expression* _Operand);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class LogicAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    LogicAnd(Expression* A, Expression*B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class BitWiseAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    BitWiseAnd(Expression* A, Expression*B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class LogicOr: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    LogicOr(Expression* A, Expression*B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class BitWiseOr: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    BitWiseOr(Expression* A, Expression*B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class LogicXor: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    LogicXor(Expression* A, Expression*B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class BitWiseXor: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    BitWiseXor(Expression* A, Expression*B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Div: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Div(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Mul: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Mul(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Mod: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Mod(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Add: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Add(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Sub: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Sub(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Gt: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Gt(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Ge: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Ge(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Lt: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Lt(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Le: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Le(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Eq: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Eq(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Neq: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Neq(Expression* _A, Expression* _B);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Conditional: public Expression{
    Expression* Condition;
    Expression* ValueTrue;
    Expression* ValueFalse;
public:
    Conditional(Expression* _Condition, Expression* _ValueTrue, Expression* _ValueFalse);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Assign: public Expression{
    Expression* Target;
    Expression* Object;
public:
    Assign(Expression* _Target, Expression* _Object);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

union value{
    int i;
    double d;
    char c;
    bool b;
};

class Constant: public Expression{
    TypeIndex Type;
    union value Value;
public:
    Constant(bool b);
    Constant(int i);
    Constant(double d);
    Constant(char c);
    Constant(){}
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class Variable: public Expression{
    std::string* Name;
public:
    Variable(std::string* _Name);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};

class StrVar: public Constant{
    std::string* Value;
public:
    StrVar(std::string* _Value);
    llvm::Value* CodeGen(IRGenerator& gen);
    llvm::Value* CodeGenPtr(IRGenerator& gen);
};