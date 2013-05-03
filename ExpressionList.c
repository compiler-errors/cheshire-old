/* File: ExpressionList.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static ExpressionList* allocExpressionList(void) {
    ExpressionList* node = (ExpressionList*) malloc(sizeof(ExpressionList));
    
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->next = NULL;
    node->parameter = NULL;
    return node;
}

ExpressionList* linkExpressionList(ExpressionNode* val, ExpressionList* next) {
    ExpressionList* node = allocExpressionList();
    
    if (node == NULL)
        return NULL;
    
    node->parameter = val;
    node->next = next;
    return node;
}

void deleteExpressionList(ExpressionList* node) {
    if (node == NULL)
        return;
    
    deleteExpressionNode(node->parameter);
    deleteExpressionList(node->next);
    free(node);
}