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
static NamedObjects namedObjects;
static LambdaTypes lambdaTypes;
ObjectMapping objectMapping;
static AncestryMap ancestryMap;
static ClassNames classNames;
KeyedLambdas keyedLambdas;

static CheshireType expectedType = TYPE_VOID;

static Boolean isInitialized = FALSE;
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
        objectMapping[typeID] = NULL;
        ancestryMap[typeID] = typeID;
        classNames[typeID] = "Object";
        //string
        typeID = typeKeys++;
        saveIdentifier("String", &string); //type 6
        namedObjects[string] = typeID;
        printf("Initializing type '%s' with key %d\n", string, typeID);
        objectMapping[typeID] = NULL;
        ancestryMap[typeID] = TYPE_OBJECT.typeKey;
        classNames[typeID] = "String";
    }

    isInitialized = TRUE;
}

void freeTypeSystem() {
    isInitialized = FALSE;

    for (auto iterator = namedObjects.begin(); iterator != namedObjects.end(); ++iterator)
        free((char*)(iterator->first));

    namedObjects.clear();
    lambdaTypes.clear();
    objectMapping.clear();
    keyedLambdas.clear();
    ancestryMap.clear();
    classNames.clear();
    typeKeys = 0;
}

CheshireScope* allocateCheshireScope() {
    CheshireScope* scope = new CheshireScope;
    scope->highestScope = NULL;
    raiseScope(scope);
    return scope;
}

void deleteCheshireScope(CheshireScope* scope) {
    fallScope(scope);

    if (scope->highestScope != NULL)
        PANIC("Scope tracking has not exited from a higher scope!");

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

void setExpectedMethodType(CheshireType type) {
    expectedType = type;
}

CheshireType getExpectedMethodType() {
    return expectedType;
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

int defineClass(const char* name, ClassList* classlist, CheshireType parent) {
    if (namedObjects.find(name) != namedObjects.end())
        PANIC("Cannot re-define class of name: %s", name);

    int typeID = typeKeys++;
    namedObjects[name] = typeID;
    objectMapping[typeID] = classlist;
    ancestryMap[typeID] = parent.typeKey;
    classNames[typeID] = name;
    return typeID;
}

Boolean isTypeName(const char* str) {
    return (Boolean)(namedObjects.find(str) != namedObjects.end());
}

CheshireType getNamedType(const char* str, Boolean isUnsafe) {
    CheshireType ret = {0, FALSE, 0};
    ERROR_IF(!isTypeName(str), "No such named type as %s", str);
    ret.typeKey = namedObjects[str];

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
    //if (t.arrayNesting != 0)
    //    return FALSE;
    if (t.typeKey == -2) //if null
        return TRUE;

    return (Boolean)(objectMapping.find(t.typeKey) != objectMapping.end());
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
        else {
            printf("%s%s", classNames[t], node.isUnsafe ? "^" : "");
        }
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
    int superID = super.typeKey;
    int subID = sub.typeKey;

    if (superID == subID)
        return TRUE;

    while (subID != TYPE_OBJECT.typeKey) {
        subID = ancestryMap[subID];

        if (superID == subID)
            return TRUE;
    }

    return FALSE;
}
