#ifndef NODES_H
#define NODES_H

#include <stdlib.h>
#include <stdio.h>
#include "ParserEnums.h"
#include "Structures.h"

#ifdef __cplusplus
extern "C" {
#endif

//defined in ExpessionNode.c
    ExpressionNode* createUnaryOperation(OperationType, ExpressionNode*);
    ExpressionNode* createBinOperation(OperationType, ExpressionNode* left, ExpressionNode* right);
    ExpressionNode* createInstanceOfNode(ExpressionNode* expression, CheshireType type);
    ExpressionNode* createVariableAccess(char* variable);
    ExpressionNode* createStringNode(char* str);
    ExpressionNode* createNumberNode(double);
    ExpressionNode* createCastOperation(ExpressionNode*, CheshireType type);
    ExpressionNode* createInstantiationOperation(InstantiationType, CheshireType type);
    ExpressionNode* createMethodCall(char* functionName, ExpressionList*);
    ExpressionNode* createIncrementOperation(ExpressionNode*, OperationType);
    ExpressionNode* createReservedLiteralNode(ReservedLiteral);
    ExpressionNode* createAccessNode(ExpressionNode*, char* variable);
    ExpressionNode* createLengthOperation(ExpressionNode*);
    ExpressionNode* dereferenceExpression(ExpressionNode*);
    ExpressionNode* createClosureNode(CheshireType, ParameterList*, BlockList* body);

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
    StatementNode* createReturnStatement(ExpressionNode*);

//defined in BlockList.c
    BlockList* linkBlockList(StatementNode*, BlockList*);

//defined in ParserTopNode.c
    ParserTopNode* createMethodDeclaration(CheshireType, char* functionName, ParameterList* params);
    ParserTopNode* createMethodDefinition(CheshireType, char* functionName, ParameterList* params, BlockList* body);
    ParserTopNode* createGlobalVariableDeclaration(CheshireType type, char* name);
    ParserTopNode* createGlobalVariableDefinition(CheshireType type, char* name, ExpressionNode* value);
    ParserTopNode* createClassDefinition(char* name, ClassList*, CheshireType parent);

//defined in ParameterList.c
    ParameterList* linkParameterList(CheshireType type, char* name, ParameterList* next);
    
//defined in ClassList.c
    ClassList* linkClassList(CheshireType, char* name, ClassList* next);

#ifdef __cplusplus
}
#endif

#endif /* NODES_H */
