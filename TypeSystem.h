#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "Structures.h"
#include "CheshireScope.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_NULL       {-2, FALSE, 0}
#define TYPE_VOID       {0, FALSE, 0}
#define TYPE_NUMBER     {1, FALSE, 0}
#define TYPE_INT        {2, FALSE, 0}
#define TYPE_DECIMAL    {3, FALSE, 0}
#define TYPE_BOOLEAN    {4, FALSE, 0}
#define TYPE_OBJECT     {5, FALSE, 0}
#define TYPE_OBJECT_HAT {5, TRUE, 0}
#define TYPE_STRING     {6, FALSE, 0}

    void initTypeSystem(void);
    void freeTypeSystem(void); //frees all of the char* references
    CheshireScope* allocateCheshireScope(void);
    void deleteCheshireScope(CheshireScope*);

    void raiseScope(CheshireScope*);
    void fallScope(CheshireScope*);
    void setExpectedMethodType(CheshireScope*, CheshireType);
    CheshireType getExpectedMethodType(CheshireScope*);
    CheshireType getVariableType(CheshireScope*, const char* name);
    void defineVariable(CheshireScope*, const char* name, CheshireType type);
    int defineClass(const char* name, ClassList*, CheshireType parent);

    Boolean isTypeName(const char*);
    CheshireType getLambdaType(CheshireType returnType, struct tagParameterList* parameters);
    CheshireType getNamedType(const char* name, Boolean isUnsafe);
    void printCheshireType(CheshireType);

    Boolean equalTypes(CheshireType left, CheshireType right);
    Boolean isVoid(CheshireType);
    Boolean isUnsafe(CheshireType);
    Boolean isBoolean(CheshireType);
    Boolean isInt(CheshireType);
    Boolean isObjectType(CheshireType);
    Boolean isLambdaType(CheshireType);
    Boolean isNumericalType(CheshireType);
    int getArrayNesting(CheshireType);
    CheshireType getArrayDereference(CheshireType);
    CheshireType getWidestNumericalType(CheshireType left, CheshireType right);

    Boolean isSuper(CheshireType parent, CheshireType child); //todo: classes

// TYPE CHECKING FUNCTIONS //

    void typeCheckTopNode(CheshireScope*, ParserTopNode*);
    CheshireType typeCheckExpressionNode(CheshireScope*, ExpressionNode*);
    void typeCheckStatementNode(CheshireScope*, StatementNode*);
    void typeCheckBlockList(CheshireScope*, BlockList*);

#ifdef __cplusplus
}
#endif

#endif /* TYPE_SYSTEM_H */
