%{

#include "ParserEnums.h"
#include "LexerUtilities.h"
#include "CheshireParser.yy.h"
#include "CheshireLexer.yy.h"

int yyerror(yyscan_t scanner, ExpressionNode** expression, const char* msg);

static int dummyIncrement = 0;
/* come up with an arbitrary "dummy name", such as __dummy_param_1 or __var_2 that should be unique across the file. */
char* createDummyName(const char* prefix) {
    int dummyID = dummyIncrement++;
    char temp[100];
    sprintf(temp, "__%s_%d", prefix, dummyID);
    temp[99] = '\0';

    int stringlen = strlen(temp);
    char* cpystring = malloc(sizeof(char) * (stringlen+1));
    memcpy(cpystring, temp, stringlen);
    cpystring[stringlen] = '\0';
    return cpystring;
}

%}

%code requires {

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#include "Structures.h"
#include "ParserNodes.h"
#include "TypeSystem.h"

}

%defines "CheshireParser.yy.h"

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { ParserTopNode** output }
%parse-param { yyscan_t scanner }
%expect 0

%union {
    char* string;
    OperationType op_type;
    ReservedLiteral reserved_literal;
    double decimal;
    int64_t integer;
    char character;
    struct tagParameterList* parameter_list;
    struct tagExpressionNode* expression;
    struct tagExpressionList* expression_list;
    CheshireType cheshire_type;
    struct tagStatementNode* statement;
    struct tagBlockList* block_list;
    struct tagClassList* class_list;
    struct tagUsingList* using_list;
}

%token TOK_EOF
%token TOK_USING
%token TOK_FWDECL
%token TOK_ASSERT
%token TOK_GLOBAL
%token TOK_CLASS
%token TOK_INHERITS
%token TOK_DEFINE
%token TOK_IF
%token TOK_ELSE
%token TOK_FOR
%token TOK_RETURN
%token TOK_WHILE
%token TOK_LPAREN
%token TOK_RPAREN
%token TOK_LBRACE
%token TOK_RBRACE
%token TOK_LBRACKET
%token TOK_RBRACKET
%token TOK_LSQUARE
%token TOK_RSQUARE
%token TOK_COMMA
%token TOK_EXTERNAL
%token TOK_PASS
%token TOK_COLON
%token TOK_COLONCOLON
%token TOK_SET
%token TOK_INSTANCEOF
%token TOK_NEW
%token TOK_NEW_HEAP
%token TOK_DELETE
%token TOK_CAST
%token TOK_LN
%token TOK_INFER
%token TOK_LEN
%token <op_type> TOK_NOT
/* Note: TOK_NOT is used for "not" and "compl" operations, like TOK_ADDSUB, etc.*/
%token <op_type> TOK_INCREMENT
%token <op_type> TOK_COMPARE
%token <op_type> TOK_AND_OR
%token <op_type> TOK_ADDSUB
%token <op_type> TOK_MULTDIV
%token <integer> TOK_INTEGER
%token <decimal> TOK_DECIMAL
%token <cheshire_type> TOK_TYPE
%token <reserved_literal> TOK_RESERVED_LITERAL
%token <string> TOK_IDENTIFIER
%token <string> TOK_STRING
%token <character> TOK_CHAR

%right TOK_COMMA
%right TOK_SET
%left TOK_AND_OR
%left TOK_COMPARE TOK_INSTANCEOF TOK_LSQUARE TOK_RSQUARE
%left TOK_ADDSUB
%left TOK_MULTDIV
%left P_UMINUS
%nonassoc TOK_NEW TOK_NEW_HEAP TOK_LEN
%left TOK_NOT
%nonassoc P_CAST
%left TOK_INCREMENT
%left TOK_COLON TOK_COLONCOLON TOK_LBRACKET TOK_RBRACKET
%nonassoc P_IF
%nonassoc TOK_ELSE
%nonassoc TOK_LPAREN TOK_RPAREN

%type <string> possible_objectname
%type <expression> expression
%type <expression> lval_expression
%type <expression> expression_statement
%type <statement> statement
%type <statement> statement_or_pass
%type <block_list> block
%type <block_list> block_contains
%type <block_list> block_or_pass
%type <expression_list> expression_list
%type <expression_list> expression_list_contains
%type <parameter_list> parameter_list
%type <parameter_list> parameter_list_contains
%type <class_list> class_list
%type <class_list> class_list_contains
%type <class_list> class_list_or_pass
%type <cheshire_type> typename
%type <using_list> using_list
%type <using_list> using_list_contains

