
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* "%code requires" blocks.  */

/* Line 1676 of yacc.c  */
#line 11 "CheshireParser.y"


#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#include "ParserNodes.h"




/* Line 1676 of yacc.c  */
#line 53 "CheshireParser.yy.h"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_ASSERT = 258,
     TOK_GLOBAL_VARIABLE = 259,
     TOK_CLASS = 260,
     TOK_INHERITS = 261,
     TOK_DEFINE_FUNCTION = 262,
     TOK_IF = 263,
     TOK_ELSE = 264,
     TOK_FOR = 265,
     TOK_RETURN = 266,
     TOK_SELF = 267,
     TOK_WHILE = 268,
     TOK_LPAREN = 269,
     TOK_RPAREN = 270,
     TOK_LBRACE = 271,
     TOK_RBRACE = 272,
     TOK_LBRACKET = 273,
     TOK_RBRACKET = 274,
     TOK_LSQUARE = 275,
     TOK_RSQUARE = 276,
     TOK_COMMA = 277,
     TOK_HAT = 278,
     TOK_PASS = 279,
     TOK_LAMBDA_PARAMS = 280,
     TOK_ACCESSOR = 281,
     TOK_SIZEOF = 282,
     TOK_SET = 283,
     TOK_INSTANCEOF = 284,
     TOK_NEW = 285,
     TOK_NEW_HEAP = 286,
     TOK_CAST = 287,
     TOK_LN = 288,
     TOK_NOT = 289,
     TOK_INCREMENT = 290,
     TOK_COMPARE = 291,
     TOK_AND_OR = 292,
     TOK_ADDSUB = 293,
     TOK_MULTDIV = 294,
     TOK_NUMBER = 295,
     TOK_RESERVED_TYPE = 296,
     TOK_RESERVED_LITERAL = 297,
     TOK_IDENTIFIER = 298,
     TOK_STRING = 299,
     TOK_BITSHIFT = 300,
     P_UMINUS = 301,
     P_CAST = 302,
     P_IF = 303,
     P_IFELSE = 304
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 30 "CheshireParser.y"

    char* string;
    OperationType op_type;
    ReservedType reserved_type;
    ReservedLiteral reserved_literal;
    double number;
    struct tagParameterList* parameter_list;
    struct tagExpressionNode* expression;
    struct tagExpressionList* expression_list;
    CheshireType cheshire_type;
    struct tagStatementNode* statement;
    struct tagBlockList* block_list;



/* Line 1676 of yacc.c  */
#line 135 "CheshireParser.yy.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




