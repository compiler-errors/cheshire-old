%{
#include "Enums.h"
#include "Nodes.h"
#include "CheshireParser.yy.h"
#include "CheshireLexer.yy.h"

int yyerror(yyscan_t scanner, ExpressionNode** expression, const char* msg);
%}

%code requires {

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

}

%defines "CheshireParser.yy.h"

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { ExpressionNode** expression }
%parse-param { yyscan_t scanner }
%expect 0

%union {
    char* string;
    OperationType op_type;
    ReservedType reserved_type;
    ReservedLiteral reserved_literal;
    double number;
    struct tagExpressionNode* expression;
    struct tagParameterList* parameter_list;
    struct tagInternalTypeNode* type_node;
    struct tagStatementNode* statement;
    struct tagBlockList* block_list;
}

%token TOK_ASSERT
%token TOK_GLOBAL_VARIABLE
%token TOK_CLASS
%token TOK_INHERITS
%token TOK_DEFINE_FUNCTION
%token TOK_ALIAS
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
%token TOK_ACCESSOR
%token TOK_SIZEOF
%token TOK_SET
%token TOK_INSTANCEOF
%token TOK_NEW
%token TOK_NEW_HEAP
%token TOK_CAST
%token TOK_LN
%token <op_type> TOK_NOT
/* Note: TOK_NOT is used for "not" and "compl" operations, like TOK_ADDSUB, etc.*/
%token <op_type> TOK_INCREMENT
%token <op_type> TOK_COMPARE
%token <op_type> TOK_AND_OR
%token <op_type> TOK_ADDSUB
%token <op_type> TOK_MULTDIV
%token <number> TOK_NUMBER
%token <reserved_type> TOK_RESERVED_TYPE
%token <reserved_literal> TOK_RESERVED_LITERAL
%token <string> TOK_IDENTIFIER
%token <string> TOK_STRING

%right TOK_COMMA
%right TOK_SET
%left TOK_AND_OR
%left TOK_COMPARE TOK_INSTANCEOF TOK_LSQUARE TOK_RSQUARE
%left TOK_BITSHIFT
%left TOK_ADDSUB
%left TOK_MULTDIV
%left P_UMINUS
%left TOK_NEW TOK_NEW_HEAP TOK_SIZEOF
%left TOK_NOT
%nonassoc P_CAST
%nonassoc TOK_INCREMENT
%left TOK_ACCESSOR TOK_LBRACKET TOK_RBRACKET
%nonassoc P_IF
%nonassoc TOK_ELSE
%nonassoc TOK_LPAREN TOK_RPAREN

%type <expression> expression
%type <expression> expression_statement
%type <statement> statement
%type <block_list> block
%type <block_list> block_contains
%type <parameter_list> parameter_list
%type <parameter_list> parameter_list_contains
%type <type_node> typename

%%

input
    : expression  { *expression = $1 ; }
    | statement  { }
    ;

statement
    : expression_statement TOK_LN  { $$ = createExpressionStatement( $1 ); }
    | block  { $$ = createBlockStatement( $1 ); }
    | TOK_IF TOK_LPAREN expression TOK_RPAREN statement TOK_ELSE statement %prec P_IFELSE  { $$ = createIfElseStatement( $3 , $5 , $7 ); }
    | TOK_IF TOK_LPAREN expression TOK_RPAREN statement %prec P_IF  { $$ = createIfStatement( $3 , $5 ); }
    | TOK_WHILE TOK_LPAREN expression TOK_RPAREN statement  { $$ = createWhileStatement( $3 , $5 ); }
    ;

block
    : TOK_LBRACE block_contains  { $$ = $2 ; } 
    ;

block_contains
    : statement block  { $$ = linkBlockList( $1 , $2 );  }
    | TOK_RBRACE  { $$ = NULL; }
    ;

