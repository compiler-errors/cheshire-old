#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "Structures.h"
#include "CheshireScope.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_NULL       ((CheshireType) {-2, 0})
#define TYPE_VOID       ((CheshireType)  {0, 0})
#define TYPE_I8         ((CheshireType)  {1, 0})
#define TYPE_I16        ((CheshireType)  {2, 0})
#define TYPE_INT        ((CheshireType)  {3, 0})
#define TYPE_I64        ((CheshireType)  {4, 0})
#define TYPE_DECIMAL    ((CheshireType)  {5, 0})
#define TYPE_BOOLEAN    ((CheshireType)  {6, 0})
#define TYPE_OBJECT     ((CheshireType)  {7, 0})
#define TYPE_STRING     ((CheshireType)  {8, 0}) //todo: give the class an actual structure ClassList* w/ methods in initTypeSystem()

    void initTypeSystem(void);
    void freeTypeSystem(void); //frees all of the char* references
    CheshireScope* allocateCheshireScope(void);
    void deleteCheshireScope(CheshireScope*);

    void raiseTypeScope(CheshireScope*);
    void fallTypeScope(CheshireScope*);
    void setExpectedMethodType(CheshireType);
    CheshireType getExpectedMethodType(void);
    CheshireType getVariableType(CheshireScope*, const char* name);
    void defineVariable(CheshireScope*, const char* name, CheshireType type);
    CheshireType getClassVariable(CheshireType, const char* variable);
    void reserveClassNameType(char* name); //"defines" a class so it can use its own name in its definition.
    int defineClass(char* name, ClassList*, CheshireType parent);

    Boolean isTypeName(const char*);
    CheshireType getLambdaType(CheshireType returnType, struct tagParameterList* parameters);
    CheshireType getNamedType(const char* name);
    char* getNamedTypeString(CheshireType);
    void printType(CheshireType);

    Boolean equalTypes(CheshireType left, CheshireType right);
    Boolean isVoid(CheshireType);
    Boolean isBoolean(CheshireType);
    Boolean isInt(CheshireType);
    Boolean isDecimal(CheshireType);
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
