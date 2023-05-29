#include "AST.h"
#define VISIBLE 1

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

FunName::FunName(std::string* _Name, int _PointerDim):Name(_Name),PointerDim(_PointerDim){
    if(VISIBLE)
        setNodeName("FunName : " + *_Name);
}

std::string* FunName::GetName(){
    return Name;
}

void Star::Add(){
    Dim++;
}

int Star::GetDim(){
    return Dim;
}

FunDefinition::FunDefinition(VarType* _ReturnType, FunName* _FunNam, ArgList* _Args, std::vector<Statement*>* _Statements):ReturnType(_ReturnType),Name(_FunNam),Args(_Args),Statements(_Statements){
    if(VISIBLE){
        addChildren(ReturnType);
        addChildren(Name);
        addChildren(Args);
        Node* VNode = new Node();
        if(_Statements!=NULL){
            VNode->setNodeName("FunBody");
            addChildren(VNode);
            for(auto i : *_Statements){
                VNode->addChildren(i);
            }
        }
        setNodeName("FunDefinition");
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
    }
    setNodeName("VarDefinition");
}

ArgList::ArgList(){
    if(VISIBLE)
        setNodeName("ArgList");
}

void ArgList::Add(VarType* _Type, std::string* _Name){
    Types.push_back(_Type);
    Names.push_back(_Name);
    if(VISIBLE){
        Node* VNode = new Node();
        Node* CNode = new Node();
        VNode->addChildren(_Type);
        CNode->setNodeName(*_Name);
        VNode->addChildren(CNode);
        VNode->setNodeName("Arg");
        addChildren(VNode);
    }
}

Var::Var(std::string* _Name):PointerDim(0),Name(_Name){
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

BuildInType::BuildInType(enum TypeIndex _Index):Index(_Index){
    if(VISIBLE)
        setNodeName("BuildInType");
}

StructType::StructType(){
    if(VISIBLE)
        setNodeName("StructType");
}

void StructType::Add(VarDefinition* _VarDefinition){
    VarDefinitions.push_back(_VarDefinition);
    if(VISIBLE)
        addChildren(_VarDefinition);
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

NegativeSign::NegativeSign(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Negative");
        addChildren(Operand);
    }
}

Increment::Increment(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Increment");
        addChildren(Operand);
    }
}

Decrement::Decrement(Expression* _Operand):Operand(_Operand){
    if(VISIBLE){
        setNodeName("Decrement");
        addChildren(Operand);
    }
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

Variable::Variable(std::string* _Name):Name(_Name){
    if(VISIBLE)
        setNodeName(*Name);
}

StrVar::StrVar(std::string* _Value):Value(_Value){
    if(VISIBLE)
        setNodeName("Constant string");
}