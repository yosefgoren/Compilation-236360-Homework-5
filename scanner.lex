%{
	#include "hw3_output.hpp"
	#include "Symtab.hpp"
	#include "parser.tab.hpp"
%}

%option noyywrap
%option yylineno

number				(0|[1-9][0-9]*)
string				(\"([^\n\r\"\\]|\\[rnt\"\\])+\")
comment				(\/\/[^\r\n]*((\r)|(\n)|(\r\n)))
whitespace			[\t\r\n ]
div_symbol			[/]

%%
void							return VOID;
int								return INT;
byte							return BYTE;
b								return B;
bool							return BOOL;
const							return CONST;
and								return AND;
or								return OR;
not								return NOT;
true							return TRUE;
false							return FALSE;
return							return RETURN;
if								return IF;
else							return ELSE;
while							return WHILE;
break							return BREAK;
continue						return CONTINUE;
;								return SC;
,								return COMMA;
\(								return LPAREN;
\)								return RPAREN;
\{								return LBRACE;
\}								return RBRACE;
=								return ASSIGN;

(==)							{
									yylval.relop = EQUAL;
									return RELOP;
								}
(!=)							{
									yylval.relop = NOT_EQUAL;
									return RELOP;
								}
(<)								{
									yylval.relop = LESS;
									return RELOP;
								}
(>)								{
									yylval.relop = GREATER;
									return RELOP;
								}
(<=)							{
									yylval.relop = LESS_EQUAL;
									return RELOP;
								}
(>=)							{
									yylval.relop = GREATER_EQUAL;
									return RELOP;
								}

(\+)							{
									yylval.binop = PLUS;
									return BINOP;
								}
(\-)							{
									yylval.binop = MINUS;
									return BINOP;
								}
(\*)							{
									yylval.binop = MULT;
									return BINOP;
								}
{div_symbol}					{
									yylval.binop = DIV;
									return BINOP;
								}
[a-zA-Z][a-zA-Z0-9]*			{
									yylval.id = new string(yytext);
									return ID;
								}
{number}						{
									yylval.number_literal = atoi(yytext);
									return NUM;
								}
{string}						{
									yylval.string_literal = new string(yytext);
									return STRING;
								}
{comment}						;
{whitespace}					;
.								{
									output::errorLex(yylineno);
									exit(420);
								}

%%