expression
    : expression_statement  { $$ = $1 ; }
    | TOK_RESERVED_LITERAL  { $$ = createReservedLiteralNode( $1 ); }
    | TOK_STRING  { $$ = createStringNode( $1 ); }
    | TOK_LPAREN expression TOK_RPAREN { $$ = $2 ; }
    | expression TOK_LBRACKET expression TOK_RBRACKET  { $$ = createBinOperation( OP_ARRAY_ACCESS , $1 , $3 ); }
    | TOK_IDENTIFIER  { $$ = createVariableAccess( $1 ); }
    | TOK_NUMBER  { $$ = createNumberNode( $1 ); }
    | TOK_SELF  { $$ = createSelfNode(); }
    | TOK_NOT expression  { $$ = createUnaryOperation( $1 , $2 ); }
    | TOK_SIZEOF expression  { $$ = createSizeOfExpression( $2 ); }
    | TOK_SIZEOF TOK_LSQUARE typename TOK_RSQUARE  { $$ = createSizeOfTypeExpression( $3 ); }
    | TOK_ADDSUB expression  %prec P_UMINUS { $$ = createUnaryOperation( $1 , $2 ); }
    | expression TOK_COMPARE expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_LSQUARE expression  { $$ = createBinOperation( OP_LESS , $1 , $3 ); }
    | expression TOK_RSQUARE expression  { $$ = createBinOperation( OP_GREATER , $1 , $3 ); }
    | expression TOK_AND_OR expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_ADDSUB expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_MULTDIV expression  { $$ = createBinOperation( $2 , $1 , $3 ); }
    | expression TOK_INSTANCEOF typename  { $$ = createInstanceOfNode( $1 , $3 ); }
    | TOK_CAST TOK_LSQUARE typename TOK_RSQUARE expression %prec P_CAST  { $$ = createCastOperation( $5 , $3 ); }
    | TOK_NEW TOK_IDENTIFIER parameter_list   { $$ = createInstantiationOperation( IT_GC , $2 , $3 ); }
    | TOK_NEW_HEAP TOK_IDENTIFIER parameter_list  { $$ = createInstantiationOperation( IT_HEAP , $2 , $3 ); }
    ;

expression_statement
    : expression TOK_SET expression  { $$ = createBinOperation( OP_SET , $1 , $3 ); }
    | expression TOK_INCREMENT  { $$ = createIncrementOperation( IPP_POST , $1 , $2 ); }
    | TOK_INCREMENT expression  { $$ = createIncrementOperation( IPP_PRE , $2 , $1 ); }
    | TOK_IDENTIFIER parameter_list  { $$ = createMethodCall( $1 , $2 ); }
    | expression TOK_ACCESSOR TOK_IDENTIFIER { $$ = createAccessNode( $1 , $3 ); }
    | expression TOK_ACCESSOR TOK_IDENTIFIER parameter_list  { $$ = createObjectCall( $1 , $3 , $4 ); }
    | expression TOK_ACCESSOR parameter_list  { $$ = createCallbackCall( $1 , $3 ); }
    ;

parameter_list
    : TOK_LPAREN parameter_list_contains TOK_RPAREN  { $$ = $2 ; }
    | TOK_LPAREN TOK_RPAREN  { $$ = NULL; }
    ;

parameter_list_contains
    : expression  { $$ = linkParameterList( $1 , NULL ); }
    | expression TOK_COMMA parameter_list_contains  { $$ = linkParameterList( $1 , $3 ); }
    ;

typename
    : TOK_IDENTIFIER  { $$ = createTypeNode( $1 , FALSE ); }
    | TOK_IDENTIFIER TOK_HAT  { $$ = createTypeNode( $1 , TRUE ); }
    | TOK_RESERVED_TYPE  { $$ = createReservedTypeNode( $1 , FALSE ); }
    | TOK_RESERVED_TYPE TOK_HAT  { $$ = createReservedTypeNode( $1 , TRUE ); }
    ;

%%
