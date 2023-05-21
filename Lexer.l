%{
#include <stdio.h>
#include <string>
#include <iostream>
#include "AST.h"
#include "Parser.tab.hpp"

extern "C" int yylex();

char Escape2Char(char ch){
	switch(ch){
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	case 'v': return '\v';
	case '\\': return '\\';
	case '\'': return '\'';
	case '\"': return '\"';
	default:
		if ('0'<=ch && ch<='9')
			return (char)(ch-'0');
		else
			return ch;
	}
}
%}

%option outfile="Lexer.cpp"

%%
","	return COMMA;
"." return DOT;
";"														{return SEMI;}
"("														{return LPAREN;}
")"														{return RPAREN;}
"["														{return LBRACKET;}
"]"														{return RBRACKET;}
"{"														{return LBRACE;}
"}"														{return RBRACE;}
"=="													{return EQ;}
">="													{return GE;}
">"														{return GT;}
"<="													{return LE;}
"<"														{return LT;}
"!="													{return NEQ;}
"!"														{return NOT;}
"="														{return ASSIGN;}
"&&"													{return AND;}
"&"														{return BAND;}
"||"													{return OR;}
"|"														{return BOR;}
"->"													{return ARW;}
"^"														{return BXOR;}
"~"														{return BNOT;}
"++"													{return DADD;}
"+"														{return ADD;}
"--"													{return DSUB;}
"-"														{return SUB;}
"*"														{return MUL;}
"/"														{return DIV;}
"%"														{return MOD;}
"?"														{return QUES;}
":"														{return COLON;}
"struct"												{return STRUCT;}
"enum"													{return ENUM;}
"if"													{return IF;}
"else"													{return ELSE;}
"for"													{return FOR;}
"while"													{return WHILE;}
"do"													{return DO;}
"continue"												{return CONTINUE;}
"return"												{return RETURN;}
"true"													{return TRUE; }
"false"													{return FALSE; }
"break"													{return BREAK; }
"bool"													{return BOOL; }
"short"													{return SHORT; }    //
"int"													{return INT; }      //
"long"													{return LONG; }
"char"													{return CHAR; }
"float"													{return FLOAT; }
"double"												{return DOUBLE; }   //
"void"													{return VOID; }
[ \t\n]													{ ; }
'\\.'													{ 
															yylval.cVal = Escape2Char(yytext[2]);
															return CHARACTER; 
														}
'[^.\\]'												{ 
															yylval.cVal = yytext[1];
															return CHARACTER; 
														}
\"(\\.|[^"\\])*\"										{
															yylval.sVal = new std::string("");
															for (int i = 1; i <= yyleng-2; i++)
																if (yytext[i] == '\\'){
																	i++;
																	yylval.sVal->push_back(Escape2Char(yytext[i]));
																}else{
																	yylval.sVal->push_back(yytext[i]);
																}
															return STRING;
														}
[a-zA-Z_][a-zA-Z0-9_]*									{ 
															yylval.sVal = new std::string(yytext, yyleng);
															return IDENTIFIER; 
														} 
[0-9]+\.[0-9]+											{ 
															double dtmp;
															sscanf(yytext, "%lf", &dtmp);
															yylval.dVal = dtmp;
															return REAL; 
														}
[0-9]+													{
															int itmp;
															sscanf(yytext, "%d", &itmp);
															yylval.iVal = itmp; 
															return INTEGER; 
														}
%%