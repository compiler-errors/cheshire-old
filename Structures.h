#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdlib.h>
#define PANIC(format, args...) { printf("Error: "); printf(format , ##args); printf("\n"); exit(0); }
#define PANIC_OR_RETURN_NULL { PANIC("Memory allocation error: ran out of memory!"); return NULL; }
#define ERROR_IF(_case, format, args...) { if (_case) { PANIC(format , ##args) } }

#include "ParserEnums.h"

#ifdef    __cplusplus
extern "C" {
#endif

// -------------------------------------------- //

    typedef int TypeKey;

    typedef struct tagCheshireType {
        TypeKey typeKey;
        int arrayNesting;
    } CheshireType;

// -------------------------------------------- //

    typedef struct tagLLVMValue {
        LLVMValueType type;
        union {
            int64_t value;
            double decimal;
            char* name;
            struct {
                char* name;
                int uid;
            } vardef;
        };
    } LLVMValue;

    typedef struct tagClassShape {
        struct tagClassShape* next;
        CheshireType type;
        const char* name;
    } ClassShape;

// -------------------------------------------- //

    struct tagParserTopNode;
    struct tagClassList;
    struct tagParameterList;
    struct tagExpressionNode;
    struct tagExpressionList;
    struct tagStatementNode;
    struct tagBlockList;

    typedef struct tagClassList {
        struct tagClassList* next;
        ClassListType type;

        union {
            struct {
                CheshireType type;
                struct tagExpressionNode* defaultValue;
                char* name;
            } variable;

            struct {
                CheshireType returnType;
                struct tagParameterList* params;
                struct tagBlockList* block;
                char* name;
            } method;

            struct {
                struct tagParameterList* params;
                struct tagExpressionList* inheritsParams;
                struct tagBlockList* block;
            } constructor;
        };
    } ClassList;

    typedef struct tagParserTopNode {
        ParserReturnType type;
        union {
            struct {
                char* functionName;
                CheshireType returnType;
                struct tagParameterList* params;
                struct tagBlockList* body;
            } method;

            struct {
                CheshireType type;
                char* name;
            } variable;

            struct {
                char* name;
                struct tagClassList* classlist;
                CheshireType parent;
            } classdef;
        };
    } ParserTopNode;

    typedef struct tagParameterList {
        CheshireType type;
        char* name;
        struct tagParameterList* next;
    } ParameterList;

    typedef struct tagExpressionNode {
        OperationType type;
        CheshireType determinedType;
        union {
            int64_t integer;
            double decimal;
            char character;
            char* string;
            ReservedLiteral reserved;
            struct tagExpressionNode* unaryChild;

            struct {
                struct tagExpressionNode* left;
                struct tagExpressionNode* right;
            } binary;


            /* simple types above */
            struct {
                struct tagExpressionNode* expression;
                char* variable;
            } access;

            struct {
                struct tagExpressionNode* expression;
                CheshireType type;
            } instanceof;

            struct {
                struct tagExpressionNode* child;
                CheshireType type;
            } cast;

            struct {
                CheshireType type;
                struct tagExpressionList* params;
            } instantiate;

            struct {
                struct tagExpressionNode* callback;
                struct tagExpressionList* params;
            } methodcall;

            struct {
                CheshireType type;
                struct tagParameterList* params;
                struct tagBlockList* body;
            } closure;

            struct {
                struct tagExpressionNode* object;
                char* method;
                struct tagExpressionList* params;
            } objectcall;
        };
    } ExpressionNode;

    typedef struct tagExpressionList {
        struct tagExpressionNode* parameter;
        struct tagExpressionList* next;
    } ExpressionList;

    typedef struct tagStatementNode {
        StatementType type;
        union {
            struct tagExpressionNode* expression;
            struct tagBlockList* block;
            struct {
                struct tagExpressionNode* condition;
                struct tagStatementNode* block;
                struct tagStatementNode* elseBlock;
            } conditional;
            struct {
                CheshireType type;
                char* variable;
                struct tagExpressionNode* value;
            } varDefinition;
        };
    } StatementNode;

    typedef struct tagBlockList {
        struct tagStatementNode* statement;
        struct tagBlockList* next;
    } BlockList;


    void deleteClassShape(ClassShape*);
    void deleteExpressionNode(ExpressionNode*);
    void deleteExpressionList(ExpressionList*);
    void deleteStatementNode(StatementNode*);
    void deleteBlockList(BlockList*);
    void deleteParserTopNode(ParserTopNode*);
    void deleteParameterList(ParameterList*);
    void deleteClassList(ClassList*);

#ifdef    __cplusplus
}
#endif

#endif /* STRUCTURES_H*/