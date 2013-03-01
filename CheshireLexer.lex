%{
#include <stdio.h>
#include <stdlib.h>
#include "ParserEnums.h"
#include "Structures.h"
//todo: move them to TypeSystem.h
#include "LexerUtil.h"
#include "CheshireParser.yy.h"

static int lineno = 0;
%}

%option header-file="CheshireLexer.yy.h"
%option warn nodefault
%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

ALPHA   [a-zA-Z]
DIGIT   [0-9]
ALPHANUMERIC    {ALPHA}|{DIGIT}
IDENTIFIER  {ALPHANUMERIC}|"$"|"_"
SIGN    ("+"|"-")?
WHITESPACE  [ \t]+
GRAPHICAL   [:graph:]|{WHITESPACE}|[\r\n]
QUOTE       "\""
BACKSLASH   "\\"

%%

"##"([^"#"]*|"#"[^"#"])*"##"  {} /*comment*/
"#"[^\n"#"]*                  {} /*comment*/
Number|Boolean|Int|Decimal|void|Object  {   yylval->cheshire_type = getType(getTypeKey(yytext), FALSE);
                                            return TOK_TYPE;
                                        }
infer     return TOK_INFER;
True|False|Null  { determineReservedLiteral(yytext, &(yylval->reserved_literal)); return TOK_RESERVED_LITERAL; }
"^"       return TOK_HAT;
pass      return TOK_PASS;
global    return TOK_GLOBAL_VARIABLE;
assert    return TOK_ASSERT;
class     return TOK_CLASS;
inherits  return TOK_INHERITS;
def       return TOK_DEFINE_FUNCTION;
if        return TOK_IF;
else      return TOK_ELSE;
for       return TOK_FOR;
return    return TOK_RETURN;
self      return TOK_SELF;
while     return TOK_WHILE;
cast      return TOK_CAST;
"("  return TOK_LPAREN;
")"  return TOK_RPAREN;
"{"  return TOK_LBRACE;
"}"  return TOK_RBRACE;
"["  return TOK_LBRACKET;
"]"  return TOK_RBRACKET;
","  return TOK_COMMA;
"::" return TOK_LAMBDA_PARAMS;
":"  return TOK_ACCESSOR;
{QUOTE}([^"\"""\n"]|{BACKSLASH}[abfnrtv"'"{QUOTE}{BACKSLASH}"?"])*{QUOTE}   { saveStringLiteral(yytext, &(yylval->string)); return TOK_STRING; }
"not"|"compl"    { determineOpType(yytext, &(yylval->op_type)); return TOK_NOT; }
"and"|"or"      { determineOpType(yytext, &(yylval->op_type)); return TOK_AND_OR; }
"++"|"--"       { determineOpType(yytext, &(yylval->op_type)); return TOK_INCREMENT; }
"sizeof"        { determineOpType(yytext, &(yylval->op_type)); return TOK_SIZEOF; }
["=""!"]"="|[">""<"]"="  { determineOpType(yytext, &(yylval->op_type)); return TOK_COMPARE; }
"<"             return TOK_LSQUARE;
">"             return TOK_RSQUARE;
"="             return TOK_SET;
"+"|"-"         { determineOpType(yytext, &(yylval->op_type)); return TOK_ADDSUB; }
"*"|"/"|"%"     { determineOpType(yytext, &(yylval->op_type)); return TOK_MULTDIV; }
"instanceof"    return TOK_INSTANCEOF;
"new"           return TOK_NEW;
"new"[" "]*"^"  return TOK_NEW_HEAP;
"delete"[" "]*"^"   return TOK_DELETE_HEAP;
0[xX][0-9A-F]+  { int x; sscanf(yytext, "%x", &x); yylval->number = (double) x; return TOK_NUMBER; }
0[0-7]+         { int x; sscanf(yytext, "%o", &x); yylval->number = (double) x; return TOK_NUMBER; }
{DIGIT}+("."{DIGIT}*)?([Ee]{SIGN}{DIGIT}+)?  { sscanf(yytext, "%lf", &(yylval->number)); return TOK_NUMBER; }
{ALPHA}{IDENTIFIER}*    {   if (isType(yytext)) {
                                yylval->cheshire_type = getType(getTypeKey(yytext), FALSE);
                                return TOK_TYPE;
                            } else {
                                saveIdentifier(yytext, &(yylval->string));
                                return TOK_IDENTIFIER;
                            }
                        }
"."             return TOK_LN; 
{WHITESPACE}+   {} /* whitespace */
\n              lineno++;
.               { fprintf(stderr, "No such character as \'%s\' at line %d.\n", yytext, lineno); exit(0); }

%%

int yyerror(yyscan_t scanner, ExpressionNode** expression, const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    return 0;
}
