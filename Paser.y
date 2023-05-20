%{
#include "AST.h" 
#include <string>
#include <iostream>
#include <vector>


void yyerror(const char *s) {
    std::printf("Error: %s\n", s);
    std::exit(1); 
}

int yylex(void);

Program *Root;
%}

%output "Parser.cpp"

%union {
    int iVal;
    std::string sVal;
    double dVal;
    char cVal;
    Program* program;
	std::vector<Definition*>* definitions;
	Definition* definition;
	FunDefinition* fundef;
	VarDefinition* vardef;
	TypeDefinition* typedefi;
	std::vector<statements*>* statements;
	FunName* funname;
	VarType* vartype;
	ArgList* arglist;
	Star* star;
	std::vector<Var*>* varlist; 
	Var* var;
	BuildInType* buildintype;
	EnumType* enumtype;
	StructType* structtype;
	Block* block;
	Statement* statement;
	IfStatement* ifstatement;
	WhileStatement* whilestatement;
	DoWhileStatement* dowhilestatement;
	ForStatement* forstatement;
	ContinueStatement* continuestatement;
	BreakStatement* breakstatement;
	ReturnStatement* returnstatement;
	Expression* expression;
	Constant* constant;
	std::vector<Expression*>* args;
}

%token  COMMA DOT SEMI QUES COLON
		LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
		EQ GE GT LE LT NEQ NOT ASSIGN
		AND BAND OR BOR
		ARW BXOR BNOT
		DADD ADD DSUB SUB
		MUL DIV MOD
		STRUCT TYPEDEF ENUM
		IF ELSE FOR WHILE DO
		BREAK CONTINUE RETURN TRUE FALSE
		BOOL SHORT INT LONG CHAR FLOAT DOUBLE VOID
		
%token<iVal> INTEGER
%token<sVal> IDENTIFIER, STRING
%token<dVal> REAL
%token<cVal> CHARACTER
%type<program> Program	
%type<definitions> Definitions 
%type<definition>	Definition
%type<fundef>	FunDefinition
%type<statements>	Statements	
%type<vardef>	VarDefinition	
%type<typedefi>	TypeDefinition	
%type<funname> FunName
%type<vartype>	VarType
%type<star> Star
%type<var> Array
%type<buildintype>	BuiltInType
%type<enumtype> EnumList, EnumAssign
%type<structtype>	StructList
%type<statement>	Statement
%type<ifstatement>	IfStatement
%type<forstatement>	ForStatement
%type<whilestatement>	WhileStatement
%type<dowhilestatement>	DoWhileStatement
%type<breakstatement>	BreakStatement
%type<continuestatement>	ContinueStatement
%type<returnstatement>	ReturnStatement
%type<block>	Block
%type<arglist>	ArgList
%type<var>	Var
%type<varlist>	VarList
%type<expression>	Expression
%type<constant>	Constant
%type<args>	Args

%nonassoc IF
%nonassoc ELSE

%left	COMMA //15
%left	FUNC_CALL_ARG_LIST
%right	ASSIGN//14
%right	QUES COLON //13
%left	OR//12
%left	AND//11
%left	BOR//10
%left	BXOR//9
%left	BAND//8
%left	EQ NEQ//7
%left	GE GT LE LT//6
%left	ADD SUB//4
%left	MUL DIV MOD//3
%right	DADD DSUB NOT BNOT//2
%left	DOT ARW//1

%start Program
%%
//
Program:	Definitions		{$$ = new AST::Program($1); Root = $$;}
			;
			
Definitions:	Definitions Definition	{$$ = $1; $$->push_back($2);}
				|	{$$ = new std::vector<Definition*>;}
				;

Definition:		FunDefinition	{$$ = $1;}
				| VarDefinition		{$$ = $1;}
				| TypeDefinition	{$$ = $1;}
				;

