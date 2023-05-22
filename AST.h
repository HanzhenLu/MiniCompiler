#pragma once
#include <iostream>
#include <vector>

class Definition;
class VarType;
class ArgList;
class Var;
class Program;


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
    void generateGraphVizOutput(Node* root);
    static int nodeCount;
    std::vector<Node*> childrenList;
    Node();
    Node(const std::string& name):mNodeName(name){currentNodeNumber = nodeCount;nodeCount++;}
    virtual ~Node();
    void addChildren(Node* nptr){
        childrenList.emplace_back(nptr);
    }
    void getGraphVizOutput(Node* child);
    std::string& getNodeName(){return mNodeName;}
};
std::string Node::graphVizRelation;
int Node::nodeCount = 1;


class Program: public Node{
    std::vector<Definition*>* Definitions;
public:
    Program(std::vector<Definition*>* _Definitions);
};

class Statement: public Node{
};

class Definition: public Statement{

};

class FunName: public Node{
    std::string* Name;
    int PointerDim;
public:
    FunName(std::string* _Name, int _PointerDim);
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
    std::vector<Statement*>* Statements;
public:
    FunDefinition(VarType* _ReturnType, FunName* _FunNam, ArgList* _Args, std::vector<Statement*>* _Statements);
};

class VarDefinition: public Definition{
    VarType* Type;
    std::vector<Var*>* List;
public:
    VarDefinition(VarType* _Type, std::vector<Var*>* _List);
};

class ArgList: public Node{
    std::vector<VarType*> Types;
    std::vector<std::string*> Names;
public:
    void Add(VarType* _Type, std::string* _Name);
};

class Var: public Node{
    int PointerDim;
    std::vector<int> ArrayDim;
    std::string* Name;
public:
    Var(std::string* _Name);
    void SetPointer(int dim);
    void AddArray(int size);
};

class VarType: public Node{
    void* Type;
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
};

class StructType: public VarType{
    std::vector<VarDefinition*> VarDefinitions;
public:
    void Add(VarDefinition* _VarDefinition);
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
};

class Expression: public Statement{

};

class Block: public Statement{
    std::vector<Statement*>* statements;
public:
    Block(std::vector<Statement*>* _statements);
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
};

class FunctionCall: public Expression{
    std::string* FunName;
    std::vector<Expression*>* Args;
public:
    FunctionCall(std::string* _FunName, std::vector<Expression*>* _Args);
};

class Component: public Expression{
    Expression* Structure;
    std::string* ComponentName;
public:
    Component(Expression* _Structure, std::string* _ComponentName);
};

class PtrComponent: public Expression{
    Expression* PtrStructure;
    std::string* ComponentName;
public:
    PtrComponent(Expression* _PtrStructure, std::string* _ComponentName);
};

class PositiveSign: public Expression{
    Expression* Operand;
public:
    PositiveSign(Expression* _Operand);
};

class NegativeSign: public Expression{
    Expression* Operand;
public:
    NegativeSign(Expression* _Operand);
};

class Increment: public Expression{
    Expression* Operand;
public:
    Increment(Expression* _Operand);
};

class Decrement: public Expression{
    Expression* Operand;
public:
    Decrement(Expression* _Operand);
};

class ValueOf: public Expression{
    Expression* Operand;
public:
    ValueOf(Expression* _Operand);
};

class AddressOf: public Expression{
    Expression* Operand;
public:
    AddressOf(Expression* _Operand);
};

class LogicNot: public Expression{
    Expression* Operand;
public:
    LogicNot(Expression* _Operand);
};

class BitWiseNot: public Expression{
    Expression* Operand;
public:
    BitWiseNot(Expression* _Operand);
};

class LogicAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    LogicAnd(Expression* A, Expression*B);
};

class BitWiseAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    BitWiseAnd(Expression* A, Expression*B);
};

class LogicOr: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    LogicOr(Expression* A, Expression*B);
};

class BitWiseOr: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    BitWiseOr(Expression* A, Expression*B);
};

class LogicXor: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    LogicXor(Expression* A, Expression*B);
};

class BitWiseXor: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    BitWiseXor(Expression* A, Expression*B);
};

class Div: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Div(Expression* _A, Expression* _B);
};

class Mul: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Mul(Expression* _A, Expression* _B);
};

class Mod: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Mod(Expression* _A, Expression* _B);
};

class Add: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Add(Expression* _A, Expression* _B);
};

class Sub: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Sub(Expression* _A, Expression* _B);
};

class Gt: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Gt(Expression* _A, Expression* _B);
};

class Ge: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Ge(Expression* _A, Expression* _B);
};

class Lt: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Lt(Expression* _A, Expression* _B);
};

class Le: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Le(Expression* _A, Expression* _B);
};

class Eq: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Eq(Expression* _A, Expression* _B);
};

class Neq: public Expression{
    Expression* OperandA;
    Expression* OperandB;
public:
    Neq(Expression* _A, Expression* _B);
};

class Conditional: public Expression{
    Expression* Condition;
    Expression* ValueTrue;
    Expression* ValueFalse;
public:
    Conditional(Expression* _Condition, Expression* _ValueTrue, Expression* _ValueFalse);
};

class Assign: public Expression{
    Expression* Target;
    Expression* Object;
public:
    Assign(Expression* _Target, Expression* _Object);
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
};

class Variable: public Expression{
    std::string* Name;
public:
    Variable(std::string* _Name);
};

class StrVar: public Constant{
    std::string* Value;
public:
    StrVar(std::string* _Value);
};