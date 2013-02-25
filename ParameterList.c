/* File: ParameterList.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static ParameterList* allocParameterList(void) {
    ParameterList* node = (ParameterList*) malloc(sizeof(ParameterList));
    
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->next = NULL;
    node->parameter = NULL;
    return node;
}

ParameterList* linkParameterList(ExpressionNode* val, ParameterList* next) {
    ParameterList* node = allocParameterList();
    
    if (node == NULL)
        return NULL;
    
    node->parameter = val;
    node->next = next;
    return node;
}

void deleteParameterList(ParameterList* node) {
    if (node == NULL)
        return;
    
    deleteExpressionNode(node->parameter);
    deleteParameterList(node->next);
    free(node);
}