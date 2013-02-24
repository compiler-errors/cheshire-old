/* 
 * File:   nodes.h
 * Author: Michael Goulet
 * Implementation: Nodes.c
 */

#ifndef NODES_H
#define	NODES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "Enums.h"

#define PANIC_OR_RETURN_NULL { printf("Couldn't allocate anything. NULL!\n"); exit(0); return NULL; }

struct tagExpressionNode;
struct tagParameterList;
struct tagInternalTypeNode;
struct tagStatementNode;
struct tagBlockList;

typedef struct tagExpressionNode {
    OperationType type;
    union { //FIXME: check if this works with union again.
        double numberValue;
        struct {
            struct tagExpressionNode* left;
            struct tagExpressionNode* right;
        } binary;
        struct tagExpressionNode* unaryChild;
        char* string;
        struct tagInternalTypeNode* typeNode;
        ReservedLiteral reserved;
        /* simple types above */
        struct {
            struct tagExpressionNode* expression;
            char* variable;
        } access;
        
        struct {
            struct tagExpressionNode* expression;
            struct tagInternalTypeNode* type;
        } instanceof;
        
        struct {
            struct tagExpressionNode* child;
            struct tagInternalTypeNode* type;
        } cast;
        
        struct {
            char* type;
            struct tagParameterList* params;
        } instantiate;
        
        struct {
            char* fn_name;
            struct tagParameterList* params;
        } methodcall;
        
        struct {
            struct tagExpressionNode* object;
            char* fn_name;
            struct tagParameterList* params;
        } objectcall;
        
        struct {
            struct tagExpressionNode* callback;
            struct tagParameterList* params;
        } callbackcall;
    };
} ExpressionNode;

typedef struct tagParameterList {
    struct tagExpressionNode* parameter;
    struct tagParameterList* next;
} ParameterList;

typedef struct tagInternalTypeNode {
    Boolean isReservedType;
    Boolean isUnsafeReference;
    union {
        ReservedType reservedType;
        char* baseType;
    };
} InternalTypeNode;

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
    };
} StatementNode;

typedef struct tagBlockList {
    struct tagStatementNode* statement;
    struct tagBlockList* next;
} BlockList;

ExpressionNode* createSelfNode(void);
ExpressionNode* createUnaryOperation(OperationType, ExpressionNode*);
ExpressionNode* createBinOperation(OperationType, ExpressionNode* left, ExpressionNode* right);
ExpressionNode* createInstanceOfNode(ExpressionNode* expression, InternalTypeNode* type);
ExpressionNode* createVariableAccess(char* variable);
ExpressionNode* createStringNode(char* str);
ExpressionNode* createNumberNode(double);
ExpressionNode* createCastOperation(ExpressionNode*, InternalTypeNode* type);
ExpressionNode* createInstantiationOperation(InstantiationType, char* type, ParameterList*);
ExpressionNode* createMethodCall(char* fn_name, ParameterList*);
ExpressionNode* createObjectCall(ExpressionNode* object, char* fn_name, ParameterList*);
ExpressionNode* createCallbackCall(ExpressionNode* callback, ParameterList*);
ExpressionNode* createIncrementOperation(IncrementPrePost, ExpressionNode*, OperationType);
ExpressionNode* createSizeOfExpression(ExpressionNode*);
ExpressionNode* createSizeOfTypeExpression(InternalTypeNode*);
ExpressionNode* createReservedLiteralNode(ReservedLiteral);
ExpressionNode* createAccessNode(ExpressionNode*, char* variable);
void deleteExpressionNode(ExpressionNode*);

ParameterList* linkParameterList(ExpressionNode* val, ParameterList* next);
void deleteParameterList(ParameterList*);

InternalTypeNode* createReservedTypeNode(ReservedType, Boolean isUnsafe);
InternalTypeNode* createTypeNode(char* base_type, Boolean isUnsafe);
void deleteTypeNode(InternalTypeNode*);

StatementNode* createExpressionStatement(ExpressionNode*);
StatementNode* createBlockStatement(BlockList*);
StatementNode* createIfStatement(ExpressionNode* condition, StatementNode* ifBlock);
StatementNode* createIfElseStatement(ExpressionNode* condition, StatementNode* ifBlock, StatementNode* elseBlock);
StatementNode* createWhileStatement(ExpressionNode* condition, StatementNode* block);
void deleteStatementNode(StatementNode*);

BlockList* linkBlockList(StatementNode*, BlockList*);
void deleteBlockList(BlockList*);

#ifdef	__cplusplus
}
#endif

#endif	/* NODES_H */

