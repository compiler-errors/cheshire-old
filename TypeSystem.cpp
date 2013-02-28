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

typedef std::pair<CheshireType, ParameterList*> LambdaType;

//todo: replace std::unordered_map with custom map!

typedef std::unordered_map<const char*, TypeKey, CStrHash, CStrEql> TypeMap;
//typedef std::unordered_map<LambdaType, TypeKey, LambdaHash, LambdaEql> LambdaMap;
typedef std::unordered_set<int> ValidObjectSet;

static Boolean isInitialized = FALSE;
static TypeMap typeMap;
//static LambdaMap lambdaMap;
static ValidObjectSet validObjectSet;
static int typeKeys = 0;

void initTypeSystem() {
    if (!isInitialized) {
        char* string;
        int typeID = 0;
        insertBaseType("void");
        insertBaseType("Number");
        insertBaseType("Int");
        insertBaseType("Decimal");
        insertBaseType("Boolean");
        
        //object
        typeID = typeKeys++;
        saveIdentifier("Object", &string);
        typeMap[string] = typeID;
        printf("Initializing type '%s' with key %d\n", string, typeID);
        validObjectSet.insert(typeID);
    }
    isInitialized = TRUE;
}

void freeTypeSystem() {
    isInitialized = FALSE;
    for (auto iterator = typeMap.begin(); iterator != typeMap.end(); ++iterator)
        free((char*)(iterator->first));
    typeMap.clear();
    validObjectSet.clear();
    typeKeys = 0;
    //todo: lambda too.
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
        int typeID = typeKeys++;
        saveIdentifier(str, &string_copy);
        typeMap[string_copy] = typeID;
        validObjectSet.insert(typeID);
        return typeID;
    }
}

//TypeKey getLambdaTypeKey(CheshireType returnType, ParameterList* parameters) {
//    
//}

CheshireType getType(TypeKey base, Boolean isUnsafe) {
    CheshireType c;
    c.typeKey = base;
    if (isUnsafe && (validObjectSet.find((int) base) == validObjectSet.end())) {
        //todo: panic, invalid type.
    }
    c.isUnsafe = isUnsafe;
    return c;
}

Boolean isValidObjectType(CheshireType t) {
    return (Boolean) (validObjectSet.find((int) (t.typeKey)) == validObjectSet.end());
}
