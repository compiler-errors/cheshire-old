/* File: 
 * Author: Michael Goulet
 * Implements: 
 */

#include "Nodes.h"

static InternalTypeNode* allocTypeNode(void) {
    InternalTypeNode* node = (InternalTypeNode*) malloc(sizeof(InternalTypeNode));
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->isReservedType = TRUE;
    node->reservedType = RT_VOID;
    return node;
}

InternalTypeNode* createReservedTypeNode(ReservedType rt, Boolean isUnsafe) {
    InternalTypeNode* node = allocTypeNode();
    
    if (node == NULL)
        return NULL;
    
    node->isReservedType = TRUE;
    node->reservedType = rt;
    node->isUnsafeReference = isUnsafe;
    return node;
}

InternalTypeNode* createTypeNode(char* base, Boolean isUnsafe) {
    InternalTypeNode* node = allocTypeNode();
    
    if (node == NULL)
        return NULL;
    
    node->isReservedType = FALSE;
    node->baseType = base;
    node->isUnsafeReference = isUnsafe;
    return node;
}

void deleteTypeNode(InternalTypeNode* node) {
    if (!(node->isReservedType))
        free(node->baseType);
    
    free(node);
}