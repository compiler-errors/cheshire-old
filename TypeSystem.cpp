/* 
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

#define __GXX_EXPERIMENTAL_CXX0X__ 1

#include <unordered_map>
#include <unordered_set>
#include "Structures.h"
#include "TypeSystem.h"
#include "LexerUtilities.h"

#define insertBaseType(type) { typeID = typeKeys++; saveIdentifier(type, &string); typeMap[string] = typeID; printf("Initializing type '%s' with key %d\n", string, typeID); }

//////////////// STATICS /////////////////
static Boolean isInitialized = FALSE;
static TypeMap typeMap;
static LambdaMap lambdaMap;
static ValidObjectSet validObjectSet;
ValidLambdaSet validLambdaSet; //accessible to extern'ed reference in TypeCheckNodes.cpp
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
        
        //string
        typeID = typeKeys++;
        saveIdentifier("String", &string); //type 6
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

TypeKey getMethodSignature(CheshireScope* scope, const char* name) {
    if (methodMappings.find(name) == methodMappings.end())
        PANIC("No such method as %s", name);
    return lambdaMap[methodMappings[name]];
}

void addMethodDeclaration(CheshireScope* scope, const char* name, CheshireType returnType, struct tagParameterList* params) {
    if (methodMappings.find(name) != methodMappings.end())
        PANIC("Redeclaration of method %s!", name);
    getLambdaTypeKey(returnType, params); //allocate it as a type.
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
    if (lambdaMap.find(l) != lambdaMap.end()) {
        return lambdaMap[l];
    } else {
        TypeKey typeID = typeKeys++;
        lambdaMap[l] = typeID;
        validLambdaSet[typeID] = l;
        return typeID;
    }
}

CheshireType getType(TypeKey base, Boolean isUnsafe) {
    CheshireType c;
    c.typeKey = base;
    c.arrayNesting = 0;
    c.isUnsafe = FALSE;
    if (isUnsafe && !isValidObjectType(c)) {
        printf("Error in type: \"");
        printCheshireType(c);
        printf("\". ");
        PANIC("Cannot apply \"unsafe\" to a non-object type!"); //using 'c' because it has the key, but no ^ applied yet.
    }
    c.isUnsafe = isUnsafe;
    return c;
}

Boolean areEqualTypes(CheshireType left, CheshireType right) {
    return (Boolean) (left.typeKey == right.typeKey && left.isUnsafe == right.isUnsafe && left.arrayNesting == right.arrayNesting);
}

Boolean isVoid(CheshireType t) {
    return (Boolean) (t.typeKey == 0);
}

Boolean isBoolean(CheshireType t) {
    return (Boolean) (t.typeKey == 4 && t.arrayNesting == 0);
}

Boolean isInt(CheshireType t) {
    return (Boolean) (t.typeKey == 2 && t.arrayNesting == 0);
}

Boolean isNumericalType(CheshireType t) {
    return (Boolean) (t.typeKey >= 1 && t.typeKey <= 3 && t.arrayNesting == 0); //between Int and Double
}

CheshireType getWidestNumericalType(CheshireType left, CheshireType right) {
    if (areEqualTypes(left, TYPE_NUMBER)) {
        if (areEqualTypes(right, TYPE_DECIMAL))
            PANIC("Cannot coerce types Number and Decimal together!");
        return TYPE_NUMBER;
    }
    if (areEqualTypes(left, TYPE_DECIMAL)) {
        if (areEqualTypes(right, TYPE_NUMBER))
            PANIC("Cannot coerce types Number and Decimal together!");
        return TYPE_DECIMAL;
    }
    if (areEqualTypes(left, TYPE_INT)) {
        if (areEqualTypes(right, TYPE_DECIMAL))
            return TYPE_DECIMAL;
        if (areEqualTypes(right, TYPE_NUMBER))
            return TYPE_NUMBER;
        return TYPE_INT;
    }
    PANIC("Reached unknown sequence of types in getWidestNumericalType()");
    return TYPE_VOID;
}

Boolean isValidObjectType(CheshireType t) {
    if (t.arrayNesting != 0)
        return FALSE;
    if (t.typeKey == -2) //if null
        return TRUE;
    return (Boolean) (validObjectSet.find(t.typeKey) != validObjectSet.end());
}

Boolean isValidLambdaType(CheshireType t) {
    if (t.arrayNesting != 0)
        return FALSE;
    return (Boolean) (validLambdaSet.find(t.typeKey) != validLambdaSet.end());
}

void printCheshireType(CheshireType node) {
    TypeKey t = node.typeKey;
    CheshireType raw = {node.typeKey, FALSE, 0};
    if (isValidLambdaType(raw)) {
        LambdaType l = validLambdaSet[t];
        printCheshireType(l.first);
        printf("::(");
        for (size_t i = 0; i < l.second.size(); i++) {
            if (i != 0)
                printf(", ");
            printCheshireType(l.second[i]);
        }
        printf(")");
    } else if (isValidObjectType(raw)) {
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
                PANIC("No such recognized type as: %d, isUnsafe = %s, arrayNesting = %d", t, node.isUnsafe ? "True" : "False", node.arrayNesting);
                break;
        }
    }
    for (int i = 0; i < node.arrayNesting; i++)
        printf("[]");
}

CheshireType isSupertype(CheshireScope* type, CheshireType super, CheshireType sub) {
    //todo: assume types are valid objects
    //TYPE_NULL is a always a subtype.
}

//CheshireType getSupertype(CheshireScope* scope, CheshireType type) {
//    //todo: assume that "type" is a valid type...
//}
