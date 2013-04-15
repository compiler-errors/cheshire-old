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

ExpressionNode* createUnaryOperation(OperationType optype, ExpressionNode* child) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    if (optype == OP_MINUS) {
        optype = OP_UNARY_MINUS;
    } else if (optype == OP_PLUS) {
        PANIC("No such operation as \"unary +\"");
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

ExpressionNode* createIntegerNode(long i) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_INTEGER;
    node->integer = i;
    return node;
}

ExpressionNode* createDecimalNode(double d) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_DECIMAL;
    node->decimal = d;
    return node;
}

ExpressionNode* createCharNode(char character) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_CHAR;
    node->character = character;
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

ExpressionNode* createAccessNode(ExpressionNode* object, char* variable) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_ACCESS;
    node->access.expression = object;
    node->access.variable = variable;
    return node;
}

ExpressionNode* createMethodCall(ExpressionNode* callback, ExpressionList* params) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_METHOD_CALL;
    node->methodcall.callback = callback;
    node->methodcall.params = params;
    return node;
}

ExpressionNode* createIncrementOperation(ExpressionNode* expression, OperationType optype) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = optype;
    node->unaryChild = expression;
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

ExpressionNode* dereferenceExpression(ExpressionNode* expression) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_DEREFERENCE;
    node->unaryChild = expression;
    return node;
}

ExpressionNode* createClosureNode(CheshireType type, ParameterList* params, BlockList* body) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_CLOSURE;
    node->closure.type = type;
    node->closure.params = params;
    node->closure.body = body;
    return node;
}

ExpressionNode* createInstantiationOperation(CheshireType type, ExpressionList* params) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_INSTANTIATION;
    node->instantiate.type = type;
    node->instantiate.params = params;
    return node;
}

ExpressionNode* createObjectCall(ExpressionNode* object, char* method, ExpressionList* params) {
    ExpressionNode* node = allocExpressionNode();

    if (node == NULL)
        return NULL;

    node->type = OP_OBJECT_CALL;
    node->objectcall.object = object;
    node->objectcall.method = method;
    node->objectcall.params = params;
    return node;
}

void deleteExpressionNode(ExpressionNode* node) {
    if (node == NULL)
        return;

    switch (node->type) {
        case OP_NOP:
            PANIC("No such operation as No-OP or Increment/Decrement without \"Post-\" or \"Pre-\"");
            break;
        case OP_NOT:
        case OP_COMPL:
        case OP_UNARY_MINUS:
        case OP_PLUSONE:
        case OP_MINUSONE:
        case OP_DEREFERENCE:
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
        case OP_METHOD_CALL:
            deleteExpressionNode(node->methodcall.callback);
            deleteExpressionList(node->methodcall.params);
            break;
        case OP_CLOSURE:
            deleteParameterList(node->closure.params);
            deleteBlockList(node->closure.body);
            break;
        case OP_INSTANTIATION:
            deleteExpressionList(node->instantiate.params);
            break;
        case OP_OBJECT_CALL:
            deleteExpressionNode(node->objectcall.object);
            free(node->objectcall.method);
            deleteExpressionList(node->objectcall.params);
            break;
        case OP_INTEGER:
        case OP_DECIMAL:
        case OP_CHAR:
        case OP_RESERVED_LITERAL:
            //DEFAULT, NO OPERATION
            break;
    }

    free(node);
}