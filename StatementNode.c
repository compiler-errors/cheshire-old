/* File: StatementNode.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static StatementNode* allocStatementNode(void) {
    StatementNode* node = (StatementNode*) malloc(sizeof(StatementNode));
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->type = S_NOP;
    return node;
}

StatementNode* createExpressionStatement(ExpressionNode* expression) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_EXPRESSION;
    node->expression = expression;
    return node;
}

StatementNode* createAssertionStatement(ExpressionNode* expression) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_ASSERT;
    node->expression = expression;
    return node;
}

StatementNode* createBlockStatement(BlockList* block) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_BLOCK;
    node->block = block;
    return node;
}

StatementNode* createIfStatement(ExpressionNode* condition, StatementNode* ifBlock) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_IF;
    node->conditional.condition = condition;
    node->conditional.block = ifBlock;
    node->conditional.elseBlock = NULL;
    return node;
}

StatementNode* createIfElseStatement(ExpressionNode* condition, StatementNode* ifBlock, StatementNode* elseBlock) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_IF_ELSE;
    node->conditional.condition = condition;
    node->conditional.block = ifBlock;
    node->conditional.elseBlock = elseBlock;
    return node;
}

StatementNode* createWhileStatement(ExpressionNode* condition, StatementNode* block) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_WHILE;
    node->conditional.condition = condition;
    node->conditional.block = block;
    node->conditional.elseBlock = NULL;
    return node;
}

StatementNode* createVariableDefinition(CheshireType type, char* variable, ExpressionNode* value) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_VARIABLE_DEF;
    node->varDefinition.type.typeKey = (TypeKey) -1;
    node->varDefinition.variable = variable;
    node->varDefinition.value = value;
    return node;
}

StatementNode* createInferDefinition(char* variable, ExpressionNode* value) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_INFER_DEF;
    node->varDefinition.type.typeKey = (TypeKey) -1;
    node->varDefinition.variable = variable;
    node->varDefinition.value = value;
    return node;
}

StatementNode* createDeleteHeapStatement(ExpressionNode* expression) {
    StatementNode* node = allocStatementNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = S_DELETE_HEAP;
    node->expression = expression;
    return node;
}

void deleteStatementNode(StatementNode* node) {
    switch (node->type) {
        case S_NOP:
            PANIC("No such operation as No-OP");
            break;
        case S_EXPRESSION:
        case S_ASSERT:
        case S_DELETE_HEAP:
            deleteExpressionNode(node->expression);
            break;
        case S_BLOCK:
            deleteBlockList(node->block);
            break;
        case S_IF:
            deleteExpressionNode(node->conditional.condition);
            deleteStatementNode(node->conditional.block);
            deleteStatementNode(node->conditional.elseBlock);
            break;
        case S_WHILE:
        case S_IF_ELSE:
            deleteExpressionNode(node->conditional.condition);
            deleteStatementNode(node->conditional.block);
            break;
        case S_VARIABLE_DEF:
        case S_INFER_DEF:
            free(node->varDefinition.variable);
            deleteExpressionNode(node->varDefinition.value);
            break;
    }
    free(node);
}