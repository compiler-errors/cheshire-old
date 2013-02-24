/* 
 * File:   enums.h
 * Author: Michael Goulet
 * Implementation: N/A
 * 
 * Defines the common enums that are used widely within the parser/lexer and the program as a whole.
 * This header file should NOT define any functions, or any structures other than global enums.
 * Enums are to be styled as such. (ex.) TypeAttribute -> "TA_" + OPTION.
 */

#ifndef ENUMS_H
#define	ENUMS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum { RT_NUMBER, RT_BOOL, RT_INT, RT_DECIMAL, RT_VOID, RT_INFER } ReservedType;
typedef enum { RL_TRUE, RL_FALSE, RL_NULL } ReservedLiteral;
typedef enum { IT_GC, IT_HEAP } InstantiationType;
typedef enum { IPP_PRE, IPP_POST } IncrementPrePost;
typedef enum { FALSE = 0, TRUE = 1 } Boolean;

typedef enum {
    OP_NOP,
    OP_NOT,
    OP_COMPL,
    OP_UNARY_MINUS,
    OP_INCREMENT,
    OP_DECREMENT,
    OP_INCREMENT_PRE,
    OP_DECREMENT_PRE,
    OP_INCREMENT_POST,
    OP_DECREMENT_POST,
    OP_SIZEOF,
    OP_EQUALS,
    OP_NOT_EQUALS,
    OP_GRE_EQUALS,
    OP_LES_EQUALS,
    OP_GREATER,
    OP_LESS,
    OP_AND,
    OP_OR,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
    OP_MOD,
    OP_SET,
    OP_SELF,
    OP_INSTANCEOF,
    OP_VARIABLE,
    OP_NUMBER,
    OP_CAST,
    OP_NEW_GC,
    OP_NEW_HEAP,
    OP_ACCESS,
    OP_METHOD_CALL,
    OP_OBJECT_CALL,
    OP_CALLBACK_CALL,
    OP_RESERVED_LITERAL,
    OP_ARRAY_ACCESS,
    OP_STRING,
    OP_SIZEOF_TYPE
} OperationType;

typedef enum {
    S_NOP,
    S_EXPRESSION,
    S_BLOCK,
    S_IF,
    S_IF_ELSE,
    S_WHILE
} StatementType;

#ifdef	__cplusplus
}
#endif

#endif	/* ENUMS_H */

