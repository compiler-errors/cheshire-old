/* 
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Structures.h"
#include "TypeSystem.h"
#include "TypeSystemUtilities.hpp"
#include "LexerUtilities.h"

#define insertBaseType(type) { typeID = typeKeys++; saveIdentifier(type, &string); typeMap[string] = typeID; printf("Initializing type '%s' with key %d\n", string, typeID); }

//////////////// STATICS /////////////////
static Boolean isInitialized = FALSE;
static TypeMap typeMap;
static LambdaMap lambdaMap;
static ValidObjectSet validObjectSet;
static ValidLambdaSet validLambdaSet;
static MethodMappings methodMappings;
static TypeKey typeKeys = 0;
//////////////////////////////////////////
static LambdaType prepareLambdaType(CheshireType returnType, ParameterList* parameters) {
    LambdaType ret;
    ret.first = returnType;
    
    int count = 0;
    for (ParameterList* p = parameters; p != NULL; p = p->next, count++) {}
    ret.second = Array<CheshireType>(count);
    int i = 0;
    for (ParameterList* p = parameters; p != NULL; p = p->next, i++) {
        if (isVoid(p->type))
            PANIC("Void type not expected in function parameter!");
        ret.second[i] = p->type;
    }
    
    return ret;
}
//////////////////////////////////////////

void initTypeSystem() {
    if (!isInitialized) {
        char* string;
        int typeID = 0;
        insertBaseType("void");   //type 0
        insertBaseType("Number"); //type 1
        insertBaseType("Int");    //type 2
        insertBaseType("Decimal");//type 3
        insertBaseType("Boolean");//type 4
        
        //object
        typeID = typeKeys++;
        saveIdentifier("Object", &string); //type 5
        typeMap[string] = typeID;
        printf("Initializing type '%s' with key %d\n", string, typeID);
        validObjectSet[typeID] = string;
    }
    isInitialized = TRUE;
}

void freeTypeSystem() {
    isInitialized = FALSE;
    for (auto iterator = typeMap.begin(); iterator != typeMap.end(); ++iterator)
        free((char*) (iterator->first));
    typeMap.clear();
    lambdaMap.clear();
    validObjectSet.clear();
    validLambdaSet.clear();
    methodMappings.clear();
    typeKeys = 0;
}

CheshireScope* allocateCheshireScope() {
    CheshireScope* scope = new CheshireScope;
    scope->highestScope = NULL;
    return scope;
}

void deleteCheshireScope(CheshireScope* scope) {
    if (scope->highestScope != NULL)
        PANIC("Scope tracking has not exited from a higher scope!"); //todo: weird wording.
    delete scope;
}

void raiseScope(CheshireScope* scope) {
    VariableScope* old = scope->highestScope;
    scope->highestScope = new VariableScope;
    scope->highestScope->parentScope = old;
}

void fallScope(CheshireScope* scope) {
    VariableScope* old = scope->highestScope;
    if (old == NULL)
        PANIC("Trying to fall to a non-existent scope!");
    scope->highestScope = scope->highestScope->parentScope;
    delete old;
}

void addMethodDeclaration(CheshireScope* scope, const char* name, CheshireType returnType, struct tagParameterList* params) {
    if (methodMappings.find(name) != methodMappings.end())
        PANIC("Redeclaration of method %s!", name);
    methodMappings[name] = prepareLambdaType(returnType, params);
}

CheshireType getVariableType(CheshireScope* scope, const char* name) {
    for (VariableScope* variableScope = scope->highestScope; variableScope != NULL; variableScope = variableScope->parentScope) {
        auto iterator = variableScope->variables.find(name);
        if (iterator != variableScope->variables.end()) {
            return iterator->second;
        }
    }
    PANIC("No such variable defined as %s.", name);
}

void defineVariable(CheshireScope* scope, const char* name, CheshireType type) {
    if (isVoid(type))
        PANIC("Cannot define variable of type VOID.");
    if (scope->highestScope->variables.find(name) != scope->highestScope->variables.end())
        PANIC("Redeclaration of variable %s!", name);
    scope->highestScope->variables[name] = type;
}

Boolean isTypeName(const char* str) {
    return (Boolean) (typeMap.find(str) != typeMap.end());
}

TypeKey getTypeKey(const char* str) {
    if (isTypeName(str)) {
        return typeMap[str];
    } else {
        //otherwise, make it into a new type.
        char* string_copy;
        TypeKey typeID = typeKeys++;
        saveIdentifier(str, &string_copy);
        typeMap[string_copy] = typeID;
        validObjectSet[typeID] = string_copy;
        return typeID;
    }
}

TypeKey getLambdaTypeKey(CheshireType returnType, ParameterList* parameters) {
    LambdaType l = prepareLambdaType(returnType, parameters);
    if (lambdaMap.find(l) != lambdaMap.end())
        return lambdaMap[l];
    else {
        TypeKey typeID = typeKeys++;
        lambdaMap[l] = typeID;
        validLambdaSet[typeID] = l;
        return typeID;
    }
}

CheshireType getType(TypeKey base, Boolean isUnsafe) {
    CheshireType c;
    c.typeKey = base;
    if (isUnsafe && (validObjectSet.find(base) == validObjectSet.end())) {
        printf("Error in type: \"");
        printCheshireType(c);
        printf("\". ");
        PANIC("Cannot apply \"unsafe\" to a non-object type!"); //using 'c' because it has the key, but no ^ applied yet.
    }
    c.isInfer = FALSE;
    c.isUnsafe = isUnsafe;
    return c;
}

Boolean isVoid(CheshireType t) {
    return (Boolean) (t.typeKey == 0);
}

Boolean isNumericalType(CheshireType t) {
    return (Boolean) (t.typeKey >= 1 && t.typeKey <= 3 && t.arrayNesting == 0); //between Int and Double
}

Boolean isValidObjectType(CheshireType t) {
    if (t.arrayNesting != 0)
        return FALSE;
    return (Boolean) (validObjectSet.find(t.typeKey) != validObjectSet.end());
}

Boolean isValidLambdaType(CheshireType t) {
    if (t.arrayNesting != 0)
        return FALSE;
    return (Boolean) (validLambdaSet.find(t.typeKey) != validLambdaSet.end());
}

void printCheshireType(CheshireType node) {
    TypeKey t = node.typeKey;
    if (isValidLambdaType(node)) {
        LambdaType l = validLambdaSet[t];
        printCheshireType(l.first);
        printf("::(");
        for (size_t i = 0; i < l.second.size(); i++) {
            if (i != 0)
                printf(", ");
            printCheshireType(l.second[i]);
        }
        printf(")");
    } else if (isValidObjectType(node)) {
        printf("%s%s", validObjectSet[t], node.isUnsafe ? "^" : "");
    } else {
        //literal type.
        switch (t) {
            case 0:
                printf("void");
                break;
            case 1:
                printf("Number");
                break;
            case 2:
                printf("Int");
                break;
            case 3:
                printf("Decimal");
                break;
            case 4:
                printf("Boolean");
                break;
            default:
                PANIC("No such recognized type as: %d", t);
                break;
        }
    }
    for (int i = 0; i < node.arrayNesting; i++)
        printf("[]");
}

//CheshireType getSupertype(CheshireScope* scope, CheshireType type) {
//    //todo: assume that "type" is a valid type...
//}

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
            typeCheckBlockList(scope, node->method.body);
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
        case OP_INCREMENT:
        case OP_DECREMENT:
            PANIC("No such operation as No-OP or Increment/Decrement without \"Post-\" or \"Pre-\"");
            return TYPE_VOID;
        case OP_NOT: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            if (isBoolean(childType)) {
                return TYPE_BOOLEAN;
            } else {
                PANIC("Expected type boolean for operation NOT");
            }
            return TYPE_BOOLEAN;
        }
        case OP_COMPL:
        case OP_INCREMENT_PRE:
        case OP_DECREMENT_PRE:
        case OP_INCREMENT_POST:
        case OP_DECREMENT_POST:
        case OP_UNARY_MINUS: {
            CheshireType childType = typeCheckExpressionNode(scope, node->unaryChild);
            if (!isNumericalType(childType))
                PANIC("Expected a numerical type for operations COMPL, ++, --, and UNARY -");
            return childType;
        }
        case OP_EQUALS:
        case OP_NOT_EQUALS: {
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            //todo: protect against void
            if (!areEqualTypes(left, right))
                PANIC("left and right types must be equal for operations == and !=");
            return TYPE_BOOLEAN;
        }
        case OP_GRE_EQUALS:
        case OP_LES_EQUALS:
        case OP_GREATER:
        case OP_LESS:
        case OP_PLUS:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_MOD:
            //expect two numerals, typecast them, etc.
        case OP_AND:
        case OP_OR:
            //expect two booleans.
        case OP_SET:
            //expect two booleans
        case OP_ARRAY_ACCESS: {
            //todo: protect against void
            CheshireType left = typeCheckExpressionNode(scope, node->binary.left);
            CheshireType right = typeCheckExpressionNode(scope, node->binary.right);
            if (left.arrayNesting == 0)
                PANIC("Cannot dereference a non-array type!");
            if (!isInt(right))
                PANIC("Index of array must be an integer!");
            left.arrayNesting--;
            return left;
        } 
        case OP_INSTANCEOF:
            //expect that left is an object expression (not ^), and right is a type of object (not ^)
            break;
        case OP_VARIABLE:
            return getVariableType(scope, node->string);
        case OP_STRING:
            //return string type (idk, todo: later?)
            break;
        case OP_CAST:
            //expect: object to object type, number to number, and object^ to object^
            break;
        case OP_METHOD_CALL:
            //check params, return type that the method returns.
            break;
        case OP_CALLBACK_CALL:
            break;
        case OP_NUMBER:
            //return type of the smallest fitting numerical type.
        case OP_RESERVED_LITERAL:
            //return type of literal.
            break;
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

void typeCheckExpressionList(CheshireScope*, ExpressionList*) {
    //todo
    //include null case.
}

void typeCheckStatementNode(CheshireScope*, StatementNode*) {
    //todo
}

void typeCheckBlockList(CheshireScope*, BlockList* list) {
    if (list == NULL) {
        
    }
}