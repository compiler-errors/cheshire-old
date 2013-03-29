#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "Structures.h"
#include "CheshireScope.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_NULL       ((CheshireType) {-2, FALSE, 0})
#define TYPE_VOID       ((CheshireType) {0, FALSE, 0})
#define TYPE_NUMBER     ((CheshireType) {1, FALSE, 0})
#define TYPE_INT        ((CheshireType) {2, FALSE, 0})
#define TYPE_DECIMAL    ((CheshireType) {3, FALSE, 0})
#define TYPE_BOOLEAN    ((CheshireType) {4, FALSE, 0})
#define TYPE_OBJECT     ((CheshireType) {5, FALSE, 0})
#define TYPE_OBJECT_HAT ((CheshireType) {5, TRUE, 0})
#define TYPE_STRING     ((CheshireType) {6, FALSE, 0})

    void initTypeSystem(void);
    void freeTypeSystem(void); //frees all of the char* references
    CheshireScope* allocateCheshireScope(void);
    void deleteCheshireScope(CheshireScope*);

    void raiseScope(CheshireScope*);
    void fallScope(CheshireScope*);
    void setExpectedMethodType(CheshireType);
    CheshireType getExpectedMethodType(void);
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

    Boolean isSuper(CheshireType parent, CheshireType child);

// TYPE CHECKING FUNCTIONS //

    void defineTopNode(CheshireScope*, ParserTopNode*); //this adds the name to the global namespace, but doesn't typecheck.
    void typeCheckTopNode(CheshireScope*, ParserTopNode*); //this typechecks, assuming that the name is added to the global namespace.
    CheshireType typeCheckExpressionNode(CheshireScope*, ExpressionNode*);
    void typeCheckStatementNode(CheshireScope*, StatementNode*);
    void typeCheckBlockList(CheshireScope*, BlockList*);

#ifdef __cplusplus
}
#endif

#endif /* TYPE_SYSTEM_H */
