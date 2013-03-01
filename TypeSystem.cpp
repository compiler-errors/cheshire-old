/* 
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "ParserNodes.h"
#include "LexerUtil.h"
#include "TypeSystemUtilities.hpp"

#define insertBaseType(type) { typeID = typeKeys++; saveIdentifier(type, &string); typeMap[string] = typeID; printf("Initializing type '%s' with key %d\n", string, typeID); }

//////////////// STATICS /////////////////
static Boolean isInitialized = FALSE;
static TypeMap typeMap;
static LambdaMap lambdaMap;
static ValidObjectSet validObjectSet;
static ValidLambdaSet validLambdaSet;
static TypeKey typeKeys = 0;

static LambdaType prepareLambdaType(CheshireType returnType, ParameterList* parameters) {
    LambdaType ret;
    ret.first = returnType;
    
    int count = 0;
    for (ParameterList* p = parameters; p != NULL; p = p->next, count++) {}
    ret.second = Array<CheshireType>(count);
    int i = 0;
    for (ParameterList* p = parameters; p != NULL; p = p->next, i++) {
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
    typeKeys = 0;
}

Boolean isType(const char* str) {
    return (Boolean) (typeMap.find(str) != typeMap.end());
}

TypeKey getTypeKey(const char* str) {
    if (isType(str)) {
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
        PANIC("Cannot apply unsafe-type to a non-object type!"); //using 'c' because it has the key, but no ^ applied yet.
    }
    c.isInfer = FALSE;
    c.isUnsafe = isUnsafe;
    return c;
}

Boolean isValidObjectType(CheshireType t) {
    return (Boolean) (validObjectSet.find(t.typeKey) != validObjectSet.end());
}

Boolean isValidLambdaType(CheshireType t) {
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
}