FunDefinition:	VarType FunName LPAREN ArgList RPAREN SEMI	{$$ = new FunDefinition($1,$2,$4,NULL);}
				| VarType FunName LPAREN ArgList RPAREN LBRACE Statements RBRACE	{$$ = new FunDefinition($1,$2,$4,$6);}
				;

FunName:	Star IDENTIFIER	{$$ = new FunName($1, $2->GetDim()); delete $2;}

Star:	Star MUL	{$$ = $1; $$->Add();}
		|	{$$ = new Star();}
		;

VarDefinition:	VarType VarList	SEMI	{$$ = new AST::VarDecl($1,$2);}
			;

VarList:	VarList COMMA Var	{$$ = $1; $$->push_back($3);}
			| Var	{$$ = new std::vector<Var*>; $$->push_back($1);}
			|	{$$ = new std::vector<Var*>;}
			;

Var:	Star Array	{$$ = $2; $$->SetPointer($1->GetDim()); delete $1;}
		;

Array: 	Array LPAREN INTEGER RPAREN	{$$ = $1; $$->AddArray($3);}
		| IDENTIFIER	{$$ = new Var($1);}
		;

TypeDefinition:	TYPEDEF VarType IDENTIFIER	SEMI	{$$ = new AST::TypeDecl($2, $3);}
				;

VarType:	BuiltInType	{$$ = $1;}
			| STRUCT LBRACE StructList RBRACE	{$$ = $3;}
			| ENUM LBRACE EnumList RBRACE	{$$ = $3;}
			| IDENTIFIER	{$$ = new RenameType($1);}
			;
			
BuiltInType: BOOL	{$$ = new BuiltInType(INT);}
			| SHORT	{$$ = new BuiltInType(Short);}
			| INT	{$$ = new BuiltInType(Int);}
			| LONG	{$$ = new BuiltInType(Long);}
			| CHAR	{$$ = new BuiltInType(Char);}
			| FLOAT	{$$ = new BuiltInType(Float);}
			| DOUBLE	{$$ = new BuiltInType(Double);}
			| VOID	{$$ = new BuiltInType(Void);}
			;

StructList:	StructList VarDefinition	{$$ = $1; $$->Add($2);}
			|	{$$ = new StructType();}
			;

EnumList:	EnumList COMMA EnumAssign	{$$ = $1; $$->Add($3);}
			| EnumAssign	{$$ = new EnumType(); $$->Add($1);}
			| 	{$$ = new EnumType();}
			;

EnumAssign:	IDENTIFIER	{$$ = new EnumDefinition($1, -1);}
			| IDENTIFIER ASSIGN INTEGER	{$$ = new EnumDefinition($1, $3);}
			;

ArgList:	ArgList COMMA VarType IDENTIFIER	{$$ = $1; $$->Add($3, $4);}
			| VarType IDENTIFIER	{$$ = new ArgList(); $$->Add($1, $2);}
			|	{$$ = new ArgList();}

Block:	LBRACE Statements RBRACE	{$$ = new Block($2);}
		;

Statements:	Statements Statement	{$$ = $1; $$->push_back($2);}	
			|	{$$ = new std::vector<Statement*>;}
			;

Statement:	Expression SEMI	{$$ = $1;}
			| IfStatement	{$$ = $1;}
			| ForStatement	{$$ = $1;}
			| WhileStatement	{$$ = $1;}
			| DoWhileStatement	{$$ = $1;}
			| BreakStatement	{$$ = $1;}
			| ContinueStatement	{$$ = $1;}
			| ReturnStatement	{$$ = $1;}
			| Block	{$$ = $1;}
			| VarDefinition	{$$ = $1;}
			| TypeDefinition	{$$ = $1;}
			| SEMI	{$$ = NULL;}
			;

IfStatement:IF LPAREN Expression RPAREN Statements ELSE Statements	{$$ = new IfStatement($3,$5,$7);   }
			| IF LPAREN Expression RPAREN Statements	{$$ = new IfStatement($3,$5,NULL);}
			;

ForStatement:	FOR LPAREN Statements SEMI Expression SEMI Expression RPAREN Statement	{$$ = new ForStatement($3,$5,$7,$9);}
				;