%%

input
    : TOK_EXTERNAL TOK_DEFINE typename TOK_IDENTIFIER parameter_list TOK_LN  { *output = createMethodDeclaration( $3 , $4 , $5 ); YYACCEPT; }
    | TOK_FWDECL possible_objectname TOK_LN  { reserveClassNameType( $2 ); *output = NULL; YYACCEPT; }
    | TOK_DEFINE typename TOK_IDENTIFIER parameter_list block_or_pass  { *output = createMethodDefinition( $2 , $3 , $4 , $5 ); YYACCEPT; }
    | TOK_EXTERNAL typename TOK_IDENTIFIER TOK_LN  { *output = createGlobalVariableDeclaration( $2 , $3 ); YYACCEPT; }
    | TOK_GLOBAL typename TOK_IDENTIFIER TOK_LN  { *output = createGlobalVariableDefinition( $2 , $3 ); YYACCEPT; }
    | TOK_CLASS possible_objectname class_list_or_pass  { CheshireType object = getNamedType("Object"); *output = createClassDefinition( $2 , $3 , object ); YYACCEPT; }
    | TOK_CLASS possible_objectname TOK_INHERITS typename class_list_or_pass  { *output = createClassDefinition( $2 , $5 , $4 ); YYACCEPT; }
    | TOK_EOF  { return -2; }
    ;

possible_objectname
    : TOK_IDENTIFIER  { $$ = $1 ; }
    | typename  { $$ = getNamedTypeString( $1 ); }
    ;

class_list_or_pass
    : TOK_PASS  { $$ = NULL; }
    | class_list  { $$ = $1; }
    ;

class_list
    : TOK_LBRACE class_list_contains  { $$ = $2; }
    ;

class_list_contains
    : typename TOK_IDENTIFIER TOK_SET expression TOK_LN class_list_contains  { $$ = linkClassVariable( $1 , $2 , $4 , $6 ); }
    | TOK_DEFINE typename TOK_IDENTIFIER parameter_list block_or_pass class_list_contains  { $$ = linkClassMethod( $2 , $4 , $3 , $5 , $6 ); }
    | TOK_DEFINE TOK_NEW parameter_list TOK_INHERITS expression_list block_or_pass class_list_contains { $$ = linkClassConstructor( $3 , $5, $6 , $7 ); }
    | TOK_DEFINE TOK_NEW parameter_list block_or_pass class_list_contains { $$ = linkClassConstructor( $3 , NULL , $4 , $5 ); }
    | TOK_RBRACE  { $$ = NULL; }
    ;

parameter_list
    : TOK_LPAREN parameter_list_contains TOK_RPAREN  { $$ = $2 ; }
    | TOK_LPAREN TOK_RPAREN  { $$ = NULL; }
    ;

parameter_list_contains
    : typename  { $$ = linkParameterList( $1 , createDummyName("param") , NULL ); }
    | typename TOK_IDENTIFIER  { $$ = linkParameterList( $1 , $2 , NULL ); }
    | typename TOK_COMMA parameter_list_contains  { $$ = linkParameterList( $1 , createDummyName("param") , $3 ); }
    | typename TOK_IDENTIFIER TOK_COMMA parameter_list_contains  { $$ = linkParameterList( $1 , $2 , $4 ); }
    ;

block_or_pass
    : TOK_PASS  { $$ = NULL; }
    | block  { $$ = $1; }
    ;

statement
    : expression_statement TOK_LN  { $$ = createExpressionStatement( $1 ); }
    | TOK_ASSERT expression TOK_LN  { $$ = createAssertionStatement( $2 ); }
    | typename TOK_IDENTIFIER TOK_SET expression TOK_LN  { $$ = createVariableDefinition( $1 , $2 , $4 ); }
    | TOK_INFER TOK_IDENTIFIER TOK_SET expression TOK_LN  { $$ = createInferDefinition( $2 , $4 ); }
    | block  { $$ = createBlockStatement( $1 ); }
    | TOK_IF TOK_LPAREN expression TOK_RPAREN statement_or_pass TOK_ELSE statement_or_pass %prec P_IFELSE  { $$ = createIfElseStatement( $3 , $5 , $7 ); }
    | TOK_IF TOK_LPAREN expression TOK_RPAREN statement_or_pass %prec P_IF  { $$ = createIfStatement( $3 , $5 ); }
    | TOK_WHILE TOK_LPAREN expression TOK_RPAREN statement_or_pass  { $$ = createWhileStatement( $3 , $5 ); }
    | TOK_RETURN expression TOK_LN  { $$ = createReturnStatement( $2 ); }
    | TOK_RETURN TOK_LN  { $$ = createReturnStatement( createReservedLiteralNode(RL_NULL) ); }
    ;

