%{

#include "ParserEnums.h"
#include "CheshireParser.yy.h"
#include "CheshireLexer.yy.h"

int yyerror(yyscan_t scanner, ExpressionNode** expression, const char* msg);

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
    double number;
    struct tagParameterList* parameter_list;
    struct tagExpressionNode* expression;
    struct tagExpressionList* expression_list;
    CheshireType cheshire_type;
    struct tagStatementNode* statement;
    struct tagBlockList* block_list;
}

%token TOK_ASSERT
%token TOK_GLOBAL_VARIABLE
%token TOK_CLASS
%token TOK_INHERITS
%token TOK_DEFINE_FUNCTION
%token TOK_IF
%token TOK_ELSE
%token TOK_FOR
%token TOK_RETURN
%token TOK_SELF
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
%token TOK_HAT
%token TOK_PASS
%token TOK_LAMBDA_PARAMS
%token TOK_ACCESSOR
%token TOK_SET
%token TOK_INSTANCEOF
%token TOK_NEW
%token TOK_NEW_HEAP
%token TOK_DELETE_HEAP
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
%token <number> TOK_NUMBER
%token <cheshire_type> TOK_TYPE
%token <reserved_literal> TOK_RESERVED_LITERAL
%token <string> TOK_IDENTIFIER
%token <string> TOK_STRING

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
%left TOK_ACCESSOR TOK_LBRACKET TOK_RBRACKET
%nonassoc P_IF
%nonassoc TOK_ELSE
%nonassoc TOK_LPAREN TOK_RPAREN

%type <expression> expression
%type <expression> lval_expression
%type <expression> expression_statement
%type <statement> statement
%type <statement> statement_or_pass
%type <block_list> block_or_pass
%type <block_list> block
%type <block_list> block_contains
%type <expression_list> expression_list
%type <expression_list> expression_list_contains
%type <parameter_list> parameter_list
%type <parameter_list> parameter_list_contains
%type <cheshire_type> typename

%%

input
    : TOK_DEFINE_FUNCTION typename TOK_IDENTIFIER parameter_list TOK_LN  { *output = createMethodDeclaration( $2 , $3 , $4 ); YYACCEPT; }
    | TOK_DEFINE_FUNCTION typename TOK_IDENTIFIER parameter_list block_or_pass  { *output = createMethodDefinition( $2 , $3 , $4 , $5 ); YYACCEPT; }
    ;

parameter_list
    : TOK_LPAREN parameter_list_contains TOK_RPAREN  { $$ = $2 ; }
    | TOK_LPAREN TOK_RPAREN  { $$ = NULL; }
    ;

parameter_list_contains
    : typename TOK_IDENTIFIER  { $$ = linkParameterList( $1 , $2 , NULL ); }
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
    | TOK_DELETE_HEAP expression TOK_LN  { $$ = createDeleteHeapStatement( $2 ); }
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
    | TOK_RESERVED_LITERAL  { $$ = createReservedLiteralNode( $1 ); }
    | TOK_STRING  { $$ = createStringNode( $1 ); }
    | TOK_LPAREN expression TOK_RPAREN { $$ = $2 ; }
    | TOK_NUMBER  { $$ = createNumberNode( $1 ); }
    | TOK_SELF  { $$ = createSelfNode(); }
    | TOK_NOT expression  { $$ = createUnaryOperation( $1 , $2 ); }
    | TOK_ADDSUB expression  %prec P_UMINUS { $$ = createUnaryOperation( $1 , $2 ); }
    | expression TOK_COMPARE expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_LSQUARE expression  { $$ = createBinOperation( OP_LESS , $1 , $3 ); }
    | expression TOK_RSQUARE expression  { $$ = createBinOperation( OP_GREATER , $1 , $3 ); }
    | expression TOK_AND_OR expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_ADDSUB expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_MULTDIV expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_INSTANCEOF typename  { $$ = createInstanceOfNode( $1 , $3 ); }
    | TOK_CAST TOK_LSQUARE typename TOK_RSQUARE expression %prec P_CAST  { $$ = createCastOperation( $5 , $3 ); }
    | TOK_NEW TOK_TYPE expression_list   { $$ = createInstantiationOperation( IT_GC , $2 , $3 ); }
    | TOK_NEW_HEAP TOK_TYPE expression_list  { $$ = createInstantiationOperation( IT_HEAP , $2 , $3 ); }
    | TOK_LEN expression  { $$ = createLengthOperation( $2 ); }
    ;

lval_expression
    : TOK_IDENTIFIER  { $$ = createVariableAccess( $1 ); }
    | expression TOK_LBRACKET expression TOK_RBRACKET  { $$ = createBinOperation( OP_ARRAY_ACCESS , $1 , $3 ); }
    | expression TOK_ACCESSOR TOK_IDENTIFIER { $$ = createAccessNode( $1 , $3 ); }
    ;

expression_statement
    : lval_expression TOK_SET expression  { $$ = createBinOperation( OP_SET , $1 , $3 ); }
    | lval_expression TOK_INCREMENT  { $$ = createIncrementOperation( $1 , $2 ); }
    | TOK_IDENTIFIER expression_list  { $$ = createMethodCall( $1 , $2 ); }
    | expression TOK_ACCESSOR TOK_IDENTIFIER expression_list  { $$ = createObjectCall( $1 , $3 , $4 ); }
    | expression TOK_ACCESSOR expression_list  { $$ = createCallbackCall( $1 , $3 ); }
    ;

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
    | typename TOK_LAMBDA_PARAMS parameter_list  { $$ = getType( getLambdaTypeKey( $1 , $3 ), FALSE ); deleteParameterList( $3 ); }
    | typename TOK_HAT  {  if ( $1.arrayNesting > 0 )
                               PANIC("Cannot apply ^ to an array typename!");
                           if ( $1.isUnsafe ) 
                               PANIC("Cannot apply ^ to a typename more than one time!");
                           $$ = getType( $1.typeKey , TRUE );
                        }
    | typename TOK_LBRACKET TOK_RBRACKET  { $$ = $1; $$.arrayNesting++; }
    ;

%%
