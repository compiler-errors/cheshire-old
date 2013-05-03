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
        PRT_NONE,
        PRT_METHOD_DECLARATION,
        PRT_METHOD_DEFINITION,
        PRT_VARIABLE_DECLARATION,
        PRT_VARIABLE_DEFINITION,
        PRT_CLASS_DEFINITION
    }
    ParserReturnType;
    typedef enum { RL_TRUE, RL_FALSE, RL_NULL } ReservedLiteral;
    typedef enum { FALSE = 0, TRUE = 1 } Boolean;
    typedef enum { CLT_METHOD, CLT_VARIABLE, CLT_CONSTRUCTOR } ClassListType;

    typedef enum {
        OP_NOP,             //placeholder type: not used - hopefully - anywhere.
        OP_INTEGER,
        OP_DECIMAL,
        OP_CHAR,
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
        OP_INSTANCEOF,
        OP_VARIABLE,
        OP_CAST,
        OP_ACCESS,
        OP_METHOD_CALL,
        OP_RESERVED_LITERAL,
        OP_ARRAY_ACCESS,
        OP_STRING,
        OP_CLOSURE,
        OP_INSTANTIATION,
        OP_OBJECT_CALL
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
        S_RETURN
    } StatementType;

    typedef enum {
        LVT_GLOBAL_VARIABLE,
        LVT_GLOBAL_METHOD,
        LVT_CLOSURE_METHOD,
        LVT_LOCAL_VALUE,
        LVT_LOCAL_VARIABLE,
        LVT_PARAMETER_VARIABLE,
        LVT_INT_LITERAL,
        LVT_DOUBLE_LITERAL,
        LVT_VOID,
        LVT_JUMPPOINT,
        LVT_METHOD_EXPORT,
        LVT_CLASS_METHOD,
        LVT_NULL
    } LLVMValueType;


#ifdef	__cplusplus
}
#endif

#endif	/* ENUMS_H */