statement_or_pass
    : TOK_PASS  { $$ = createBlockStatement(NULL); }
    | statement  { $$ = $1 ; }
    ;

block
    : TOK_LBRACE block_contains  { $$ = $2 ; }
    ;

block_contains
    : statement block_contains  { $$ = linkBlockList( $1 , $2 );  }
    | TOK_RBRACE  { $$ = NULL; }
    ;

expression
    : expression_statement  { $$ = $1 ; }
    | lval_expression  { $$ = dereferenceExpression( $1 ); }
    | TOK_INTEGER  { $$ = createIntegerNode( $1 ); }
    | TOK_DECIMAL  { $$ = createDecimalNode( $1 ); }
    | TOK_CHAR  { $$ = createCharNode( $1 ); }
    | TOK_RESERVED_LITERAL  { $$ = createReservedLiteralNode( $1 ); }
    | TOK_STRING  { $$ = createStringNode( $1 ); }
    | TOK_LPAREN expression TOK_RPAREN { $$ = $2 ; }
    | TOK_NOT expression  { $$ = createUnaryOperation( $1 , $2 ); }
    | TOK_ADDSUB expression  %prec P_UMINUS { $$ = createUnaryOperation( $1 , $2 ); }
    | expression TOK_COMPARE expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_LSQUARE expression  { $$ = createBinOperation( OP_LESS , $1 , $3 ); }
    | expression TOK_RSQUARE expression  { $$ = createBinOperation( OP_GREATER , $1 , $3 ); }
    | expression TOK_AND_OR expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_ADDSUB expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_MULTDIV expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_INSTANCEOF typename  { $$ = createInstanceOfNode( $1 , $3 ); }
    | typename TOK_LPAREN expression TOK_RPAREN  { $$ = createCastOperation( $3 , $1 ); }
    | TOK_NEW TOK_TYPE expression_list  { $$ = createInstantiationOperation( $2 , $3 ); }
    | TOK_DEFINE typename TOK_COLON parameter_list TOK_USING using_list block_or_pass  { $$ = createClosureNode( $2 , $4 , $6, $7 ); }
    | TOK_DEFINE typename TOK_COLON parameter_list block_or_pass  { $$ = createClosureNode( $2 , $4 , NULL , $5 ); }
    ;

lval_expression
    : TOK_IDENTIFIER  { $$ = createVariableAccess( $1 ); }
    | expression TOK_LBRACKET expression TOK_RBRACKET  { $$ = createBinOperation( OP_ARRAY_ACCESS , $1 , $3 ); }
    | expression TOK_COLON TOK_IDENTIFIER { $$ = createAccessNode( $1 , $3 ); }
    ;

expression_statement
    : lval_expression TOK_SET expression  { $$ = createBinOperation( OP_SET , $1 , $3 ); }
    | lval_expression TOK_INCREMENT  { $$ = createIncrementOperation( $1 , $2 ); }
    | expression expression_list  { $$ = createMethodCall( $1 , $2 ); }
    | expression TOK_COLONCOLON TOK_IDENTIFIER expression_list  { $$ = createObjectCall( $1 , $3 , $4 ); }
    ;

using_list
    : TOK_LPAREN using_list_contains TOK_RPAREN  { $$ = $2 ; }
    | TOK_LPAREN TOK_RPAREN  { $$ = NULL; }
    ;

using_list_contains
    : TOK_IDENTIFIER TOK_COMMA using_list_contains  { $$ = linkUsingList( $1 , $3 ); }
    | TOK_IDENTIFIER  { $$ = linkUsingList( $1 , NULL ); }

expression_list
    : TOK_LPAREN expression_list_contains TOK_RPAREN  { $$ = $2 ; }
    | TOK_LPAREN TOK_RPAREN  { $$ = NULL; }
    ;

expression_list_contains
    : expression  { $$ = linkExpressionList( $1 , NULL ); }
    | expression TOK_COMMA expression_list_contains  { $$ = linkExpressionList( $1 , $3 ); }
    ;

typename
    : TOK_TYPE  { $$ = $1 ; }
    | typename TOK_COLON parameter_list  { $$ = getLambdaType( $1 , $3 ); deleteParameterList( $3 ); }
    | typename TOK_LBRACKET TOK_RBRACKET  { $$ = $1; $$.arrayNesting++; }
    ;

%%
