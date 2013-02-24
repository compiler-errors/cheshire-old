/* File: 
 * Author: Michael Goulet
 * Implements: 
 */

#include "Nodes.h"

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

void deleteStatementNode(StatementNode* node) {
    switch (node->type) {
        case S_NOP:
            //TODO: Panic, since I'm not supposed to be here.
            break;
        case S_EXPRESSION:
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
    }
    free(node);
}