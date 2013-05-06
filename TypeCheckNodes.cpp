/*
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

#include "Structures.h"
#include "TypeSystem.h"
#include "SyntaxTreeUtil.h"
#include "LexerUtilities.h"
#include <cmath>
#include <climits>

using std::floor;

#define WIDEN_NODE(newtype, oldtype, node) if (!equalTypes(newtype, oldtype)) { node = createCastOperation(node, newtype); typeCheckExpressionNode(scope, node); }

/* This "method" is defined to implement the common "store-into-variable-and-widen-if-necessary"
 * method that is used for parameters, storing things into lval's, and also variable definitions.
 */
#define STORE_EXPRESSION_INTO_LVAL(ltype, rtype, node, reason) \
    if (!equalTypes(ltype, rtype)) { \
        if (isNumericalType(ltype) && isNumericalType(rtype)) { \
            CheshireType widetype = getWidestNumericalType(ltype, rtype); \
            ERROR_IF(!equalTypes(widetype, ltype), "Cannot store widened number into ltype!") \
            WIDEN_NODE(widetype, rtype, node); \
        } else if (equalTypes(TYPE_NULL, rtype) && (isObjectType(ltype) || isLambdaType(ltype) || ltype.arrayNesting > 0)) { \
            WIDEN_NODE(ltype, rtype, node); \
        } else if (isObjectType(ltype) && isObjectType(rtype)) { \
            if (!isSuper(ltype, rtype)) { \
                PANIC("Cannot store type into non-super-type!"); \
            } \
            WIDEN_NODE(ltype, rtype, node); \
        } else \
            PANIC("left and right types must be both numerical, object or the same exact type for %s", reason); \
    }

//////////////// STATICS /////////////////
extern KeyedLambdas keyedLambdas;
extern ObjectMapping objectMapping;
extern AncestryMap ancestryMap;
//////////////////////////////////////////

void typeCheckTopNode(CheshireScope* scope, ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            PANIC("Type system received PRT_NONE.");
            break;
        case PRT_METHOD_DEFINITION:
            setExpectedMethodType(node->method.returnType);
            raiseTypeScope(scope);

            for (ParameterList* p = node->method.params; p != NULL; p = p->next)
                defineVariable(scope, p->name, p->type);

            typeCheckBlockList(scope, node->method.body);
            fallTypeScope(scope);
            setExpectedMethodType(TYPE_VOID);
            break;
        case PRT_CLASS_DEFINITION: {
            Boolean constructor = FALSE;

            for (ClassList* c = node->classdef.classlist; c != NULL; c = c->next) {
                switch (c->type) {
                    case CLT_CONSTRUCTOR: {
                        constructor = TRUE;
                        raiseTypeScope(scope);
                        setExpectedMethodType(TYPE_VOID);

                        for (ParameterList* p = c->constructor.params; p != NULL; p = p->next)
                            defineVariable(scope, p->name, p->type);

                        CheshireType superctor = getClassVariable(node->classdef.parent, "new");

                        if (equalTypes(superctor, TYPE_VOID)) {
                            ERROR_IF(c->constructor.inheritsParams != NULL, "Expected empty parameter list for default super constructor.");
                        } else {
                            LambdaType superctorMethod = keyedLambdas[superctor];
                            unsigned int index = 1; //one parameter provided implicitly: the "self" reference.

                            for (ExpressionList* paramNode = c->constructor.inheritsParams; paramNode != NULL; paramNode = paramNode->next, index++) {
                                ERROR_IF(index >= superctorMethod.second.size(), "Too many parameters for method call. Method takes %d parameters, more than %d parameters given!", superctorMethod.second.size(), index);
                                CheshireType parameterExpectedType = superctorMethod.second[index];
                                CheshireType parameterGivenType = typeCheckExpressionNode(scope, paramNode->parameter);
                                STORE_EXPRESSION_INTO_LVAL(parameterExpectedType, parameterGivenType, paramNode->parameter, "parameter");
                            }
                        }

                        typeCheckBlockList(scope, c->constructor.block);
                        fallTypeScope(scope);
                    }
                    break;
                    case CLT_VARIABLE: {
                        CheshireType defaultValue = typeCheckExpressionNode(scope, c->variable.defaultValue);
                        STORE_EXPRESSION_INTO_LVAL(c->variable.type, defaultValue, c->variable.defaultValue, "class variable.");
                    }
                    break;
                    case CLT_METHOD: {
                        setExpectedMethodType(c->method.returnType);
                        raiseTypeScope(scope);

                        for (ParameterList* p = c->method.params; p != NULL; p = p->next)
                            defineVariable(scope, p->name, p->type);

                        typeCheckBlockList(scope, c->method.block);
                        fallTypeScope(scope);
                        setExpectedMethodType(TYPE_VOID);
                    }
                    break;
                }
            }

            if (!constructor) {
                CheshireType superctor = getClassVariable(node->classdef.parent, "new");

                if (!equalTypes(TYPE_VOID, superctor)) {
                    LambdaType superctorMethod = keyedLambdas[superctor];

                    if (!(equalTypes(superctorMethod.first, TYPE_VOID) &&
                            superctorMethod.second.size() == 1u &&
                            equalTypes(superctorMethod.second[0], getNamedType(node->classdef.name)))) {
                        PANIC("Invalid super-constructor for default method!");
                    }
                }
            }
        }
        break;
        case PRT_VARIABLE_DEFINITION:
        case PRT_METHOD_DECLARATION:
        case PRT_VARIABLE_DECLARATION:
            break;
    }
}

