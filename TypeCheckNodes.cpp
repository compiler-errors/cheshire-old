/* 
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

#define __GXX_EXPERIMENTAL_CXX0X__ 1

#include "Structures.h"
#include "TypeSystem.h"
#include <cmath>
#include <climits>

using std::floor;

#define WIDEN_NODE(type, node) node = createCastOperation(node, type)

/* This "method" is defined to 
 */
#define STORE_EXPRESSION_INTO_LVAL(ltype, rtype, node, reason) \
            if (!areEqualTypes(ltype, rtype)) { \
                if (isNumericalType(ltype) && isNumericalType(rtype)) { \
                    CheshireType widetype = getWidestNumericalType(ltype, rtype); \
                    if (!areEqualTypes(ltype, widetype)) \
                        PANIC("Not possible to store widened type into lvalue for %s", reason); \
                    WIDEN_NODE(widetype, node); \
                } else if (isValidObjectType(ltype) && isValidObjectType(rtype)) { \
                    /*todo: check for ancestry. null case too; if it's null, you don't have to cast.*/ \
                } else \
                    PANIC("left and right types must both be numerical or objects for %s", reason); \
            }

//////////////// STATICS /////////////////
extern ValidLambdaSet validLambdaSet;
//////////////////////////////////////////

void typeCheckTopNode(CheshireScope* scope, ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            PANIC("Type system received PRT_NONE.");
            break;
        case PRT_METHOD_DECLARATION:
            addMethodDeclaration(scope, node->method.functionName, node->method.type, node->method.params);
            break;
        case PRT_METHOD_DEFINITION:
            addMethodDeclaration(scope, node->method.functionName, node->method.type, node->method.params);
            raiseScope(scope);
            typeCheckBlockList(scope, node->method.body);
            fallScope(scope);
            break;
    }
}

void typeCheckParameterList(CheshireScope*, ParserTopNode*) {
    //todo
    //include null case.
}

