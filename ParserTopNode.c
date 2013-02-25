/* File: ParserTopNode.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static ParserTopNode* allocParserTopNode(void) {
    ParserTopNode* node = (ParserTopNode*) malloc(sizeof(ParserTopNode));
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->type = PRT_NONE;
    return node;
}

ParserTopNode* createMethodDeclaration(InternalTypeNode* type, char* functionName) {
    ParserTopNode* node = allocParserTopNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = PRT_METHOD_DECLARATION;
    node->method.type = type;
    node->method.functionName = functionName;
    return node;
}

ParserTopNode* createMethodDefinition(InternalTypeNode* type, char* functionName, BlockList* body) {
    ParserTopNode* node = allocParserTopNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = PRT_METHOD_DEFINITION;
    node->method.type = type;
    node->method.functionName = functionName;
    node->method.body = body;
    return node;
}

void deleteParserTopNode(ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            //todo: panic! I shouldn't be here.
            break;
        case PRT_METHOD_DECLARATION:
            deleteTypeNode(node->method.type);
            free(node->method.functionName);
            break;
        case PRT_METHOD_DEFINITION:
            deleteTypeNode(node->method.type);
            free(node->method.functionName);
            deleteBlockList(node->method.body);
    }
    free(node);
}