void defineTopNode(CheshireScope* scope, ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            PANIC("Type system received PRT_NONE.");
            break;
        case PRT_METHOD_DECLARATION:
        case PRT_METHOD_DEFINITION:
            defineVariable(scope, node->method.functionName, getLambdaType(node->method.returnType, node->method.params));
            break;
        case PRT_VARIABLE_DECLARATION:
        case PRT_VARIABLE_DEFINITION:
            defineVariable(scope, node->variable.name, node->variable.type);
            break;
        case PRT_CLASS_DEFINITION: {
            ERROR_IF(!isObjectType(node->classdef.parent), "Invalid parent type of class %s", node->classdef.name);
            int typekey = defineClass(node->classdef.name, node->classdef.classlist, node->classdef.parent);
            CStrEql streql;

            for (ClassList* c = node->classdef.classlist; c != NULL; c = c->next) {
                switch (c->type) {
                    case CLT_CONSTRUCTOR:
                        c->constructor.params = linkParameterList(((CheshireType) {
                            typekey, 0
                        }), saveIdentifierReturn("self"), c->constructor.params);

                        for (ClassList* c2 = c->next; c2 != NULL; c2 = c2->next)
                            ERROR_IF(c2->type == CLT_CONSTRUCTOR, "Class must have only one constructor!");

                        break;
                    case CLT_VARIABLE: {
                        char* name = c->variable.name;

                        for (ClassList* c2 = c->next; c2 != NULL; c2 = c2->next) {
                            if (c2->type == CLT_VARIABLE)
                                ERROR_IF(streql(name, c2->variable.name), "Multiple definition of %s", name);

                            if (c2->type == CLT_METHOD)
                                ERROR_IF(streql(name, c2->method.name), "Multiple definition of %s", name);
                        }

                        for (TypeKey ancestor = node->classdef.parent.typeKey; !equalTypes( {ancestor, 0}, TYPE_OBJECT); ancestor = ancestryMap[ancestor]) {
                            if (ancestor == getNamedType(node->classdef.name).typeKey)
                                PANIC("Circular reference to class %s", node->classdef.name);

                            for (ClassList* c2 = objectMapping[ancestor]; c2 != NULL; c2 = c2->next) {
                                if (c2->type == CLT_VARIABLE)
                                    ERROR_IF(streql(name, c2->variable.name), "Multiple definition of %s", name);

                                if (c2->type == CLT_METHOD)
                                    ERROR_IF(streql(name, c2->method.name), "Multiple definition of %s", name);
                            }
                        }
                        break;
                    }
                    case CLT_METHOD: {
                        c->method.params = linkParameterList(((CheshireType) {
                            typekey, 0
                        }), saveIdentifierReturn("self"), c->method.params);
                        char* name = c->method.name;

                        for (ClassList* c2 = c->next; c2 != NULL; c2 = c2->next) {
                            if (c2->type == CLT_VARIABLE)
                                ERROR_IF(streql(name, c2->variable.name), "Multiple definition of %s", name);

                            if (c2->type == CLT_METHOD) {
                                ERROR_IF(streql(name, c2->method.name), "Multiple definition of %s", name);
                            }
                        }

                        for (TypeKey ancestor = node->classdef.parent.typeKey; !equalTypes( {ancestor, 0}, TYPE_OBJECT); ancestor = ancestryMap[ancestor]) {
                            if (ancestor == getNamedType(node->classdef.name).typeKey)
                                PANIC("Circular reference to class %s", node->classdef.name);

                            for (ClassList* c2 = objectMapping[ancestor]; c2 != NULL; c2 = c2->next) {
                                if (c2->type == CLT_VARIABLE)
                                    ERROR_IF(streql(name, c2->variable.name), "Multiple definition of %s", name);

                                if (c2->type == CLT_METHOD) {
                                    //check for override.
                                    if (streql(name, c2->method.name)) {
                                        if (equalTypes(c->method.returnType, c2->method.returnType)) {
                                            for (ParameterList* a = c->method.params->next, * b = c2->method.params->next; true; a = a->next, b = b->next) {
                                                if (a == NULL && b == NULL) {
                                                    break;
                                                } else if (a == NULL || b == NULL) { //otherwise, if only 1 is null...
                                                    PANIC("Invalid override of %s -- unmatching parameter list sizes.", name);
                                                } else {
                                                    ERROR_IF(!equalTypes(a->type, b->type), "Invalid override of %s -- unmatching parameter types.", name);
                                                }
                                            }
                                        } else
                                            PANIC("Invalid override of %s -- unmatching return types.", name);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
}

CheshireType typeCheckExpressionNode(CheshireScope* scope, ExpressionNode* node) {
    switch (node->type) {
        case OP_NOP:
            PANIC("No such operation as No-OP");
            return node->determinedType = TYPE_VOID;
        case OP_NOT: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);

            if (isBoolean(childType)) {
                return node->determinedType = TYPE_BOOLEAN;
            } else {
                PANIC("Expected type \"boolean\" for operation NOT");
            }

            return node->determinedType = TYPE_BOOLEAN;
        }
        case OP_PLUSONE:
        case OP_MINUSONE: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            return node->determinedType = childType;
        }
        case OP_COMPL: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            ERROR_IF(!isNumericalType(childType), "Expected a numerical type for operation: COMPL.");
            ERROR_IF(equalTypes(childType, TYPE_DECIMAL), "Unexpected type: 'double' for COMPL operation!");
            return node->determinedType = childType;
        }
        case OP_UNARY_MINUS: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            ERROR_IF(!isNumericalType(childType), "Expected a numerical type for operation: UNARY -");
            return node->determinedType = childType;
        }
        case OP_GRE_EQUALS:
        case OP_LES_EQUALS:
        case OP_GREATER:
        case OP_LESS:
        case OP_EQUALS:
        case OP_NOT_EQUALS: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            ERROR_IF(isVoid(left) || isVoid(right), "Cannot compare void type in operations >=, <=, >, <, !=, or ==");

            if (isNumericalType(left) && isNumericalType(right)) {
                CheshireType widetype = getWidestNumericalType(left, right);
                WIDEN_NODE(widetype, left, node->binary.left);
                WIDEN_NODE(widetype, right, node->binary.right);
            } else if ((node->type == OP_EQUALS || node->type == OP_NOT_EQUALS) && isObjectType(left) && isObjectType(right)) {
                if (!equalTypes(left, TYPE_NULL) && !equalTypes(right, TYPE_NULL)) {
                    ERROR_IF(!isSuper(left, right) && !isSuper(right, left), "Comparison must be in a super-subtype relationship.");
                }
            } else
                PANIC("left and right types must be numerical for operations >=, <=, >, <, including object for == and !=");

            return node->determinedType = TYPE_BOOLEAN;
        }
        case OP_PLUS:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_MOD: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);

            if (isNumericalType(left) && isNumericalType(right)) {
                CheshireType widetype = getWidestNumericalType(left, right);
                WIDEN_NODE(widetype, left, node->binary.left);
                WIDEN_NODE(widetype, right, node->binary.right);
                return node->determinedType = widetype;
            } else
                PANIC("left and right types must be numerical for operations +, -, *, /, and %%");

            return node->determinedType = left;
        }
        case OP_AND:
        case OP_OR: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            ERROR_IF(!isBoolean(left) || !isBoolean(right), "Expected type Boolean in expression \"and\" or \"or\"");
            return node->determinedType = TYPE_BOOLEAN;
        }
        case OP_SET: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            STORE_EXPRESSION_INTO_LVAL(left, right, node->binary.right, "Operator =");
            return node->determinedType = left;
        }
        case OP_ARRAY_ACCESS: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            ERROR_IF(getArrayNesting(left) == 0, "Cannot dereference a non-array type!");
            ERROR_IF(isVoid(left), "No such dereference of type void[]!");
            ERROR_IF(!isInt(right), "Index of array must be an integer!");
            return node->determinedType = getArrayDereference(left);
        }
        case OP_INSTANCEOF: {
            CheshireType child = typeCheckExpressionNode(scope, node->instanceof.expression);
            ERROR_IF(!isObjectType(child) || !isObjectType(node->instanceof.type), "Expected object types for operation instanceof");
            ERROR_IF(!isSuper(child, node->instanceof.type) || !isSuper(node->instanceof.type, child), "Instanceof may only compare type relationships that are possible!");
            return node->determinedType = TYPE_BOOLEAN;
        }
        case OP_VARIABLE: {
            CheshireType ret = getVariableType(scope, node->string);
            return node->determinedType = ret;
        }
        case OP_STRING:
            return node->determinedType = TYPE_STRING;
        case OP_CAST: {
            CheshireType cast = node->cast.type;
            CheshireType child = typeCheckExpressionNode(scope, node->cast.child);

            if (isObjectType(cast) && equalTypes(TYPE_NULL, child)) {//null case!
                return node->determinedType = cast;
            }

            //ERROR_IF(isNumericalType(cast) ^ isNumericalType(child), "Received a mix of numerical and non-numerical types in cast!");
            //ERROR_IF(isObjectType(cast) ^ isObjectType(child), "Received a mix of object and non-object types in cast!");
            //ERROR_IF(isLambdaType(cast) || isLambdaType(child), "Cannot operate on lambda types in cast!");
            //ERROR_IF(cast.arrayNesting > 0, "Cannot cast arrays!");
            return node->determinedType = cast;
        }
        case OP_METHOD_CALL: {
            ExpressionList* expressions = node->methodcall.params;
            CheshireType functionType = typeCheckExpressionNode(scope, node->methodcall.callback);
            ERROR_IF(!isLambdaType(functionType), "Type is not invocable!");
            LambdaType method_signature = keyedLambdas[functionType];
            unsigned int index = 0;

            for (ExpressionList* paramNode = expressions; paramNode != NULL; paramNode = paramNode->next, index++) {
                ERROR_IF(index >= method_signature.second.size(), "Too many parameters for method call. Method takes %d parameters, more than %d parameters given!", method_signature.second.size(), index);
                CheshireType parameterExpectedType = method_signature.second[index];
                CheshireType parameterGivenType = typeCheckExpressionNode(scope, paramNode->parameter);
                STORE_EXPRESSION_INTO_LVAL(parameterExpectedType, parameterGivenType, paramNode->parameter, "parameter");
            }

            ERROR_IF(index != method_signature.second.size(), "Not enough parameters for method call!");
            return node->determinedType = method_signature.first;
        }
        case OP_RESERVED_LITERAL: {
            switch (node->reserved) {
                case RL_TRUE:
                case RL_FALSE:
                    return node->determinedType = TYPE_BOOLEAN;
                case RL_NULL:
                    return node->determinedType = TYPE_NULL;
            }
        }
        case OP_INTEGER:
            //todo: make sure it is within the regular integer bounds -2^31 to 2^31-1 or whatever.
            return node->determinedType = TYPE_INT;
        case OP_DECIMAL:
            return node->determinedType = TYPE_DECIMAL;
        case OP_CHAR:
            return node->determinedType = TYPE_I8;
        case OP_DEREFERENCE: {
            CheshireType child = typeCheckExpressionNode(scope, node->unaryChild);
            return node->determinedType = child;
        }
        case OP_CLOSURE: {
            auto oldscope = scope->highestScope;

            for (; scope->highestScope->parentScope != NULL; scope->highestScope = scope->highestScope->parentScope);

            CheshireType currentExpectedType = getExpectedMethodType();
            setExpectedMethodType(node->closure.type);
            raiseTypeScope(scope);
            auto newscope = scope->highestScope;

            for (UsingList* u = node->closure.usingList; u != NULL; u = u->next) {
                scope->highestScope = oldscope;
                CheshireType vartype = getVariableType(scope, u->variable);
                u->type = vartype;
                scope->highestScope = newscope;
                defineVariable(scope, u->variable, vartype);
            }

            for (ParameterList* p = node->closure.params; p != NULL; p = p->next)
                defineVariable(scope, p->name, p->type);

            typeCheckBlockList(scope, node->closure.body);
            fallTypeScope(scope);
            scope->highestScope = oldscope; //restore old scoping.
            setExpectedMethodType(currentExpectedType);
            return node->determinedType = getLambdaType(node->closure.type, node->closure.params);
        }
        case OP_ACCESS: {
            CheshireType childType = typeCheckExpressionNode(scope, node->access.expression);
            CheshireType ret = getClassVariable(childType, node->access.variable);
            ERROR_IF(equalTypes(ret, TYPE_VOID), "Could not find variable %s", node->access.variable);
            return node->determinedType = ret;
        }
        case OP_INSTANTIATION: {
            ERROR_IF(!isObjectType(node->instantiate.type), "Cannot instantiate a non-object type!");
            CheshireType methodType = getClassVariable(node->instantiate.type, "new");

            if (equalTypes(TYPE_VOID, methodType)) {
                ERROR_IF(node->instantiate.params != NULL, "Expected empty parameter list for default constructor!");
                return node->determinedType = node->instantiate.type;
            }

            ERROR_IF(!isLambdaType(methodType), "Fatal constructor error!");
            LambdaType method = keyedLambdas[methodType];
            unsigned int index = 1; //one parameter provided implicitly: the "self" reference.

            for (ExpressionList* paramNode = node->instantiate.params; paramNode != NULL; paramNode = paramNode->next, index++) {
                ERROR_IF(index >= method.second.size(), "Too many parameters for method call. Method takes %d parameters, more than %d parameters given!", method.second.size(), index);
                CheshireType parameterExpectedType = method.second[index];
                CheshireType parameterGivenType = typeCheckExpressionNode(scope, paramNode->parameter);
                STORE_EXPRESSION_INTO_LVAL(parameterExpectedType, parameterGivenType, paramNode->parameter, "parameter");
            }

            ERROR_IF(index != method.second.size(), "Not enough parameters for method call!");
            return node->determinedType = node->instantiate.type;
        }
        case OP_OBJECT_CALL: {
            CheshireType obj = typeCheckExpressionNode(scope, node->objectcall.object);
            CheshireType methodType = getClassVariable(obj, node->objectcall.method);
            ERROR_IF(!isLambdaType(methodType), "Cannot invoke non-lambda class member.");
            LambdaType method = keyedLambdas[methodType];
            ERROR_IF(method.second.size() == 0, "Lambda type not valid for object call syntax!");
            unsigned int index = 1; //one parameter provided implicitly: the "self" reference.

            for (ExpressionList* paramNode = node->objectcall.params; paramNode != NULL; paramNode = paramNode->next, index++) {
                ERROR_IF(index >= method.second.size(), "Too many parameters for method call. Method takes %d parameters, more than %d parameters given!", method.second.size(), index);
                CheshireType parameterExpectedType = method.second[index];
                CheshireType parameterGivenType = typeCheckExpressionNode(scope, paramNode->parameter);
                STORE_EXPRESSION_INTO_LVAL(parameterExpectedType, parameterGivenType, paramNode->parameter, "parameter");
            }

            ERROR_IF(index != method.second.size(), "Not enough parameters for method call!");
            return node->determinedType = method.first;
        }
    }

    PANIC("FATAL ERROR IN TYPE CHECKING.");
}

