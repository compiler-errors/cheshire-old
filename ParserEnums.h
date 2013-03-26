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

    typedef enum {
        PRT_NONE, PRT_METHOD_DECLARATION, PRT_METHOD_DEFINITION
    }
    ParserReturnType;
    typedef enum { RL_TRUE, RL_FALSE, RL_NULL } ReservedLiteral;
    typedef enum { IT_GC, IT_HEAP } InstantiationType;
    typedef enum { FALSE = 0, TRUE = 1 } Boolean;

    typedef enum {
        OP_NOP,             //placeholder type -- not used - hopefully - anywhere.
        OP_DEREFERENCE,
        OP_NOT,
        OP_COMPL,
        OP_UNARY_MINUS,
        OP_PLUSONE,
        OP_MINUSONE,
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
        OP_LENGTH
    } OperationType;

    typedef enum {
        S_NOP,
        S_VARIABLE_DEF,
        S_INFER_DEF,
        S_EXPRESSION,
        S_ASSERT,
        S_BLOCK,
        S_IF,
        S_IF_ELSE,
        S_WHILE,
        S_DELETE_HEAP,
        S_RETURN
    } StatementType;

#ifdef	__cplusplus
}
#endif

#endif	/* ENUMS_H */

