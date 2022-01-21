%{
	#include "fparser.tab.hpp"
%}

%option noyywrap
%option yylineno

%%

(0|[1-9][0-9]*)		{
						yylval.num = new std::string(yytext);
						return NUMBER;
					}
\+					return ADDITION;
\(					return LPAREN;
\)					return RPAREN;
true				return TRUE;
false				return FALSE;
\|					return OR;
&					return AND;
!					return NOT;
.	

%%