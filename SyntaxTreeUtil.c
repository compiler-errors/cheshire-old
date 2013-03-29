/* File: SyntaxTreeUtil.c
 * Author: Michael Goulet
 * Implements: SyntaxTreeUtil.h
 */

#include <stdlib.h>
#include <stdio.h>
#include "TypeSystem.h"
#include "SyntaxTreeUtil.h"

void printExpression(ExpressionNode* node) {
    if (node == NULL) {
        printf("NULL VALUE");
        return;
    }

    switch (node->type) {
        case OP_NOP:
            PANIC("No such operation as No-OP");
            break;
        case OP_DEREFERENCE:
            printExpression(node->unaryChild);
            break;
        case OP_PLUSONE:
            printf("(");
            printExpression(node->unaryChild);
            printf("++)");
            break;
        case OP_MINUSONE:
            printf("(");
            printExpression(node->unaryChild);
            printf("--)");
            break;
        case OP_LENGTH:
            printf("(len ");
            printExpression(node->unaryChild);
            printf(")");
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
        case OP_INSTANCEOF:
            printf("(");
            printExpression(node->instanceof.expression);
            printf(" instanceof ");
            printCheshireType(node->instanceof.type);
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
            printCheshireType(node->cast.type);
            printf("> ");
            printExpression(node->cast.child);
            printf(")");
            break;
        case OP_NEW_GC: {
            CheshireType t = node->instantiate.type;
            printf("(new ");
            printCheshireType(t);
            printf(")");
            break;
        }
        case OP_NEW_HEAP: {
            CheshireType t = node->instantiate.type;
            printf("(new^ ");
            printCheshireType(t);
            printf(")");
            break;
        }
        case OP_METHOD_CALL:
            printf("(%s", node->methodcall.fn_name);
            printParameterList(node->methodcall.params);
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
        case OP_CLOSURE:
            printf("(");
            printCheshireType(getLambdaType(node->closure.type, node->closure.params));
            printf(" <CLOSURE>)");
            break;
    }
}

void printParameterList(ExpressionList* param) {
    printf("(");

    if (param == NULL) {
        printf("void");
    } else {
        for (; param != NULL; param = param->next) {
            printExpression(param->parameter);

            if (param->next != NULL)
                printf(", ");
        }
    }

    printf(")");
}