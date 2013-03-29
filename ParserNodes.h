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
    ExpressionNode* createInstanceOfNode(ExpressionNode*, CheshireType instanceofType);
    ExpressionNode* createVariableAccess(char*);
    ExpressionNode* createStringNode(char*);
    ExpressionNode* createLargeIntegerNode(long);
    ExpressionNode* createIntegerNode(long);
    ExpressionNode* createDecimalNode(double);
    ExpressionNode* createCastOperation(ExpressionNode*, CheshireType newType);
    ExpressionNode* createInstantiationOperation(InstantiationType, CheshireType);
    ExpressionNode* createMethodCall(char* name, ExpressionList*);
    ExpressionNode* createIncrementOperation(ExpressionNode*, OperationType);
    ExpressionNode* createReservedLiteralNode(ReservedLiteral);
    ExpressionNode* createAccessNode(ExpressionNode*, char* classVariable);
    ExpressionNode* createLengthOperation(ExpressionNode*);
    ExpressionNode* dereferenceExpression(ExpressionNode*);
    ExpressionNode* createClosureNode(CheshireType, ParameterList*, BlockList*);

//defined in ExpressionList.c
    ExpressionList* linkExpressionList(ExpressionNode*, ExpressionList*);

//Defined in StatementNode.c
    StatementNode* createExpressionStatement(ExpressionNode*);
    StatementNode* createAssertionStatement(ExpressionNode*);
    StatementNode* createBlockStatement(BlockList*);
    StatementNode* createIfStatement(ExpressionNode* condition, StatementNode* ifBlock);
    StatementNode* createIfElseStatement(ExpressionNode* condition, StatementNode* ifBlock, StatementNode* elseBlock);
    StatementNode* createWhileStatement(ExpressionNode* condition, StatementNode* whileBlock);
    StatementNode* createVariableDefinition(CheshireType, char* name, ExpressionNode* value);
    StatementNode* createInferDefinition(char* name, ExpressionNode* value);
    StatementNode* createDeleteHeapStatement(ExpressionNode*);
    StatementNode* createReturnStatement(ExpressionNode*);

//defined in BlockList.c
    BlockList* linkBlockList(StatementNode*, BlockList*);

//defined in ParserTopNode.c
    ParserTopNode* createMethodDeclaration(CheshireType, char* name, ParameterList* params);
    ParserTopNode* createMethodDefinition(CheshireType, char* name, ParameterList* params, BlockList* body);
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