void typeCheckStatementNode(CheshireScope* scope, StatementNode* node) {
    switch (node->type) {
        case S_NOP:
            PANIC("No such statement as No-OP!");
            break;
        case S_VARIABLE_DEF: {
            CheshireType expectedType = node->varDefinition.type;
            CheshireType givenType = typeCheckExpressionNode(scope, node->varDefinition.value);
            defineVariable(scope, node->varDefinition.variable, expectedType);
            STORE_EXPRESSION_INTO_LVAL(expectedType, givenType, node->varDefinition.value, "variable definition");
        }
        break;
        case S_INFER_DEF: {
            CheshireType givenType = typeCheckExpressionNode(scope, node->varDefinition.value);
            defineVariable(scope, node->varDefinition.variable, givenType);
            node->varDefinition.type = givenType; //"infer" the type of the variable.

            if (isNull(givenType))
                PANIC("Cannot infer type of TYPE_NULL!");

            printf("Inferred ");
            printType(givenType);
            printf(" for expression: ");
            printExpression(node->varDefinition.value);
            printf("\n");
        }
        break;
        case S_EXPRESSION: {
            typeCheckExpressionNode(scope, node->expression);
        }
        break;
        case S_ASSERT: {
            CheshireType givenType = typeCheckExpressionNode(scope, node->expression);

            if (!isBoolean(givenType))
                PANIC("Expected type boolean for assertion statement");
        }
        break;
        case S_BLOCK: {
            raiseTypeScope(scope);
            typeCheckBlockList(scope, node->block);
            fallTypeScope(scope);
        }
        break;
        case S_IF: {
            CheshireType condition = typeCheckExpressionNode(scope, node->expression);

            if (!isBoolean(condition))
                PANIC("Expected type boolean for if statement");

            raiseTypeScope(scope);
            typeCheckStatementNode(scope, node->conditional.block);
            fallTypeScope(scope);
        }
        break;
        case S_IF_ELSE: {
            CheshireType condition = typeCheckExpressionNode(scope, node->expression);

            if (!isBoolean(condition))
                PANIC("Expected type boolean for if-else statement");

            raiseTypeScope(scope);
            typeCheckStatementNode(scope, node->conditional.block);
            fallTypeScope(scope);
            raiseTypeScope(scope);
            typeCheckStatementNode(scope, node->conditional.elseBlock);
            fallTypeScope(scope);
        }
        break;
        case S_WHILE: {
            CheshireType condition = typeCheckExpressionNode(scope, node->expression);

            if (!isBoolean(condition))
                PANIC("Expected type boolean for while loop");

            raiseTypeScope(scope);
            typeCheckStatementNode(scope, node->conditional.block);
            fallTypeScope(scope);
        }
        break;
        case S_RETURN: {
            CheshireType type = typeCheckExpressionNode(scope, node->expression);
            CheshireType expected = getExpectedMethodType();

            if (equalTypes(expected, TYPE_VOID)) {
                if (!equalTypes(type, TYPE_NULL))
                    PANIC("Cannot return non-void from a void method!");
            } else {
                STORE_EXPRESSION_INTO_LVAL(expected, type, node->expression, "return statement");
            }
        }
        break;
    }
}

void typeCheckBlockList(CheshireScope* scope, BlockList* list) {
    if (list == NULL)
        return;

    typeCheckStatementNode(scope, list->statement);
    typeCheckBlockList(scope, list->next);
}
