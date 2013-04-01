%{
#include <stdio.h>
#include <stdlib.h>
#include "ParserEnums.h"
#include "Structures.h"
#include "LexerUtilities.h"
#include "CheshireParser.yy.h"

static int lineno = 0;
%}

%option header-file="CheshireLexer.yy.h"
%option warn nodefault
%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

ALPHA   [a-zA-Z]
IDENTIFIER_START    [a-zA-Z_]
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
infer     return TOK_INFER;
True|False|Null  { determineReservedLiteral(yytext, &(yylval->reserved_literal)); return TOK_RESERVED_LITERAL; }
external  return TOK_EXTERNAL;
pass      return TOK_PASS;
global    return TOK_GLOBAL;
assert    return TOK_ASSERT;
class     return TOK_CLASS;
inherits  return TOK_INHERITS;
def       return TOK_DEFINE;
if        return TOK_IF;
else      return TOK_ELSE;
for       return TOK_FOR;
return    return TOK_RETURN;
while     return TOK_WHILE;
cast      return TOK_CAST;
len       return TOK_LEN;
"("  return TOK_LPAREN;
")"  return TOK_RPAREN;
"{"  return TOK_LBRACE;
"}"  return TOK_RBRACE;
"["  return TOK_LBRACKET;
"]"  return TOK_RBRACKET;
","  return TOK_COMMA;
":"  return TOK_COLON;
"::" return TOK_COLONCOLON;
{QUOTE}([^"\"""\n"]|{BACKSLASH}[abfnrtv"'"{QUOTE}{BACKSLASH}"?"])*{QUOTE}   { saveStringLiteral(yytext, &(yylval->string)); return TOK_STRING; }
"not"|"compl"    { determineOpType(yytext, &(yylval->op_type)); return TOK_NOT; }
"and"|"or"      { determineOpType(yytext, &(yylval->op_type)); return TOK_AND_OR; }
"++"|"--"       { determineOpType(yytext, &(yylval->op_type)); return TOK_INCREMENT; }
["=""!"]"="|[">""<"]"="  { determineOpType(yytext, &(yylval->op_type)); return TOK_COMPARE; }
"<"             return TOK_LSQUARE;
">"             return TOK_RSQUARE;
"="             return TOK_SET;
"+"|"-"         { determineOpType(yytext, &(yylval->op_type)); return TOK_ADDSUB; }
"*"|"/"|"%"     { determineOpType(yytext, &(yylval->op_type)); return TOK_MULTDIV; }
"instanceof"    return TOK_INSTANCEOF;
"new"           return TOK_NEW;
"delete"        return TOK_DELETE;
[1-9]{DIGIT}*   { long x; sscanf(yytext, "%ld", &x); yylval->integer = x; return TOK_INTEGER; }
0[xX][0-9A-F]+  { long x; sscanf(yytext, "%lx", &x); yylval->integer = x; return TOK_INTEGER; }
0[0-7]*         { long x; sscanf(yytext, "%lo", &x); yylval->integer = x; return TOK_INTEGER; }
{DIGIT}+("."{DIGIT}+)?([Ee]{SIGN}{DIGIT}+)?  { sscanf(yytext, "%lf", &(yylval->decimal)); return TOK_DECIMAL; }
"."             return TOK_LN;
{IDENTIFIER_START}{IDENTIFIER}* {   if (isTypeName(yytext)) {
                                        yylval->cheshire_type = getNamedType(yytext);
                                        return TOK_TYPE;
                                    } else {
                                        saveIdentifier(yytext, &(yylval->string));
                                        return TOK_IDENTIFIER;
                                    }
                                }
{WHITESPACE}+   {} /* whitespace */
\n              lineno++;
<<eof>>         return TOK_EOF;
.               { fprintf(stderr, "No such character as \'%s\' at line %d.\n", yytext, lineno); exit(0); }

%%

int yyerror(yyscan_t scanner, ExpressionNode** expression, const char* msg) {
    fprintf(stderr, "Error: %s at line %d\n", msg, lineno);
    return 0;
}
