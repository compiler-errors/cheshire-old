/*
 * File:   TypeSystem.cpp
 * Author: Michael Goulet
 * Implements: TypeSystem.h
 */

#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include "Structures.h"
#include "TypeSystem.h"
#include "LexerUtilities.h"
//#include "CodeEmitting.h"

using std::max;

#define insertBaseType(type) { typeID = typeKeys++; namedObjects[type] = typeID; printf("Initializing type '%s' with key %d\n", type, typeID); }

//////////////// STATICS /////////////////
static AllocatedTypeStrings allocatedTypeStrings;
static NamedObjects namedObjects;
static LambdaTypes lambdaTypes;
ObjectMapping objectMapping;
AncestryMap ancestryMap;
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
        int typeID = 0;
        insertBaseType("void");   //type 0
        insertBaseType("I8");     //type 1 C-type
        insertBaseType("I16");    //type 2 C-type
        insertBaseType("Int");    //type 3
        insertBaseType("I64");    //type 4 C-type
        insertBaseType("Decimal");//type 5
        insertBaseType("Boolean");//type 6
        //object
        char* object_copy = saveIdentifierReturn("Object");
        allocatedTypeStrings.insert(object_copy);
        typeID = typeKeys++;
        namedObjects[object_copy] = typeID;
        printf("Initializing type '%s' with key %d\n", "Object", typeID);
        objectMapping[typeID] = NULL;
        ancestryMap[typeID] = typeID;
        classNames[typeID] = object_copy;
        //string
        char* string_copy = saveIdentifierReturn("String");
        allocatedTypeStrings.insert(string_copy);
        typeID = typeKeys++;
        namedObjects[string_copy] = typeID;
        printf("Initializing type '%s' with key %d\n", "String", typeID);
        objectMapping[typeID] = NULL;
        ancestryMap[typeID] = TYPE_OBJECT.typeKey;
        classNames[typeID] = string_copy;
    } else {
        PANIC("Double initialization of type system!");
    }

    isInitialized = TRUE;
}

void freeTypeSystem() {
    isInitialized = FALSE;

    for (AllocatedTypeStrings::iterator i = allocatedTypeStrings.begin(); i != allocatedTypeStrings.end(); ++i)
        free(*i);

    allocatedTypeStrings.clear();
    namedObjects.clear();
    lambdaTypes.clear();
    objectMapping.clear();
    keyedLambdas.clear();
    ancestryMap.clear();
    classNames.clear();
    //typeKeys = 0;
}

CheshireScope* allocateCheshireScope() {
    CheshireScope* scope = new CheshireScope;
    scope->highestScope = NULL;
    raiseTypeScope(scope);
    return scope;
}

void deleteCheshireScope(CheshireScope* scope) {
    fallTypeScope(scope);

    if (scope->highestScope != NULL)
        PANIC("Scope tracking has not exited from a higher scope!");

    delete scope;
}

void raiseTypeScope(CheshireScope* scope) {
    VariableScope* old = scope->highestScope;
    scope->highestScope = new VariableScope;
    scope->highestScope->parentScope = old;
}