WhileStatement:	WHILE LPAREN Expression RPAREN Statement	{$$ = new WhileStatement($3,$5);}
			;

DoWhileStatement:	DO Statement WHILE LPAREN Expression RPAREN SEMI					{$$ = new DoWhileStatement($2,$5);}
			;

ContinueStatement:	CONTINUE SEMI	{$$ = new ContinueStatement();}
			;

BreakStatement:	BREAK SEMI	{$$ = new BreakStatement();}
			;

ReturnStatement:	RETURN SEMI	{$$ = new ReturnStatement(NULL);}
					| RETURN Expression SEMI	{$$ = new ReturnStatement($2);}
					;

Expression:	Expression LBRACKET Expression RBRACKET %prec ARW	{$$ = new GetItem($1,$3);}
			| IDENTIFIER LPAREN Args RPAREN	{$$ = new FunctionCall($1,$3);}
			| Expression DOT IDENTIFIER	{$$ = new Component($1,$3);}
			| Expression ARW IDENTIFIER	{$$ = new PtrComponent($1,$3);}
			| ADD Expression	%prec NOT	{$$ = new PositiveSign($2);}
			| SUB Expression	%prec NOT	{$$ = new NegativeSign($2);}
			| Expression DADD 	%prec ARW	{$$ = new Increment($1);}
			| Expression DSUB	%prec ARW	{$$ = new Decrement($1);}
			| MUL Expression	%prec NOT	{$$ = new ValueOf($2);}
			| BAND Expression	%prec NOT	{$$ = new AddressOf($2);}
			| NOT Expression	{$$ = new LogicNot($2);}
			| BNOT Expression	{$$ = new BitwiseNot($2);}
			| Expression DIV Expression	{$$ = new Div($1,$3);}
			| Expression MUL Expression	{$$ = new Mul($1,$3);} 
			| Expression MOD Expression	{$$ = new Mod($1,$3);}
			| Expression ADD Expression	{$$ = new Add($1,$3);} 
			| Expression SUB Expression	{$$ = new Sub($1,$3);} 
			| Expression GT Expression	{$$ = new GT($1,$3);} 
			| Expression GE Expression	{$$ = new GE($1,$3);} 
			| Expression LT Expression	{$$ = new LT($1,$3);} 
			| Expression LE Expression	{$$ = new LE($1,$3);} 
			| Expression EQ Expression	{$$ = new EQ($1,$3);} 
			| Expression NEQ Expression	{$$ = new NEQ($1,$3);} 
			| Expression BAND Expression	{$$ = new BitWiseAND($1,$3);}
			| Expression BXOR Expression	{$$ = new BitwiseXOR($1,$3);}
			| Expression BOR Expression	{$$ = new BitWiseOR($1,$3);} 
			| Expression AND Expression	{$$ = new AND($1,$3);} 
			| Expression OR Expression	{$$ = new OR($1,$3);} 
			| Expression QUES Expression COLON Expression	{$$ = new Conditional($1,$3,$5);}
			| Expression ASSIGN Expression	{$$ = new Assign($1,$3);} 
			| LPAREN Expression RPAREN	{$$ = $2;}
			| IDENTIFIER	{$$ = new Variable($1);} 
			| Constant	{$$ = $1;}
			;

Args:	Args COMMA Expression	{$$ = $1; $$->push_back($3);}
			| Expression %prec FUNC_CALL_ARG_LIST	{$$ = new std::vector<Expression*>;$$->push_back($1);}
			|	{$$ = new std::vector<Expression*>;}
			;

Constant:	TRUE	{$$ =  new Constant(true);}
			| FALSE	{$$ =  new Constant(false);}
			| CHARACTER	{$$ =  new Constant($1);}
			| INTEGER {$$ =  new Constant($1);}
			| REAL	{$$ =  new Constant($1);}
			| STRING {$$ = new StrVar($1);}
			;
%%