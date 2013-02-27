/* File: ParameterList.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static ParameterList* allocParameterList(void) {
    ParameterList* node = (ParameterList*) malloc(sizeof(ParameterList));
    
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->type.typeKey = -1;
    node->name = NULL;
    node->next = NULL;
    return node;
}

ParameterList* linkParameterList(CheshireType type, char* name, ParameterList* next) {
    ParameterList* node = allocParameterList();
    
    if (node == NULL)
        return NULL;
    
    node->type = type;
    node->name = name;
    node->next = next;
    return node;
}

void deleteParameterList(ParameterList* node) {
    if (node == NULL)
        return;
    
    free(node->name);
    deleteParameterList(node->next);
    free(node);
}