CheshireType typeCheckExpressionNode(CheshireScope* scope, ExpressionNode* node) {
    switch (node->type) {
        case OP_NOP:
            PANIC("No such operation as No-OP");
            return TYPE_VOID;
        case OP_NOT: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            if (isBoolean(childType)) {
                return TYPE_BOOLEAN;
            } else {
                PANIC("Expected type \"boolean\" for operation NOT");
            }
            return TYPE_BOOLEAN;
        }
        case OP_PLUSONE:
        case OP_MINUSONE: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            return childType;
        }
        case OP_COMPL:
        case OP_UNARY_MINUS: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            if (!isNumericalType(childType))
                PANIC("Expected a numerical type for operations COMPL and UNARY -");
            return childType;
        }
        case OP_GRE_EQUALS:
        case OP_LES_EQUALS:
        case OP_GREATER:
        case OP_LESS:
        case OP_EQUALS:
        case OP_NOT_EQUALS: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            //todo: protect against void
            if (!areEqualTypes(left, right)) {
                if (isNumericalType(left) && isNumericalType(right)) {
                    CheshireType widetype = getWidestNumericalType(left, right);
                    if (!areEqualTypes(widetype, left)) WIDEN_NODE(widetype, node->binary.left);
                    if (!areEqualTypes(widetype, right)) WIDEN_NODE(widetype, node->binary.right);
                } else if ((node->type == OP_EQUALS || node->type == OP_NOT_EQUALS) && isValidObjectType(left) && isValidObjectType(right)) {
                    //todo: check for ancestry, null case too.
                } else
                    PANIC("left and right types must be numerical for operations >=, <=, >, <, including object for == and !=");
            }
            return TYPE_BOOLEAN;
        }
        case OP_PLUS:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_MOD: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            if (!areEqualTypes(left, right)) {
                if (isNumericalType(left) && isNumericalType(right)) {
                    CheshireType widetype = getWidestNumericalType(left, right);
                    WIDEN_NODE(widetype, node->binary.left);
                    WIDEN_NODE(widetype, node->binary.right);
                } else
                    PANIC("left and right types must be numerical for operations +, -, *, /, and %%");
            }
            return left;
        }
        case OP_AND:
        case OP_OR: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            if (!isBoolean(left) || !isBoolean(right))
                PANIC("Expected type Boolean in expression \"and\" or \"or\"");
            return TYPE_BOOLEAN;
        }
        case OP_SET: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            STORE_EXPRESSION_INTO_LVAL(left, right, node->binary.right, "Operator =");
            return left;
        }
        case OP_ARRAY_ACCESS: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            if (left.arrayNesting == 0)
                PANIC("Cannot dereference a non-array type!");
            if (isVoid(left))
                PANIC("No such dereference of type void[]!");
            if (!isInt(right))
                PANIC("Index of array must be an integer!");
            left.arrayNesting--;
            return left;
        }
        case OP_INSTANCEOF: {
            CheshireType child = typeCheckExpressionNode(scope, node->instanceof.expression);
            if (!isValidObjectType(child) || !isValidObjectType(node->instanceof.type))
                PANIC("Expected object types for operation instanceof");
            //todo: check ancestry.
            return child;
        }
        case OP_VARIABLE: {
            CheshireType ret = getVariableType(scope, node->string);
            return ret;
        }
        case OP_STRING:
            return TYPE_STRING;
        case OP_CAST: {
            CheshireType cast = node->cast.type;
            CheshireType child = typeCheckExpressionNode(scope, node->cast.child);
            if (isNumericalType(cast) && isNumericalType(child)) {
                return cast;
            } else if (isValidObjectType(cast) && isValidObjectType(child)) {
                if (cast.isUnsafe && child.isUnsafe) {
                    return child;
                } else if (!cast.isUnsafe && !cast.isUnsafe) {
                    //todo: check if they're ancestral, null case
                    return child;
                } else
                    PANIC("Received a mix of unsafe and managed types in cast!");
            } else
                PANIC("Recieved invalid types for cast operation");
        }
        case OP_METHOD_CALL: {
            ExpressionList* expressions = node->methodcall.params;
            LambdaType method_signature = validLambdaSet[getMethodSignature(scope, node->methodcall.fn_name)];
            unsigned int index = 0;
            for (ExpressionList* paramNode = expressions; paramNode != NULL; paramNode = paramNode->next, index++) {
                if (index >= method_signature.second.size())
                    PANIC("Too many parameters for method call. Method takes %d parameters, more than %d parameters given!", method_signature.second.size(), index);
                CheshireType parameterExpectedType = method_signature.second[index];
                CheshireType parameterGivenType = typeCheckExpressionNode(scope, paramNode->parameter);
                STORE_EXPRESSION_INTO_LVAL(parameterExpectedType, parameterGivenType, paramNode->parameter, "parameter");
            }
            if (index != method_signature.second.size())
                PANIC("Not enough parameters for method call!");
            return method_signature.first;
        }
        case OP_CALLBACK_CALL: {
            ExpressionList* expressions = node->callbackcall.params;
            CheshireType callbackType = typeCheckExpressionNode(scope, node->callbackcall.callback);
            if (!isValidLambdaType(callbackType) || callbackType.arrayNesting != 0)
                PANIC("Type is not invocable!");
            LambdaType method_signature = validLambdaSet[callbackType.typeKey];
            unsigned int index = 0;
            for (ExpressionList* paramNode = expressions; paramNode != NULL; paramNode = paramNode->next, index++) {
                if (index >= method_signature.second.size())
                    PANIC("Too many parameters for method call!");
                CheshireType parameterExpectedType = method_signature.second[index];
                CheshireType parameterGivenType = typeCheckExpressionNode(scope, paramNode->parameter);
                STORE_EXPRESSION_INTO_LVAL(parameterExpectedType, parameterGivenType, paramNode->parameter, "parameter");
            }
            if (index != method_signature.second.size())
                PANIC("Not enough parameters for method call!");
            return method_signature.first;
        }
        case OP_NUMBER: {
            double number = node->numberValue;
            if (number == floor(number)) {
                if (number >= INT_MIN && number <= INT_MAX) {
                    printf("%f interpreted as int\n", number);
                    return TYPE_INT;
                }
                printf("%f interpreted as number\n", number);
                return TYPE_NUMBER;
            } else {
                printf("%f interpreted as decimal\n", number);
                return TYPE_DECIMAL;
            }
        }
        case OP_RESERVED_LITERAL: {
            switch (node->reserved) {
                case RL_TRUE:
                case RL_FALSE:
                    return TYPE_BOOLEAN;
                case RL_NULL:
                    return TYPE_OBJECT;
            }
        }
        case OP_LENGTH:
            return TYPE_INT;
        case OP_DEREFERENCE: {
            CheshireType child = typeCheckExpressionNode(scope, node->unaryChild);
            return child;
        }
        case OP_NEW_GC:
        case OP_NEW_HEAP:
            //todo: implement later w/ classes, return type of the object (or w/ heap, the object and ^). check params
            break;
        case OP_OBJECT_CALL:
            //todo: implement later w/ classes.
            break;
        case OP_ACCESS:
            //todo: implement later w/ classes
            break;
        case OP_SELF:
            //todo: implement later w/ classes. store self type in scope obj?
            break;
    }
    return TYPE_VOID;
}