void fallTypeScope(CheshireScope* scope) {
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

void reserveClassNameType(char* name) {
    allocatedTypeStrings.insert(name);

    if (isTypeName(name)) {
        ERROR_IF(!isObjectType(getNamedType(name)), "Cannot forward-declare non-object-types!");
        int typeID = getNamedType(name).typeKey;
        namedObjects.erase(namedObjects.find(classNames[typeID]));
        classNames[typeID] = name;
        namedObjects[name] = typeID;
        return;
    }

    int typeID = typeKeys++;
    namedObjects[name] = typeID;
    objectMapping[typeID] = NULL;
    classNames[typeID] = name;
}

int defineClass(char* name, ClassList* classlist, CheshireType parent) {
    reserveClassNameType(name);

    if (objectMapping[getNamedType(name).typeKey] != NULL)
        PANIC("Cannot re-define class of name: %s", name);

    int typeID = getNamedType(name).typeKey;
    namedObjects[name] = typeID;
    objectMapping[typeID] = classlist;
    ancestryMap[typeID] = parent.typeKey;
    classNames[typeID] = name;
    return typeID;
}

CheshireType getClassVariable(CheshireType type, const char* variable) {
    CStrEql streql;
    ERROR_IF(!isObjectType(type), "Cannot fetch object variable from non-object type.");

    if (!equalTypes(type, TYPE_OBJECT) && !streql(variable, "new")) {
        CheshireType supervar = getClassVariable( {ancestryMap[type.typeKey], 0}, variable);

        if (!equalTypes(supervar, TYPE_VOID))
            return supervar;
    }

    for (ClassList* p = objectMapping[type.typeKey]; p != NULL; p = p->next) {
        switch (p->type) {
            case CLT_CONSTRUCTOR:

                if (streql(variable, "new"))
                    return getLambdaType(TYPE_VOID, p->constructor.params);

                break;
            case CLT_VARIABLE:

                if (streql(p->variable.name, variable))
                    return p->variable.type;

                break;
            case CLT_METHOD:

                if (streql(p->method.name, variable))
                    return getLambdaType(p->method.returnType, p->method.params);

                break;
        }
    }

    return TYPE_VOID;
}

Boolean isTypeName(const char* str) {
    return (Boolean)(namedObjects.find(str) != namedObjects.end());
}

CheshireType getNamedType(const char* str) {
    CheshireType ret = TYPE_VOID;
    ERROR_IF(!isTypeName(str), "No such named type as %s", str);
    ret.typeKey = namedObjects[str];
    return ret;
}

char* getNamedTypeString(CheshireType type) {
    if (isObjectType(type) && type.arrayNesting == 0) {
        return saveIdentifierReturn(classNames[type.typeKey]);
    }

    PANIC("Invalid class name!");
}

CheshireType getLambdaType(CheshireType returnType, ParameterList* parameters) {
    LambdaType l = prepareLambdaType(returnType, parameters);

    if (lambdaTypes.find(l) != lambdaTypes.end()) {
        return lambdaTypes[l];
    } else {
        TypeKey typeID = typeKeys++;
        CheshireType t;
        t.typeKey = typeID;
        t.arrayNesting = 0;
        lambdaTypes[l] = t;
        keyedLambdas[t] = l;
        return t;
    }
}

Boolean equalTypes(CheshireType left, CheshireType right) {
    return (Boolean)(left.typeKey == right.typeKey && left.arrayNesting == right.arrayNesting);
}

Boolean isVoid(CheshireType t) {
    return (Boolean)(t.typeKey == TYPE_VOID.typeKey);
}

Boolean isNull(CheshireType t) {
    return (Boolean)(t.typeKey == TYPE_NULL.typeKey);
}

Boolean isBoolean(CheshireType t) {
    return (Boolean)(t.typeKey == TYPE_BOOLEAN.typeKey && t.arrayNesting == 0);
}

Boolean isInt(CheshireType t) {
    return (Boolean)(t.typeKey == TYPE_INT.typeKey && t.arrayNesting == 0);
}

Boolean isDecimal(CheshireType t) {
    return (Boolean)(t.typeKey == TYPE_DECIMAL.typeKey && t.arrayNesting == 0);
}

Boolean isNumericalType(CheshireType t) {
    return (Boolean)(t.typeKey >= TYPE_I8.typeKey && t.typeKey <= TYPE_DECIMAL.typeKey && t.arrayNesting == 0);
}

int getArrayNesting(CheshireType t) {
    return t.arrayNesting;
}

CheshireType getArrayDereference(CheshireType t) {
    t.arrayNesting--;
    return t;
}

CheshireType getWidestNumericalType(CheshireType left, CheshireType right) {
    return {max(left.typeKey, right.typeKey), 0};
}

Boolean isObjectType(CheshireType t) {
    //if (t.arrayNesting != 0)
    //    return FALSE;
    if (t.typeKey == TYPE_NULL.typeKey) //if null
        return TRUE;

    return (Boolean)(objectMapping.find(t.typeKey) != objectMapping.end());
}

Boolean isLambdaType(CheshireType t) {
    if (t.arrayNesting != 0)
        return FALSE;

    return (Boolean)(keyedLambdas.find(t) != keyedLambdas.end());
}

void printType(CheshireType node) {
    TypeKey t = node.typeKey;
    CheshireType raw = {node.typeKey, 0};

    if (isLambdaType(raw)) {
        LambdaType l = keyedLambdas[raw];
        printType(l.first);
        printf("::(");

        for (size_t i = 0; i < l.second.size(); i++) {
            if (i != 0)
                printf(", ");

            printType(l.second[i]);
        }

        printf(")");
    } else if (isObjectType(raw)) {
        if (raw.typeKey == TYPE_NULL.typeKey)
            printf("NULL_TYPE");
        else {
            printf("%s", classNames[t]);
        }
    } else {
        switch (t) {
            case 0:
                printf("void");
                break;
            case 1:
                printf("I8");
                break;
            case 2:
                printf("I16");
                break;
            case 3:
                printf("Int");
                break;
            case 4:
                printf("I64");
                break;
            case 5:
                printf("Decimal");
                break;
            case 6:
                printf("Boolean");
                break;
            default:
                PANIC("No such recognized type as: %d, arrayNesting = %d", t, node.arrayNesting);
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
