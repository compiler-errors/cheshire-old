/*
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

//I need to define this manually, or else that
#define __GXX_EXPERIMENTAL_CXX0X__ 1

#include <unordered_map>
#include <unordered_set>
#include "Structures.h"
#include "TypeSystem.h"
#include "LexerUtilities.h"

#define insertBaseType(type) { typeID = typeKeys++; saveIdentifier(type, &string); namedObjects[string] = typeID; printf("Initializing type '%s' with key %d\n", string, typeID); }

//////////////// STATICS /////////////////
static Boolean isInitialized = FALSE;
static NamedObjects namedObjects;
static LambdaTypes lambdaTypes;
static ObjectNamings objectNamings;
KeyedLambdas keyedLambdas; //externalized for TypeCheckNodes.cpp, todo: find another way to do this: It's ugly.
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
        namedObjects[string] = typeID;
        printf("Initializing type '%s' with key %d\n", string, typeID);
        objectNamings[typeID] = string;
        //string
        typeID = typeKeys++;
        saveIdentifier("String", &string); //type 6
        namedObjects[string] = typeID;
        printf("Initializing type '%s' with key %d\n", string, typeID);
        objectNamings[typeID] = string;
    }

    isInitialized = TRUE;
}

void freeTypeSystem() {
    isInitialized = FALSE;

    for (auto iterator = namedObjects.begin(); iterator != namedObjects.end(); ++iterator)
        free((char*)(iterator->first));

    namedObjects.clear();
    lambdaTypes.clear();
    objectNamings.clear();
    keyedLambdas.clear();
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

void setExpectedMethodType(CheshireScope* scope, CheshireType type) {
    scope->expectedType = type;
}

CheshireType getExpectedMethodType(CheshireScope* scope) {
    return scope->expectedType;
}

CheshireType getMethodSignature(CheshireScope* scope, const char* name) {
    if (methodMappings.find(name) == methodMappings.end())
        PANIC("No such method as %s", name);

    return methodMappings[name];
}

void addMethodDeclaration(CheshireScope* scope, const char* name, CheshireType returnType, struct tagParameterList* params) {
    if (methodMappings.find(name) != methodMappings.end())
        PANIC("Redeclaration of method %s!", name);

    getLambdaType(returnType, params); //todo: this method isn't pretty, but I need to make sure the lambda is a type.
    methodMappings[name] = getLambdaType(returnType, params);
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
    return (Boolean)(namedObjects.find(str) != namedObjects.end());
}

CheshireType getNamedType(const char* str, Boolean isUnsafe) {
    CheshireType ret;

    if (isTypeName(str)) {
        ret.typeKey = namedObjects[str];
    } else {
        //otherwise, make it into a new type.
        char* string_copy;
        TypeKey typeID = typeKeys++;
        saveIdentifier(str, &string_copy);
        namedObjects[string_copy] = typeID;
        objectNamings[typeID] = string_copy;
        ret.typeKey = typeID;
    }

    if (isUnsafe && !isObjectType(ret)) {
        printf("Error in type: \"");
        printCheshireType(ret);
        printf("\". ");
        PANIC("Cannot apply \"unsafe\" to a non-object type!"); //using 'c' because it has the key, but no ^ applied yet.
    }

    ret.isUnsafe = isUnsafe;
    return ret;
}

CheshireType getLambdaType(CheshireType returnType, ParameterList* parameters) {
    LambdaType l = prepareLambdaType(returnType, parameters);

    if (lambdaTypes.find(l) != lambdaTypes.end()) {
        return lambdaTypes[l];
    } else {
        TypeKey typeID = typeKeys++;
        CheshireType t = {typeID, FALSE, 0};
        lambdaTypes[l] = t;
        keyedLambdas[t] = l;
        return t;
    }
}

Boolean equalTypes(CheshireType left, CheshireType right) {
    return (Boolean)(left.typeKey == right.typeKey && left.isUnsafe == right.isUnsafe && left.arrayNesting == right.arrayNesting);
}

Boolean isVoid(CheshireType t) {
    return (Boolean)(t.typeKey == 0);
}

Boolean isUnsafe(CheshireType t) {
    return t.isUnsafe;
}

Boolean isBoolean(CheshireType t) {
    return (Boolean)(t.typeKey == 4 && t.arrayNesting == 0);
}

Boolean isInt(CheshireType t) {
    return (Boolean)(t.typeKey == 2 && t.arrayNesting == 0);
}

Boolean isNumericalType(CheshireType t) {
    return (Boolean)(t.typeKey >= 1 && t.typeKey <= 3 && t.arrayNesting == 0);  //between Int and Double
}

int getArrayNesting(CheshireType t) {
    return t.arrayNesting;
}

CheshireType getArrayDereference(CheshireType t) {
    t.arrayNesting--;
    return t;
}

CheshireType getWidestNumericalType(CheshireType left, CheshireType right) {
    if (equalTypes(left, TYPE_NUMBER)) {
        if (equalTypes(right, TYPE_DECIMAL))
            PANIC("Cannot coerce types Number and Decimal together!");

        return TYPE_NUMBER;
    }

    if (equalTypes(left, TYPE_DECIMAL)) {
        if (equalTypes(right, TYPE_NUMBER))
            PANIC("Cannot coerce types Number and Decimal together!");

        return TYPE_DECIMAL;
    }

    if (equalTypes(left, TYPE_INT)) {
        if (equalTypes(right, TYPE_DECIMAL))
            return TYPE_DECIMAL;

        if (equalTypes(right, TYPE_NUMBER))
            return TYPE_NUMBER;

        return TYPE_INT;
    }

    PANIC("Reached unknown sequence of types in getWidestNumericalType()");
    return TYPE_VOID;
}

Boolean isObjectType(CheshireType t) {
    if (t.arrayNesting != 0)
        return FALSE;

    if (t.typeKey == -2) //if null
        return TRUE;

    return (Boolean)(objectNamings.find(t.typeKey) != objectNamings.end());
}

Boolean isLambdaType(CheshireType t) {
    if (t.arrayNesting != 0)
        return FALSE;

    return (Boolean)(keyedLambdas.find(t) != keyedLambdas.end());
}

void printCheshireType(CheshireType node) {
    TypeKey t = node.typeKey;
    CheshireType raw = {node.typeKey, FALSE, 0};

    if (isLambdaType(raw)) {
        LambdaType l = keyedLambdas[raw];
        printCheshireType(l.first);
        printf("::(");

        for (size_t i = 0; i < l.second.size(); i++) {
            if (i != 0)
                printf(", ");

            printCheshireType(l.second[i]);
        }

        printf(")");
    } else if (isObjectType(raw)) {
        if (raw.typeKey == -2)
            printf("NULL_TYPE");
        else
            printf("%s%s", objectNamings[t], node.isUnsafe ? "^" : "");
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
                PANIC("No such recognized type as: %d, isUnsafe = %s, arrayNesting = %d", t, node.isUnsafe ? "True" : "False", node.arrayNesting);
                break;
        }
    }

    for (int i = 0; i < node.arrayNesting; i++)
        printf("[]");
}

Boolean isSuper(CheshireType super, CheshireType sub) {
    return FALSE;
}
