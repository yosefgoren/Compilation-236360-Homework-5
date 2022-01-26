/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_HPP_INCLUDED
# define YY_YY_PARSER_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INT = 258,                     /* INT  */
    VOID = 259,                    /* VOID  */
    BYTE = 260,                    /* BYTE  */
    B = 261,                       /* B  */
    BOOL = 262,                    /* BOOL  */
    CONST = 263,                   /* CONST  */
    TRUE = 264,                    /* TRUE  */
    FALSE = 265,                   /* FALSE  */
    RETURN = 266,                  /* RETURN  */
    WHILE = 267,                   /* WHILE  */
    BREAK = 268,                   /* BREAK  */
    CONTINUE = 269,                /* CONTINUE  */
    SC = 270,                      /* SC  */
    COMMA = 271,                   /* COMMA  */
    ID = 272,                      /* ID  */
    NUM = 273,                     /* NUM  */
    STRING = 274,                  /* STRING  */
    ASSIGN = 275,                  /* ASSIGN  */
    OR = 276,                      /* OR  */
    AND = 277,                     /* AND  */
    RELOP = 278,                   /* RELOP  */
    LOW_PRIO_BINOP = 279,          /* LOW_PRIO_BINOP  */
    HIGH_PRIO_BINOP = 280,         /* HIGH_PRIO_BINOP  */
    NOT = 281,                     /* NOT  */
    LPAREN = 282,                  /* LPAREN  */
    RPAREN = 283,                  /* RPAREN  */
    LBRACE = 284,                  /* LBRACE  */
    RBRACE = 285,                  /* RBRACE  */
    IF = 286,                      /* IF  */
    ELSE = 287                     /* ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 126 "parser.ypp"

	//lexer proivided fields:
	string* id;
	string* string_literal;
	int number_literal;
	
	//parser metadata fields:
	int line_number;
	bool is_const;
	
	Binop binop;
	Relop relop;
	ExpType exp_type;

	std::vector<Expression*>* exp_list;
	std::vector<Parameter>* formals_list;
	std::string* label;
	
	Expression* expression;
	BranchBlock* branch_block;
	RunBlock* run_block;
	NumericExp* numeric_exp;
	Parameter* formal;
	FuncDecl* func_decl;

	DecInfo dec_info;

#line 124 "parser.tab.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_HPP_INCLUDED  */
