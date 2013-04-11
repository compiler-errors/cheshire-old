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

ParserTopNode* createGlobalVariableDeclaration(CheshireType type, char* name) {
    ParserTopNode* node = allocParserTopNode();

    if (node == NULL)
        return NULL;

    node->type = PRT_VARIABLE_DECLARATION;
    node->variable.type = type;
    node->variable.name = name;
    return node;
}

ParserTopNode* createGlobalVariableDefinition(CheshireType type, char* name, ExpressionNode* value) {
    ParserTopNode* node = allocParserTopNode();

    if (node == NULL)
        return NULL;

    node->type = PRT_VARIABLE_DEFINITION;
    node->variable.type = type;
    node->variable.name = name;
    node->variable.value = value;
    return node;
}

ParserTopNode* createClassDefinition(char* name, ClassList* classlist, CheshireType parent) {
    ParserTopNode* node = allocParserTopNode();

    if (node == NULL)
        return NULL;
    
    node->type = PRT_CLASS_DEFINITION;
    node->classdef.name = name;
    node->classdef.classlist = classlist;
    node->classdef.parent = parent;
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
            free(node->method.functionName); //don't forget to free all of the ParserTopNode(s) -AFTER- code has been emitted from all of the things that depend on it.
            deleteBlockList(node->method.body);
            deleteParameterList(node->method.params);
            break;
        case PRT_VARIABLE_DECLARATION:
            free(node->variable.name);
            break;
        case PRT_VARIABLE_DEFINITION:
            free(node->variable.name);
            deleteExpressionNode(node->variable.value);
            break;
        case PRT_CLASS_DEFINITION:
            //free(node->classdef.name);
            deleteClassList(node->classdef.classlist);
            break;
    }

    free(node);
}