/* File: SyntaxTreeUtil.c
 * Author: Michael Goulet
 * Implements: SyntaxTreeUtil.h 
 */

#include <stdlib.h>
#include <stdio.h>
#include "SyntaxTreeUtil.h"

void printExpression(ExpressionNode* node) {
    if (node == NULL) {
        printf("NULL?!");
        return;
    }
    
    switch (node->type) {
        case OP_NOP:
        case OP_INCREMENT:
        case OP_DECREMENT:
            printf("This shouldn't be here!");
            exit(0);
            break;
        case OP_INCREMENT_PRE:
            printf("(++");
            printExpression(node->unaryChild);
            printf(")");
            break;
        case OP_INCREMENT_POST:
            printf("(");
            printExpression(node->unaryChild);
            printf("++)");
            break;
        case OP_DECREMENT_PRE:
            printf("(--");
            printExpression(node->unaryChild);
            printf(")");
            break;
        case OP_DECREMENT_POST:
            printf("(");
            printExpression(node->unaryChild);
            printf("--)");
            break;
        case OP_NOT:
            printf("(not ");
            printExpression(node->unaryChild);
            printf(")");
            break;
        case OP_UNARY_MINUS:
            printf("(-");
            printExpression(node->unaryChild);
            printf(")");
            break;
        case OP_COMPL:
            printf("(compl ");
            printExpression(node->unaryChild);
            printf(")");
            break;
        case OP_SIZEOF:
            printf("(sizeof ");
            printExpression(node->unaryChild);
            printf(")");
            break;
        case OP_SIZEOF_TYPE:
            printf("(sizeof ");
            printInternalTypeNode(node->typeNode);
            printf(")");
            break;
        case OP_EQUALS:
            printf("(");
            printExpression(node->binary.left);
            printf(" == ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_NOT_EQUALS:
            printf("(");
            printExpression(node->binary.left);
            printf(" != ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_GRE_EQUALS:
            printf("(");
            printExpression(node->binary.left);
            printf(" >= ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_LES_EQUALS:
            printf("(");
            printExpression(node->binary.left);
            printf(" <= ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_GREATER:
            printf("(");
            printExpression(node->binary.left);
            printf(" > ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_LESS:
            printf("(");
            printExpression(node->binary.left);
            printf(" < ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_AND:
            printf("(");
            printExpression(node->binary.left);
            printf(" and ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_OR:
            printf("(");
            printExpression(node->binary.left);
            printf(" or ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_PLUS:
            printf("(");
            printExpression(node->binary.left);
            printf(" + ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_MINUS:
            printf("(");
            printExpression(node->binary.left);
            printf(" - ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_MULT:
            printf("(");
            printExpression(node->binary.left);
            printf(" * ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_DIV:
            printf("(");
            printExpression(node->binary.left);
            printf(" / ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_MOD:
            printf("(");
            printExpression(node->binary.left);
            printf(" %% ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_SET:
            printf("(");
            printExpression(node->binary.left);
            printf(" = ");
            printExpression(node->binary.right);
            printf(")");
            break;
        case OP_ACCESS:
            printf("(");
            printExpression(node->access.expression);
            printf(":%s)", node->access.variable);
            break;
        case OP_SELF:
            printf("(self)");
            break;
        case OP_INSTANCEOF:
            printf("(");
            printExpression(node->instanceof.expression);
            printf(" instanceof ");
            printInternalTypeNode(node->instanceof.type);
            printf(")");
            break;
        case OP_VARIABLE:
            printf("(%s)", node->string);
            break;
        case OP_STRING:
            printf("(\"%s\")", node->string);
            break;
        case OP_NUMBER:
            printf("(%lf)", node->numberValue);
            break;
        case OP_CAST:
            printf("(cast <");
            printInternalTypeNode(node->cast.type);
            printf("> ");
            printExpression(node->cast.child);
            printf(")");
            break;
        case OP_NEW_GC:
            printf("(new %s", node->instantiate.type);
            printParameterList(node->instantiate.params);
            printf(")");
            break;
        case OP_NEW_HEAP:
            printf("(new^ %s", node->instantiate.type);
            printParameterList(node->instantiate.params);
            printf(")");
            break;
        case OP_METHOD_CALL:
            printf("(%s", node->methodcall.fn_name);
            printParameterList(node->methodcall.params);
            printf(")");
            break;
        case OP_OBJECT_CALL:
            printf("(");
            printExpression(node->objectcall.object);
            printf(":%s", node->objectcall.fn_name);
            printParameterList(node->objectcall.params);
            printf(")");
            break;
        case OP_CALLBACK_CALL:
            printf("(");
            printExpression(node->callbackcall.callback);
            printf(":");
            printParameterList(node->callbackcall.params);
            printf(")");
            break;
        case OP_RESERVED_LITERAL:
            printf("(");
            switch (node->reserved) {
                case RL_TRUE:
                    printf("True!");
                    break;
                case RL_FALSE:
                    printf("False!");
                    break;
                case RL_NULL:
                    printf("Null!");
                    break;
            }
            printf(")");
            break;
        case OP_ARRAY_ACCESS:
            printf("(");
            printExpression(node->binary.left);
            printf("[");
            printExpression(node->binary.right);
            printf("]");
            printf(")");
            break;
    }
}

void printParameterList(ParameterList* param) {
    printf("(");
    if (param == NULL) {
        printf("void");
    } else {
        for ( ; param != NULL; param = param->next ) {
            printExpression(param->parameter);

            if (param->next != NULL)
                printf(", ");
        }
    }
    printf(")");
}

void printInternalTypeNode(InternalTypeNode* node) {
    if (node->isReservedType) {
        switch (node->reservedType) {
            case RT_NUMBER:
                printf("Number");
                break;
            case RT_BOOL:
                printf("Bool");
                break;
            case RT_INT:
                printf("Int");
                break;
            case RT_DECIMAL:
                printf("Decimal");
                break;
            case RT_VOID:
                printf("void");
                break;
            case RT_INFER:
                printf("infer");
                break;
        }
    } else {
        printf("%s", node->baseType); //FIXME: ("%s", string) might be redundant?
    }
    
    if (node->isUnsafeReference)
        printf("^");
}