void typeCheckStatementNode(CheshireScope* scope, StatementNode* node) {
    switch (node->type) {
        case S_NOP:
            PANIC("No such statement as No-OP!");
            break;
        case S_VARIABLE_DEF: {
            CheshireType expectedType = node->varDefinition.type;
            CheshireType givenType = typeCheckExpressionNode(scope, node->varDefinition.value);
            printCheshireType(expectedType);
            printCheshireType(givenType);
            STORE_EXPRESSION_INTO_LVAL(expectedType, givenType, node->varDefinition.value, "variable definition");
            defineVariable(scope, node->varDefinition.variable, expectedType);
        } break;
        case S_INFER_DEF: {
            CheshireType givenType = typeCheckExpressionNode(scope, node->varDefinition.value);
            node->varDefinition.type = givenType; //"infer" the type of the variable.
            defineVariable(scope, node->varDefinition.variable, givenType);
        } break;
        case S_EXPRESSION: {
            typeCheckExpressionNode(scope, node->expression);
        } break;
        case S_ASSERT: {
            CheshireType givenType = typeCheckExpressionNode(scope, node->expression);
            if (!isBoolean(givenType))
                PANIC("Expected type boolean for assertion statement");
        } break;
        case S_BLOCK: {
            raiseScope(scope);
            typeCheckBlockList(scope, node->block);
            fallScope(scope);
        } break;
        case S_IF:{
            CheshireType condition = typeCheckExpressionNode(scope, node->expression);
            if (!isBoolean(condition))
                PANIC("Expected type boolean for if statement");
            raiseScope(scope);
            typeCheckStatementNode(scope, node->conditional.block);
            fallScope(scope);
        } break;
        case S_IF_ELSE: {
            CheshireType condition = typeCheckExpressionNode(scope, node->expression);
            if (!isBoolean(condition))
                PANIC("Expected type boolean for if-else statement");
            raiseScope(scope);
            typeCheckStatementNode(scope, node->conditional.block);
            fallScope(scope);
            
            raiseScope(scope);
            typeCheckStatementNode(scope, node->conditional.elseBlock);
            fallScope(scope);
        } break;
        case S_WHILE: {
            CheshireType condition = typeCheckExpressionNode(scope, node->expression);
            if (!isBoolean(condition))
                PANIC("Expected type boolean for while loop");
            raiseScope(scope);
            typeCheckStatementNode(scope, node->conditional.block);
            fallScope(scope);
        } break;
        case S_DELETE_HEAP: {
            CheshireType type = typeCheckExpressionNode(scope, node->expression);
            if (!type.isUnsafe)
                PANIC("Expected unsafe type in operator delete^");
        } break;
    }
}

void typeCheckBlockList(CheshireScope* scope, BlockList* list) {
    if (list == NULL)
        return;
    typeCheckStatementNode(scope, list->statement);
    typeCheckBlockList(scope, list->next);
}