/* 
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

#include <unordered_map>
#include <utility>
#include "LexerUtil.h"
#include "TypeSystem.h"

#define insertBaseType(type) { typeID = typeKeys++; saveStringLiteral(type, &string); TypeMap[string] = typeID; }

typedef std::pair<CheshireType, ParameterList*> LambdaType;

typedef std::unordered_map<const char*, TypeKey, CStrHash, CStrEql> TypeMap;
typedef std::unordered_map<LambdaType, TypeKey, LambdaHash, LambdaEql> LambdaMap;
typedef std::unordered_set<int> ValidObjectSet;

static Boolean isInitialized = FALSE;
static TypeMap typeMap;
static LambdaMap lambdaMap;
static ValidSupertypeSet validObjectSet;
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
        saveStringLiteral("Object", &string);
        typeMap[string] = typeID;
        validObjectSet.insert(typeID);
    }
    isInitialized = TRUE;
}

void freeTypeSystem() {
    isInitialized = FALSE;
    //todo: clear all of the sets.
}

Boolean isType(const char* str) {
    return (Boolean) typeMap.contains(str);
}

TypeKey getTypeKey(const char* str) {
    if (isType(str)) {
        return typeMap[str];
    } else {
        //otherwise, make it into a new type.
        char* string_copy;
        int typeID = typeKeys++;
        saveStringLiteral(str, &string_copy);
        typeMap[string] = typeID;
        validObjectSet.insert(typeID);
    }
}

TypeKey getLambdaTypeKey(CheshireType returnType, ParameterList* parameters) {
    
}

CheshireType getType(TypeKey base, Boolean isUnsafe) {
    CheshireType c;
    c.typeKey = base;
    if (!isValidObjectType(base) && isUnsafe) {
        //todo: panic, invalid type.
    }
    c.isUnsafe = isUnsafe;
    return c;
}

Boolean isValidObjectType(CheshireType t) {
    return (Boolean) validObjectSet.contains((int) t.typeKey);
}