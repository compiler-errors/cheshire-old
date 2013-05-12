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
        printf("- INVALID EXPRESSION -");
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
            printType(node->instanceof.type);
            printf(")");
            break;
        case OP_VARIABLE:
            printf("(%s)", node->string);
            break;
        case OP_STRING:
            printf("(\"%s\")", node->string);
            break;
        case OP_INTEGER:
            printf("(%lld)", node->integer);
            break;
        case OP_LONG_INTEGER:
            printf("(%lldL)", node->integer);
            break;
        case OP_DECIMAL:
            printf("(%lf)", node->decimal);
            break;
        case OP_CHAR:
            printf("('%c')", node->character);
            break;
        case OP_CAST:
            printf("((");
            printType(node->cast.type);
            printf(") ");
            printExpression(node->cast.child);
            printf(")");
            break;
        case OP_METHOD_CALL:
            printf("(");
            printExpression(node->methodcall.callback);
            printParameters(node->methodcall.params);
            printf(")");
            break;
        case OP_RESERVED_LITERAL:
            printf("(");

            switch (node->reserved) {
                case RL_TRUE:
                    printf("True");
                    break;
                case RL_FALSE:
                    printf("False");
                    break;
                case RL_NULL:
                    printf("Null");
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
            printType(getLambdaType(node->closure.type, node->closure.params));
            printf(" <CLOSURE>)");
            break;
        case OP_LAMBDA:
            printf("(<LAMBDA>)"); //todo: really do this...
            break;
        case OP_INSTANTIATION:
            printf("(new ");
            printType(node->instantiate.type);
            printParameters(node->instantiate.params);
            printf(")");
            break;
        case OP_OBJECT_CALL:
            printf("(");
            printExpression(node->objectcall.object);
            printf("::%s", node->objectcall.method);
            printParameters(node->objectcall.params);
            printf(")");
            break;
        case OP_LENGTH:
            printf("(len ");
            printExpression(node->unaryChild);
            printf(")");
            break;
        case OP_CHOOSE:
            printf("{");
            printExpression(node->choose.iftrue);
            printf(" if ");
            printExpression(node->choose.condition);
            printf(" else ");
            printExpression(node->choose.iffalse);
            printf("}");
            break;
    }
}

void printParameters(ExpressionList* param) {
    printf("(");

    if (param != NULL) {
        for (; param != NULL; param = param->next) {
            printExpression(param->parameter);

            if (param->next != NULL)
                printf(", ");
        }
    }

    printf(")");
}