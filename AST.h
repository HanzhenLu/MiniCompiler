#include <iostream>
#include <vector>

class Node{
};

class Definition;

class Program: public Node{
    std::vector<Definition*>* Definitions;
    Program(std::vector<Definition*>* _Definitions);
};

class Statement: public Node{
};

class Definition: public Statement{

};

class TypeDefinition: public Definition{
    VarType* Type;
    std::string Alias;
    TypeDefinition(VarType* _Type, std::string _Alias);
};

class FunName: public Node{
    std::string Name;
    int PointerDim;
    FunName(std::string _Name, int _PointDim);
};

class Star: public Node{
    int Dim;
public:
    void Add();
    int GetDim();
};

class FunDefinition: public Definition{
    VarType* RetrurnType;
    FunName* Name;
    ArgList* Args;
    std::vector<Statement*>* Statements;
    FunDefinition(VarType* _ReturnType, FunName* _FunNam, ArgList* _Args, std::vector<Statement*>* _Statements);
};

class VarDefinition: public Definition{
    VarType* Type;
    std::vector<Var*>* List;
    VarDefinition(VarType* _Type, std::vector<Var*>* _List);
};

class ArgList: public Node{
    std::vector<VarType*> Types;
    std::vector<std::string> Names;
public:
    void Add(VarType _Type, std::string _Name);
};

class Var: public Node{
    bool Pointer;
    bool Array;
    int PointerDim;
    std::vector<int> ArrayDim;
    std::string Name;
    Var(std::string _Name);
public:
    void SetPointer(int dim);
    void AddArray(int _Size);
};

class VarType: public Node{
    void* Type;
};

class BuildInType: public VarType{
    enum TypeIndex{
        INT,
        SHORT,
        LONG,
        FLOAT,
        DOUBLE,
        CHAR,
        BOOL,
        VOID
    };
    enum TypeIndex Index;
    BuildInType(enum TypeIndex _Index);
};

class RenameType: public VarType{
    std::string name;
    RenameType(std::string _name);
};

class StructType: public VarType{
    std::vector<VarType*> Types;
    std::vector<std::string> Names;
public:
    void Add(VarType* _Type, std::string _Name);
};

class EnumType: public VarType{
    std::vector<std::string> Names;
    std::vector<int> Values;
public:
    void Add(std::string _Name, int Values);
};

class Expression: public Node{

};

class Block: public Statement{
    Statement* statements;
    Block(Statement* _statements);
};

class IfStatement: public Statement{
    Expression* Condition;
    Statement* True;
    Statement* False;
    IfStatement(Expression* _Condition, Statement* _True, Statement* _Falsee);
};

class WhileStatement: public Statement{
    Expression* Condition;
    Statement* Loop;
    WhileStatement(Expression* _Condition, Statement* Loop);
};

class DoWhileStatement: public Statement{
    Expression* Condition;
    Statement* Loop;
    DoWhileStatement(Expression* _Condition, Statement* Loop);
};

class ForStatement: public Statement{
    Statement* Initialization;
    Expression* Condition;
    Statement* Collection;
    Statement* Loop;
    ForStatement(Statement* _Initialization, Expression* _Condition, Statement* _Collection, Statement* _Loop);
};

class ContinueStatement: public Statement{
};

class BreakStatement: public Statement{

};

class ReturnStatement: public Statement{
    Expression* ReturnValue;
    ReturnStatement(Expression* _ReturnValue);
};

class GetItem: public Expression{
    Var* Array;
    Expression* Index;
    GetItem(Var* _Array, Expression* Index);
};

class FunctionCall: public Expression{
    std::string FunName;
    std::vector<Expression*>* Args;
    FunctionCall(std::string _FunName, std::vector<Expression*>* _Args);
};

class Component: public Expression{
    Expression* Structure;
    std::string ComponentName;
    Component(Expression* _Structure, std::string _ComponentName);
};

class PtrComponent: public Expression{
    Expression* PtrStructure;
    std::string ComponentName;
    PtrComponent(Expression* _PtrStructure, std::string _ComponentName);
};

class PositiveSign: public Expression{
    Expression* Operand;
    PositiveSign(Expression* _Operand);
};

class NegativeSign: public Expression{
    Expression* Operand;
    NegativeSign(Expression* _Operand);
};

class AutoIncrement: public Expression{
    Expression* Operand;
    AutoIncrement(Expression* _Operand);
};

class AutoDecrement: public Expression{
    Expression* Operand;
    AutoDecrement(Expression* _Operand);
};

class ValueOf: public Expression{
    Expression* Operand;
    ValueOf(Expression* _Operand);
};

class AddressOf: public Expression{
    Expression* Operand;
    AddressOf(Expression* _Operand);
};

class LogicNot: public Expression{
    Expression* Operand;
    LogicNot(Expression* _Operand);
};

class BitWiseNot: public Expression{
    Expression* Operand;
    BitWiseNot(Expression* _Operand);
};

class LogicAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    LogicAnd(Expression* A, Expression*B);
};

class BitWiseAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    BitWiseAnd(Expression* A, Expression*B);
};

class LogicOr: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    LogicOr(Expression* A, Expression*B);
};

class BitWiseOr: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    BitWiseOr(Expression* A, Expression*B);
};

class LogicXor: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    LogicXor(Expression* A, Expression*B);
};

class BitWiseXor: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    BitWiseXor(Expression* A, Expression*B);
};

class LogicAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    LogicAnd(Expression* A, Expression*B);
};

class BitWiseAnd: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    BitWiseAnd(Expression* A, Expression*B);
};

class Div: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    Div(Expression* _A, Expression* _B);
};

class Mul: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    Mul(Expression* _A, Expression* _B);
};

class Mod: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    Mod(Expression* _A, Expression* _B);
};

class Add: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    Add(Expression* _A, Expression* _B);
};

class Sub: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    Sub(Expression* _A, Expression* _B);
};

class GT: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    GT(Expression* _A, Expression* _B);
};

class GE: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    GE(Expression* _A, Expression* _B);
};

class LT: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    LT(Expression* _A, Expression* _B);
};

class LE: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    LE(Expression* _A, Expression* _B);
};

class EQ: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    EQ(Expression* _A, Expression* _B);
};

class NEQ: public Expression{
    Expression* OperandA;
    Expression* OperandB;
    NEQ(Expression* _A, Expression* _B);
};

class Conditional: public Expression{
    Expression* Condition;
    Expression* ValueTrue;
    Expression* ValueFalse;
    Conditional(Expression* _Condition, Expression* ValueTrue, Expression* ValueFalse);
};

class Assign: public Expression{
    Expression* Target;
    Expression* Object;
    Assign(Expression* _Target, Expression* _Object);
};

class Constant: public Expression{
    BuildInType::VarType Type;
    union value{
        int i;
        short s;
        long l;
        float f;
        double d;
        char c;
        bool b;
    };
    union value Value;
    Constant(bool b);
    Constant(short s);
    Constant(long l);
    Constant(float f);
    Constant(double d);
    Constant(char c);
    Constant(bool b);
};