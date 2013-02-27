/* File: ExpressionNode.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static ExpressionNode* allocExpressionNode(void) {
    ExpressionNode* node = (ExpressionNode*) malloc(sizeof(ExpressionNode));
    
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->type = OP_NOP;
    return node;
}

ExpressionNode* createSelfNode(void) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_SELF;
    return node;
}

ExpressionNode* createUnaryOperation(OperationType optype, ExpressionNode* child) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    if (optype == OP_MINUS) {
        optype = OP_UNARY_MINUS;
    } else if (optype == OP_PLUS) {
        //todo: PANIC! I shouldn't be getting unary +.
    }
    
    node->type = optype;
    node->unaryChild = child;
    return node;
}

ExpressionNode* createBinOperation(OperationType optype, ExpressionNode* left, ExpressionNode* right) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = optype;
    node->binary.left = left;
    node->binary.right = right;
    return node;
}

ExpressionNode* createInstanceOfNode(ExpressionNode* expression, CheshireType type) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_INSTANCEOF;
    node->instanceof.expression = expression;
    node->instanceof.type = type;
    return node;
}

ExpressionNode* createVariableAccess(char* variable) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_VARIABLE;
    node->string = variable;
    return node;
}

ExpressionNode* createStringNode(char* str) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_STRING;
    node->string = str;
    return node;
}

ExpressionNode* createNumberNode(double value) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_NUMBER;
    node->numberValue = value;
    return node;
}

ExpressionNode* createCastOperation(ExpressionNode* expression, CheshireType type) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_CAST;
    node->cast.child = expression;
    node->cast.type = type;
    return node;
}

ExpressionNode* createInstantiationOperation(InstantiationType itype, char* type, ExpressionList* params) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    if (itype == IT_GC)
        node->type = OP_NEW_GC;
    else
        node->type = OP_NEW_HEAP;
    
    node->instantiate.type = type;
    node->instantiate.params = params;
    return node;
}

ExpressionNode* createAccessNode(ExpressionNode* object, char* variable) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_ACCESS;
    node->access.expression = object;
    node->access.variable = variable;
    return node;
}

ExpressionNode* createMethodCall(char* fn_name, ExpressionList* params) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_METHOD_CALL;
    node->methodcall.fn_name = fn_name;
    node->methodcall.params = params;
    return node;
}

ExpressionNode* createObjectCall(ExpressionNode* object, char* fn_name, ExpressionList* params) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_OBJECT_CALL;
    node->objectcall.object = object;
    node->objectcall.fn_name = fn_name;
    node->objectcall.params = params;
    return node;
}

ExpressionNode* createCallbackCall(ExpressionNode* callback, ExpressionList* params) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_CALLBACK_CALL;
    node->callbackcall.callback = callback;
    node->callbackcall.params = params;
    return node;
}

ExpressionNode* createIncrementOperation(IncrementPrePost ipp, ExpressionNode* expression, OperationType optype) {
    ExpressionNode* node = allocExpressionNode();
    
    if (ipp == IPP_PRE) {
        if (optype == OP_INCREMENT)
            optype = OP_INCREMENT_PRE;
        else
            optype = OP_DECREMENT_PRE;
    } else {
        if (optype == OP_INCREMENT)
            optype = OP_INCREMENT_POST;
        else
            optype = OP_DECREMENT_POST;
    }
    
    if (node == NULL)
        return NULL;
    
    node->type = optype;
    node->unaryChild = expression;
    return node;
}

ExpressionNode* createSizeOfExpression(ExpressionNode* child) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_SIZEOF;
    node->unaryChild = child;
    return node;
}

ExpressionNode* createSizeOfTypeExpression(CheshireType typeNode) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_SIZEOF_TYPE;
    node->typeNode = typeNode;
    return node;
}

ExpressionNode* createReservedLiteralNode(ReservedLiteral rl) {
    ExpressionNode* node = allocExpressionNode();
    
    if (node == NULL)
        return NULL;
    
    node->type = OP_RESERVED_LITERAL;
    node->reserved = rl;
    return node;
}

void deleteExpressionNode(ExpressionNode* node) {
    if (node == NULL)
        return;
    
    switch (node->type) {
        case OP_NOP:
        case OP_INCREMENT:
        case OP_DECREMENT:
            //TODO: Panic, since I'm not supposed to be here.
            break;
        case OP_NOT:
        case OP_COMPL:
        case OP_INCREMENT_PRE:
        case OP_DECREMENT_PRE:
        case OP_INCREMENT_POST:
        case OP_DECREMENT_POST:
        case OP_SIZEOF:
        case OP_UNARY_MINUS:
            deleteExpressionNode(node->unaryChild); // Unary Operations
            break;
        case OP_EQUALS:
        case OP_NOT_EQUALS:
        case OP_GRE_EQUALS:
        case OP_LES_EQUALS:
        case OP_GREATER:
        case OP_LESS:
        case OP_AND:
        case OP_OR:
        case OP_PLUS:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_MOD:
        case OP_SET:
        case OP_ARRAY_ACCESS:
            deleteExpressionNode(node->binary.left);
            deleteExpressionNode(node->binary.right);
            break;
        case OP_ACCESS:
            deleteExpressionNode(node->access.expression);
            free(node->access.variable);
            break;
        case OP_SIZEOF_TYPE:
            break;
        case OP_INSTANCEOF:
            deleteExpressionNode(node->instanceof.expression);
            break;
        case OP_VARIABLE:
        case OP_STRING:
            free(node->string);
            break;
        case OP_CAST:
            deleteExpressionNode(node->cast.child);
            break;
        case OP_NEW_GC:
        case OP_NEW_HEAP:
            deleteExpressionList(node->instantiate.params);
            free(node->instantiate.type);
            break;
        case OP_METHOD_CALL:
            free(node->methodcall.fn_name);
            deleteExpressionList(node->methodcall.params);
            break;
        case OP_OBJECT_CALL:
            free(node->objectcall.fn_name);
            deleteExpressionNode(node->objectcall.object);
            deleteExpressionList(node->objectcall.params);
            break;
        case OP_CALLBACK_CALL:
            deleteExpressionNode(node->callbackcall.callback);
            deleteExpressionList(node->callbackcall.params);
            break;
        case OP_NUMBER:
        case OP_SELF:
        case OP_RESERVED_LITERAL:
            //DEFAULT, NO OPERATION
            break;
    }
    
    free(node);
}