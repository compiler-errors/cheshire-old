#ifndef NODES_H
#define NODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "ParserEnums.h"
#include "Structures.h"

//defined in ExpessionNode.c
ExpressionNode* createSelfNode(void);
ExpressionNode* createUnaryOperation(OperationType, ExpressionNode*);
ExpressionNode* createBinOperation(OperationType, ExpressionNode* left, ExpressionNode* right);
ExpressionNode* createInstanceOfNode(ExpressionNode* expression, CheshireType type);
ExpressionNode* createVariableAccess(char* variable);
ExpressionNode* createStringNode(char* str);
ExpressionNode* createNumberNode(double);
ExpressionNode* createCastOperation(ExpressionNode*, CheshireType type);
ExpressionNode* createInstantiationOperation(InstantiationType, CheshireType type, ExpressionList*);
ExpressionNode* createMethodCall(char* functionName, ExpressionList*);
ExpressionNode* createObjectCall(ExpressionNode* object, char* functionName, ExpressionList*);
ExpressionNode* createCallbackCall(ExpressionNode* callback, ExpressionList*);
ExpressionNode* createIncrementOperation(ExpressionNode*, OperationType);
ExpressionNode* createReservedLiteralNode(ReservedLiteral);
ExpressionNode* createAccessNode(ExpressionNode*, char* variable);
ExpressionNode* createLengthOperation(ExpressionNode*);
ExpressionNode* dereferenceExpression(ExpressionNode*);

//defined in ExpressionList.c
ExpressionList* linkExpressionList(ExpressionNode* val, ExpressionList* next);

//Defined in StatementNode.c
StatementNode* createExpressionStatement(ExpressionNode*);
StatementNode* createAssertionStatement(ExpressionNode*);
StatementNode* createBlockStatement(BlockList*);
StatementNode* createIfStatement(ExpressionNode* condition, StatementNode* ifBlock);
StatementNode* createIfElseStatement(ExpressionNode* condition, StatementNode* ifBlock, StatementNode* elseBlock);
StatementNode* createWhileStatement(ExpressionNode* condition, StatementNode* block);
StatementNode* createVariableDefinition(CheshireType, char* variable, ExpressionNode* value);
StatementNode* createInferDefinition(char* variable, ExpressionNode* value);
StatementNode* createDeleteHeapStatement(ExpressionNode*);

//defined in BlockList.c
BlockList* linkBlockList(StatementNode*, BlockList*);

//defined in ParserTopNode.c
ParserTopNode* createMethodDeclaration(CheshireType, char* functionName, ParameterList* params);
ParserTopNode* createMethodDefinition(CheshireType, char* functionName, ParameterList* params, BlockList* body);

//defined in ParameterList.c
ParameterList* linkParameterList(CheshireType type, char* name, ParameterList* next);


#ifdef __cplusplus
}
#endif

#endif /* NODES_H */