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

ParserTopNode* createMethodDeclaration(CheshireType type, char* functionName, ParameterList* params) {
    ParserTopNode* node = allocParserTopNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = PRT_METHOD_DECLARATION;
    node->method.type = type;
    node->method.functionName = functionName;
    node->method.params = params;
    return node;
}

ParserTopNode* createMethodDefinition(CheshireType type, char* functionName, ParameterList* params, BlockList* body) {
    ParserTopNode* node = allocParserTopNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = PRT_METHOD_DEFINITION;
    node->method.type = type;
    node->method.functionName = functionName;
    node->method.params = params;
    node->method.body = body;
    return node;
}

void deleteParserTopNode(ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            PANIC("No such statement as No-OP.");
            break;
        case PRT_METHOD_DECLARATION:
            free(node->method.functionName);
            deleteParameterList(node->method.params);
            break;
        case PRT_METHOD_DEFINITION:
            free(node->method.functionName);
            deleteBlockList(node->method.body);
            deleteParameterList(node->method.params);
            break;
    }
    free(node);
}