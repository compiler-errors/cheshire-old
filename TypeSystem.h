#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "Structures.h"
#include "CheshireScope.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_VOID       getType(0, FALSE)
#define TYPE_INT        getType(1, FALSE)
#define TYPE_NUMBER     getType(2, FALSE)
#define TYPE_DECIMAL    getType(3, FALSE)
#define TYPE_BOOLEAN    getType(4, FALSE)
#define TYPE_OBJECT     getType(5, FALSE)
#define TYPE_OBJECT_HAT getType(5, TRUE)
#define TYPE_STRING     getType(6, FALSE)
    
void initTypeSystem(void);
void freeTypeSystem(void); //frees all of the char* references
CheshireScope* allocateCheshireScope(void);
void deleteCheshireScope(CheshireScope*);

void raiseScope(CheshireScope*);
void fallScope(CheshireScope*);
TypeKey getMethodSignature(CheshireScope*, const char* name);
void addMethodDeclaration(CheshireScope*, const char* name, CheshireType returnType, struct tagParameterList*);
CheshireType getVariableType(CheshireScope*, const char* name);
void defineVariable(CheshireScope*, const char* name, CheshireType type);

Boolean isTypeName(const char*);
TypeKey getTypeKey(const char*); //creates a new type key if it doesn't exist.
TypeKey getLambdaTypeKey(CheshireType returnType, struct tagParameterList* parameters);
CheshireType getType(TypeKey base, Boolean isUnsafe);
void printCheshireType(CheshireType);

Boolean areEqualTypes(CheshireType left, CheshireType right);
Boolean isVoid(CheshireType);
Boolean isBoolean(CheshireType);
Boolean isInt(CheshireType);
Boolean isValidObjectType(CheshireType);
Boolean isValidLambdaType(CheshireType);
Boolean isNumericalType(CheshireType);
CheshireType getWidestNumericalType(CheshireType left, CheshireType right);

//CheshireType getSupertype(CheshireScope*, CheshireType); todo: classes

// TYPE CHECKING FUNCTIONS //

void typeCheckTopNode(CheshireScope*, ParserTopNode*);
void typeCheckParameterList(CheshireScope*, ParserTopNode*);
CheshireType typeCheckExpressionNode(CheshireScope*, ExpressionNode*);
void typeCheckStatementNode(CheshireScope*, StatementNode*);
void typeCheckBlockList(CheshireScope*, BlockList*);

#ifdef __cplusplus
}
#endif

#endif /* TYPE_SYSTEM_H */