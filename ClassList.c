/* File: ClassList.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static ClassList* allocClassList(void) {
    ClassList* node = (ClassList*) malloc(sizeof(ClassList));
    
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    return node;
}

ClassList* linkClassVariable(CheshireType type, char* name, ExpressionNode* defaultValue, ClassList* next) {
    ClassList* node = allocClassList();
    
    if (node == NULL)
        return NULL;
    
    node->type = CLT_VARIABLE;
    node->variable.name = name;
    node->variable.type = type;
    node->variable.defaultValue = defaultValue;
    node->next = next;
    return node;
}

ClassList* linkClassMethod(CheshireType returnType, ParameterList* params, char* name, BlockList* block, ClassList* next) {
    ClassList* node = allocClassList();
    
    if (node == NULL)
        return NULL;
    
    node->type = CLT_METHOD;
    node->method.returnType = returnType;
    node->method.params = params;
    node->method.block = block;
    node->method.name = name;
    node->next = next;
    return node;
}

ClassList* linkClassConstructor(ParameterList* params, ExpressionList* inheritsParams, BlockList* block, ClassList* next) {
    ClassList* node = allocClassList();
    
    if (node == NULL)
        return NULL;
    
    node->type = CLT_CONSTRUCTOR;
    node->constructor.params = params;
    node->constructor.inheritsParams = inheritsParams;
    node->constructor.block = block;
    node->next = next;
    return node;
}

void deleteClassList(ClassList* node) {
    if (node == NULL)
        return;
    
    switch (node->type) {
        case CLT_VARIABLE:
            free(node->variable.name);
            deleteExpressionNode(node->variable.defaultValue);
            break;
        case CLT_METHOD:
            free(node->method.name);
            deleteBlockList(node->method.block);
            deleteParameterList(node->method.params);
            break;
        case CLT_CONSTRUCTOR:
            deleteParameterList(node->constructor.params);
            deleteBlockList(node->constructor.block);
            break;
    }
    
    deleteClassList(node->next);
    free(node);
}