/* File:
 * Author: Michael Goulet
 * Implements:
 */

#include <stdlib.h>
#include <string.h>
#include "CodeEmitting.h"
#include "TypeSystem.h"
#include "ParserEnums.h"
#include "Structures.h"

#define PRINT(str, args...) fprintf(out, str , ##args)

#define UNARY(instruction, type, a) \
    { \
        PRINT("    %s ", instruction); \
        emitType(out, type); \
        PRINT(" "); \
        emitValue(out, a); \
        PRINT("\n"); \
    }

#define BINARY(instruction, type, a, b) \
    { \
        PRINT("    %s ", instruction); \
        emitType(out, type); \
        PRINT(" "); \
        emitValue(out, a); \
        PRINT(", "); \
        emitValue(out, b); \
        PRINT("\n"); \
    }

#define UNARY_STORE(storage, instruction, type, a) \
    { \
        PRINT("    "); \
        emitValue(out, storage); \
        PRINT(" = "); \
        PRINT("%s ", instruction); \
        emitType(out, type); \
        PRINT(" "); \
        emitValue(out, a); \
        PRINT("\n"); \
    }

#define BINARY_STORE(storage, instruction, type, a, b) \
    { \
        PRINT("    "); \
        emitValue(out, storage); \
        PRINT(" = "); \
        PRINT("%s ", instruction); \
        emitType(out, type); \
        PRINT(" "); \
        emitValue(out, a); \
        PRINT(", "); \
        emitValue(out, b); \
        PRINT("\n"); \
    }

#define UNIQUE_IDENTIFIER (unique_identifier++)

static int unique_identifier = 0;


static inline LLVMValue getIntegerLiteral(int literal) {
    LLVMValue l;
    l.type = LVT_INT_LITERAL;
    l.value = literal;
    return l;
}

static inline LLVMValue getDecimalLiteral(double literal) {
    LLVMValue l;
    l.type = LVT_DOUBLE_LITERAL;
    l.decimal = literal;
    return l;
}

static inline LLVMValue getTemporaryStorage(int identifier) {
    LLVMValue l;
    l.type = LVT_LOCAL_VALUE;
    l.value = identifier;
    return l;
}

static inline LLVMValue getClosureMethod(int identifier) {
    LLVMValue l;
    l.type = LVT_CLOSURE_METHOD;
    l.value = identifier;
    return l;
}

static inline LLVMValue getGlobalStorage(char* name) {
    LLVMValue l;
    l.type = LVT_GLOBAL_VARIABLE;
    l.name = name;
    return l;
}

static inline LLVMValue getGlobalMethodStorage(char* name) {
    LLVMValue l;
    l.type = LVT_GLOBAL_METHOD;
    l.name = name;
    return l;
}

static inline LLVMValue getParameterStorage(char* name) {
    LLVMValue l;
    l.type = LVT_PARAMETER_VARIABLE;
    l.name = name;
    return l;
}

static inline LLVMValue getLocalVariableStorage(char* name) {
    LLVMValue l;
    l.type = LVT_LOCAL_VARIABLE;
    l.vardef.name = name;
    l.vardef.uid = UNIQUE_IDENTIFIER; //unique identifier included to alleviate conflicting scope-differing instances of same variable name.
    return l;
}

static inline LLVMValue getClassMethodStorage(char* classname, char* methodname) {
    LLVMValue l;
    l.type = LVT_CLASS_METHOD;
    l.classmethod.classname = classname;
    l.classmethod.methodname = methodname;
    return l;
}

static inline LLVMValue emitSizeOfClass(FILE* out, CheshireType t) {
    LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
    LLVMValue intval = getTemporaryStorage(UNIQUE_IDENTIFIER);
    PRINT("    ");
    emitValue(out, l);
    PRINT(" = getelementptr ");
    emitType(out, t);
    PRINT(" null, i32 1\n");
    PRINT("    ");
    emitValue(out, intval);
    PRINT(" = ptrtoint ");
    emitType(out, t);
    PRINT(" ");
    emitValue(out, l);
    PRINT(" to i32\n");
    return intval;
}

static inline void emitNonTypecheckedUpcast(FILE* out, LLVMValue* parameterValue, CheshireType* parameterType, LLVMValue givenValue, CheshireType selfType, CheshireType superType) {
    if (equalTypes(superType, selfType)) {
        *parameterValue = givenValue;
        *parameterType = selfType;
    } else {
        LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
        PRINT("    ");
        emitValue(out, l);
        PRINT(" = bitcast ");
        emitType(out, selfType);
        PRINT(" ");
        emitValue(out, givenValue);
        PRINT(" to ");
        emitType(out, superType);
        PRINT("\n");
        *parameterValue = l;
        *parameterType = superType;
    }
}

static inline LLVMValue getMethodExport(char* name) {
    LLVMValue l;
    l.type = LVT_METHOD_EXPORT;
    l.name = name;
    return l;
}

void forwardDefinition(ParserTopNode* node) {
    switch (node->type) {
        case PRT_METHOD_DECLARATION:
        case PRT_METHOD_DEFINITION: {
            LLVMValue exportedMethod = getGlobalMethodStorage(node->method.functionName); //register before definition so it is usable.
            registerVariable(node->method.functionName, exportedMethod);
        } break;
        case PRT_VARIABLE_DEFINITION:
        case PRT_VARIABLE_DECLARATION: {
            LLVMValue l = getGlobalStorage(node->variable.name);
            registerVariable(node->variable.name, l);
        } break;
        default:
            break;
    }
}

void emitCode(FILE* out, ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            break;
        case PRT_METHOD_DECLARATION: {
            LLVMValue l = getMethodExport(node->method.functionName);
            PRINT("@_M_%s = external constant ", node->method.functionName);
            emitType(out, getLambdaType(node->method.returnType, node->method.params));
            PRINT(" ");
            emitValue(out, l);
            PRINT("\n\n");
            break;
        }
        case PRT_METHOD_DEFINITION: {
            LLVMValue l = getMethodExport(node->method.functionName);
            PRINT("@_M_%s = constant ", node->method.functionName);
            emitType(out, getLambdaType(node->method.returnType, node->method.params));
            PRINT(" ");
            emitValue(out, l);
            PRINT("\n\n");
            PRINT("define fastcc ");
            emitType(out, node->method.returnType);
            PRINT(" @_MethodImpl_%s(", node->method.functionName);
            ParameterList* p;

            for (p = node->method.params; p != NULL; p = p->next) {
                emitType(out, p->type);
                PRINT(" ");
                LLVMValue paramValue = getParameterStorage(p->name);
                emitValue(out, paramValue);

                if (p->next != NULL)
                    PRINT(", ");
            }

            PRINT(") {\n");
            raiseVariableScope();

            for (p = node->method.params; p != NULL; p = p->next) {
                LLVMValue l = getParameterStorage(p->name);
                PRINT("    ");
                LLVMValue variable = getLocalVariableStorage(p->name);
                emitValue(out, variable);
                PRINT(" = alloca ");
                emitType(out, p->type);
                PRINT("\n");
                PRINT("    store ");
                emitType(out, p->type);
                PRINT(" ");
                emitValue(out, l);
                PRINT(", ");
                emitType(out, p->type);
                PRINT("* ");
                emitValue(out, variable);
                PRINT("\n");
                registerVariable(p->name, variable);
            }

            emitBlock(out, node->method.body);
            fallVariableScope();

            if (!isVoid(node->method.returnType)) {
                UNARY("ret", node->method.returnType, isDecimal(node->method.returnType) ? getDecimalLiteral(0) : getIntegerLiteral(0)); //implicit, fallthrough return in non-void function.
            } else {
                PRINT("    ret void\n");
            }

            PRINT("}\n\n");
            break;
        }
        case PRT_VARIABLE_DECLARATION: {
            PRINT("@%s = external global ", node->variable.name);
            emitType(out, node->variable.type);
            PRINT("\n\n");
            break;
        }
        case PRT_VARIABLE_DEFINITION: {
            PRINT("@%s = common global ", node->variable.name);
            emitType(out, node->variable.type);
            PRINT(" ");
            if (isNumericalType(node->variable.type)) {
                emitValue(out, isDecimal(node->variable.type) ? getDecimalLiteral(0) : getIntegerLiteral(0));
            } else {
                LLVMValue nullValue;
                nullValue.type = LVT_NULL;
                emitValue(out, nullValue);
            }
            PRINT("\n\n");
            break;
        }
        case PRT_CLASS_DEFINITION: {
            ClassShape* c = getClassShape(getNamedType(node->classdef.name));
            PRINT("%%_Class_%s = type {", node->classdef.name);
            ClassShape* shapeNode;

            for (shapeNode = c; shapeNode != NULL; shapeNode = shapeNode->next) {
                emitType(out, shapeNode->type);

                if (shapeNode->next != NULL)
                    PRINT(", ");
            }

            PRINT("}\n\n");
            Boolean constructor = FALSE;
            ClassList* classnode;

            for (classnode = node->classdef.classlist; classnode != NULL; classnode = classnode->next) {
                switch (classnode->type) {
                    case CLT_CONSTRUCTOR: {
                        constructor = TRUE;
                        PRINT("define fastcc void @_New_%s(", node->classdef.name);
                        ParameterList* p;

                        for (p = classnode->constructor.params; p != NULL; p = p->next) {
                            emitType(out, p->type);
                            PRINT(" ");
                            LLVMValue paramValue = getParameterStorage(p->name);
                            emitValue(out, paramValue);

                            if (p->next != NULL)
                                PRINT(", ");
                        }

                        PRINT(") {\n");
                        raiseVariableScope();

                        for (p = classnode->constructor.params; p != NULL; p = p->next) {
                            LLVMValue l = getParameterStorage(p->name);
                            PRINT("    ");
                            LLVMValue variable = getLocalVariableStorage(p->name);
                            emitValue(out, variable);
                            PRINT(" = alloca ");
                            emitType(out, p->type);
                            PRINT("\n");
                            PRINT("    store ");
                            emitType(out, p->type);
                            PRINT(" ");
                            emitValue(out, l);
                            PRINT(", ");
                            emitType(out, p->type);
                            PRINT("* ");
                            emitValue(out, variable);
                            PRINT("\n");
                            registerVariable(p->name, variable);
                        }

                        int paramLength = 1;
                        ExpressionList* e;

                        for (e = classnode->constructor.inheritsParams; e != NULL; e = e->next)
                            paramLength++;

                        LLVMValue selfReference = fetchVariable("self");
                        LLVMValue deallocatedSelf = getTemporaryStorage(UNIQUE_IDENTIFIER);
                        PRINT("    ");
                        emitValue(out, deallocatedSelf);
                        PRINT(" = load ");
                        emitType(out, getNamedType(node->classdef.name));
                        PRINT("* ");
                        emitValue(out, selfReference);
                        PRINT("\n");
                        LLVMValue* parameters = malloc(sizeof(LLVMValue) * paramLength);
                        CheshireType* parameterTypes = malloc(sizeof(CheshireType) * paramLength);
                        emitNonTypecheckedUpcast(out, &(parameters[0]), &(parameterTypes[0]), deallocatedSelf, getNamedType(node->classdef.name), node->classdef.parent);
                        int i;

                        for (e = classnode->constructor.inheritsParams, i = 1; e != NULL; e = e->next, i++) {
                            parameters[i] = emitExpression(out, e->parameter);
                            parameterTypes[i] = e->parameter->determinedType;
                        }

                        char* superName = getNamedTypeString(node->classdef.parent);
                        PRINT("    call fastcc void @_New_%s(", superName);
                        free(superName);

                        for (i = 0; i < paramLength; i++) {
                            emitType(out, parameterTypes[i]);
                            PRINT(" ");
                            emitValue(out, parameters[i]);

                            if (i != paramLength - 1)
                                PRINT(", ");
                        }

                        free(parameters);
                        free(parameterTypes);
                        PRINT(")\n");
                        ClassList* subnode;

                        for (subnode = node->classdef.classlist; subnode != NULL; subnode = subnode->next) {
                            switch (subnode->type) {
                                case CLT_VARIABLE: {
                                    LLVMValue defaultValue = emitExpression(out, subnode->variable.defaultValue);
                                    LLVMValue var = getTemporaryStorage(UNIQUE_IDENTIFIER);
                                    PRINT("    ");
                                    emitValue(out, var);
                                    PRINT(" = getelementptr ");
                                    emitType(out, getNamedType(node->classdef.name));
                                    PRINT(" ");
                                    emitValue(out, deallocatedSelf);
                                    PRINT(", i32 0, i32 %d\n", getObjectElement(getNamedType(node->classdef.name), subnode->variable.name));
                                    PRINT("    store ");
                                    emitType(out, subnode->variable.defaultValue->determinedType);
                                    PRINT(" ");
                                    emitValue(out, defaultValue);
                                    PRINT(", ");
                                    emitType(out, subnode->variable.defaultValue->determinedType);
                                    PRINT("* ");
                                    emitValue(out, var);
                                    PRINT("\n");
                                }
                                break;
                                case CLT_METHOD: {
                                    LLVMValue defaultValue = getClassMethodStorage(node->classdef.name, subnode->method.name);
                                    LLVMValue classStorage = getTemporaryStorage(UNIQUE_IDENTIFIER);
                                    CheshireType type = getLambdaType(subnode->method.returnType, subnode->method.params);
                                    PRINT("    ");
                                    emitValue(out, classStorage);
                                    PRINT(" = getelementptr ");
                                    emitType(out, getNamedType(node->classdef.name));
                                    PRINT(" ");
                                    emitValue(out, deallocatedSelf);
                                    PRINT(", i32 0, i32 %d\n", getObjectElement(getNamedType(node->classdef.name), subnode->method.name));

                                    if (!equalTypes(TYPE_VOID, getClassVariable(node->classdef.parent, subnode->method.name))) { //if not an override
                                        emitNonTypecheckedUpcast(out, &defaultValue, &type, defaultValue, type, getClassVariable(node->classdef.parent, subnode->method.name));
                                    }

                                    PRINT("    store ");
                                    emitType(out, type);
                                    PRINT(" ");
                                    emitValue(out, defaultValue);
                                    PRINT(", ");
                                    emitType(out, type);
                                    PRINT("* ");
                                    emitValue(out, classStorage);
                                    PRINT("\n");
                                }
                                break;
                                case CLT_CONSTRUCTOR:
                                    break;
                            }
                        }

                        emitBlock(out, classnode->constructor.block);
                        fallVariableScope();
                        PRINT("    ret void\n");
                        PRINT("}\n\n");
                    }
                    break;
                    case CLT_METHOD: {
                        PRINT("define fastcc ");
                        emitType(out, classnode->method.returnType);
                        PRINT(" @_ClassMethod_%s_%s(", node->classdef.name, classnode->method.name);
                        ParameterList* p;

                        for (p = classnode->method.params; p != NULL; p = p->next) {
                            emitType(out, p->type);
                            PRINT(" ");
                            LLVMValue paramValue = getParameterStorage(p->name);
                            emitValue(out, paramValue);

                            if (p->next != NULL)
                                PRINT(", ");
                        }

                        PRINT(") {\n");
                        raiseVariableScope();

                        for (p = classnode->method.params; p != NULL; p = p->next) {
                            LLVMValue l = getParameterStorage(p->name);
                            PRINT("    ");
                            LLVMValue variable = getLocalVariableStorage(p->name);
                            emitValue(out, variable);
                            PRINT(" = alloca ");
                            emitType(out, p->type);
                            PRINT("\n");
                            PRINT("    store ");
                            emitType(out, p->type);
                            PRINT(" ");
                            emitValue(out, l);
                            PRINT(", ");
                            emitType(out, p->type);
                            PRINT("* ");
                            emitValue(out, variable);
                            PRINT("\n");
                            registerVariable(p->name, variable);
                        }

                        emitBlock(out, classnode->method.block);
                        fallVariableScope();

                        if (isVoid(classnode->method.returnType)) {
                            PRINT("    ret void");
                        } else if (!isVoid(classnode->method.returnType)) {
                            UNARY("ret", classnode->method.returnType, isDecimal(classnode->method.returnType) ? getDecimalLiteral(0) : getIntegerLiteral(0)); 
                            //implicit, fallthrough return in non-void function.
                        } else {
                            LLVMValue nullValue;
                            nullValue.type = LVT_NULL;
                            UNARY("ret", classnode->method.returnType, nullValue);
                        }

                        PRINT("}\n\n");
                    }
                    break;
                    case CLT_VARIABLE:
                        break;
                }
            }

            if (!constructor) {
                PRINT("define fastcc void @_New_%s(%%_Class_%s* %%_Param_self) {\n", node->classdef.name, node->classdef.name);
                raiseVariableScope();
                LLVMValue l = getParameterStorage("self");
                PRINT("    ");
                LLVMValue variable = getLocalVariableStorage("self");
                emitValue(out, variable);
                PRINT(" = alloca ");
                emitType(out, getNamedType(node->classdef.name));
                PRINT("\n");
                PRINT("    store ");
                emitType(out, getNamedType(node->classdef.name));
                PRINT(" ");
                emitValue(out, l);
                PRINT(", ");
                emitType(out, getNamedType(node->classdef.name));
                PRINT("* ");
                emitValue(out, variable);
                PRINT("\n");
                registerVariable("self", variable);
                char* superName = getNamedTypeString(node->classdef.parent);
                LLVMValue selfReference = fetchVariable("self");
                LLVMValue deallocatedSelf = getTemporaryStorage(UNIQUE_IDENTIFIER);
                PRINT("    ");
                emitValue(out, deallocatedSelf);
                PRINT(" = load ");
                emitType(out, getNamedType(node->classdef.name));
                PRINT("* ");
                emitValue(out, selfReference);
                PRINT("\n");
                LLVMValue superValue;
                CheshireType superType;
                emitNonTypecheckedUpcast(out, &superValue, &superType, deallocatedSelf, getNamedType(node->classdef.name), node->classdef.parent);
                PRINT("    call fastcc void @_New_%s(", superName);
                emitType(out, superType);
                PRINT(" ");
                emitValue(out, superValue);
                PRINT(")\n");
                free(superName);
                ClassList* subnode;

                for (subnode = node->classdef.classlist; subnode != NULL; subnode = subnode->next) {
                    switch (subnode->type) {
                        case CLT_VARIABLE: {
                            LLVMValue defaultValue = emitExpression(out, subnode->variable.defaultValue);
                            LLVMValue var = getTemporaryStorage(UNIQUE_IDENTIFIER);
                            PRINT("    ");
                            emitValue(out, var);
                            PRINT(" = getelementptr ");
                            emitType(out, getNamedType(node->classdef.name));
                            PRINT(" ");
                            emitValue(out, deallocatedSelf);
                            PRINT(", i32 0, i32 %d\n", getObjectElement(getNamedType(node->classdef.name), subnode->variable.name));
                            PRINT("    store ");
                            emitType(out, subnode->variable.defaultValue->determinedType);
                            PRINT(" ");
                            emitValue(out, defaultValue);
                            PRINT(", ");
                            emitType(out, subnode->variable.defaultValue->determinedType);
                            PRINT("* ");
                            emitValue(out, var);
                            PRINT("\n");
                        }
                        break;
                        case CLT_METHOD: {
                            LLVMValue defaultValue = getClassMethodStorage(node->classdef.name, subnode->method.name);
                            LLVMValue classStorage = getTemporaryStorage(UNIQUE_IDENTIFIER);
                            CheshireType type = getLambdaType(subnode->method.returnType, subnode->method.params);
                            PRINT("    ");
                            emitValue(out, classStorage);
                            PRINT(" = getelementptr ");
                            emitType(out, getNamedType(node->classdef.name));
                            PRINT(" ");
                            emitValue(out, deallocatedSelf);
                            PRINT(", i32 0, i32 %d\n", getObjectElement(getNamedType(node->classdef.name), subnode->method.name));

                            if (!equalTypes(TYPE_VOID, getClassVariable(node->classdef.parent, subnode->method.name))) { //if not an override
                                emitNonTypecheckedUpcast(out, &defaultValue, &type, defaultValue, type, getClassVariable(node->classdef.parent, subnode->method.name));
                            }

                            PRINT("    store ");
                            emitType(out, type);
                            PRINT(" ");
                            emitValue(out, defaultValue);
                            PRINT(", ");
                            emitType(out, type);
                            PRINT("* ");
                            emitValue(out, classStorage);
                            PRINT("\n");
                        }
                        break;
                        case CLT_CONSTRUCTOR:
                            break;
                    }
                }

                fallVariableScope();
                PRINT("    ret void\n");
                PRINT("}\n\n");
            }
        }
        break;
    }
    flushPreambles(out);
}

void emitBlock(FILE* out, BlockList* node) {
    raiseVariableScope();

    for (; node != NULL; node = node->next) {
        StatementNode* statement = node->statement;
        emitStatement(out, statement);
    }

    fallVariableScope();
}

void emitStatement(FILE* out, StatementNode* statement) {
    switch (statement->type) {
        case S_NOP:
            break;
        case S_VARIABLE_DEF:
        case S_INFER_DEF: {
            LLVMValue l = emitExpression(out, statement->varDefinition.value);
            PRINT("    ");
            LLVMValue variable = getLocalVariableStorage(statement->varDefinition.variable);
            emitValue(out, variable);
            PRINT(" = alloca ");
            emitType(out, statement->varDefinition.type);
            PRINT("\n");
            PRINT("    store ");
            emitType(out, statement->varDefinition.type);
            PRINT(" ");
            emitValue(out, l);
            PRINT(", ");
            emitType(out, statement->varDefinition.type);
            PRINT("* ");
            emitValue(out, variable);
            PRINT("\n");
            registerVariable(statement->varDefinition.variable, variable);
        }
        break;
        case S_EXPRESSION: {
            emitExpression(out, statement->expression);
        }
        break;
        case S_ASSERT: {
            LLVMValue assertion = emitExpression(out, statement->expression);
            PRINT("    call fastcc void _Assert(");
            emitValue(out, assertion);
            PRINT(")\n");
        } break;
        case S_BLOCK: {
            emitBlock(out, statement->block);
        }
        break;
        case S_IF: {
            LLVMValue branchfactor = emitExpression(out, statement->conditional.condition);
            int labeltrue = UNIQUE_IDENTIFIER, labelfalse = UNIQUE_IDENTIFIER;
            PRINT("    br i1 ");
            emitValue(out, branchfactor);
            PRINT(", label %%label%d, label %%label%d\n", labeltrue, labelfalse);
            PRINT("label%d:\n", labeltrue);
            raiseVariableScope();
            emitStatement(out, statement->conditional.block);
            fallVariableScope();
            PRINT("label%d:\n", labelfalse);
        }
        break;
        case S_IF_ELSE: {
            LLVMValue branchfactor = emitExpression(out, statement->conditional.condition);
            int labeltrue = UNIQUE_IDENTIFIER, labelfalse = UNIQUE_IDENTIFIER, labelend = UNIQUE_IDENTIFIER;
            PRINT("    br i1 ");
            emitValue(out, branchfactor);
            PRINT(", label %%label%d, label %%label%d\n", labeltrue, labelfalse);
            PRINT("label%d:\n", labeltrue);
            raiseVariableScope();
            emitStatement(out, statement->conditional.block);
            fallVariableScope();
            PRINT("    br label %%label%d\n", labelend);
            PRINT("label%d:\n", labelfalse);
            raiseVariableScope();
            emitStatement(out, statement->conditional.elseBlock);
            fallVariableScope();
            PRINT("    br label %%label%d\n", labelend);
            PRINT("label%d:\n", labelend);
        }
        break;
        case S_WHILE: {
            int labelbegin = UNIQUE_IDENTIFIER, labeltrue = UNIQUE_IDENTIFIER, labelend = UNIQUE_IDENTIFIER;
            PRINT("    br label %%label%d\n", labelbegin);
            PRINT("label%d:\n", labelbegin);
            LLVMValue branchfactor = emitExpression(out, statement->conditional.condition);
            PRINT("    br i1 ");
            emitValue(out, branchfactor);
            PRINT(", label %%label%d, label %%label%d\n", labeltrue, labelend);
            PRINT("label%d:\n", labeltrue);
            raiseVariableScope();
            emitStatement(out, statement->conditional.block);
            fallVariableScope();
            PRINT("    br label %%label%d\n", labelbegin);
            PRINT("label%d:\n", labelend);
        }
        break;
        case S_RETURN: {
            if (isNull(statement->expression->determinedType)) {
                PRINT("    ret void\n");
            } else {
                LLVMValue l = emitExpression(out, statement->expression);
                PRINT("    ret ");
                emitType(out, statement->expression->determinedType);
                PRINT(" ");
                emitValue(out, l);
                PRINT("\n");
            }
        }
        break;
    }
}

LLVMValue emitExpression(FILE* out, ExpressionNode* node) {
    switch (node->type) {
        case OP_NOP:
            break;
        case OP_INTEGER: {
            LLVMValue l = getIntegerLiteral(node->integer);
            return l;
        }
        break;
        case OP_DECIMAL: {
            LLVMValue l = getDecimalLiteral(node->decimal);
            return l;
        }
        break;
        case OP_CHAR: {
            LLVMValue l = getIntegerLiteral(node->character);
            return l;
        }
        break;
        case OP_DEREFERENCE: {
            LLVMValue child = emitExpression(out, node->unaryChild);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, l);
            PRINT(" = load ");
            emitType(out, node->determinedType);
            PRINT("* ");
            emitValue(out, child);
            PRINT("\n");
            return l;
        }
        break;
        case OP_NOT:
        case OP_COMPL: {
            LLVMValue a = emitExpression(out, node->unaryChild);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            UNARY_STORE(l, "not", node->determinedType, a);
            return l;
        }
        break;
        case OP_UNARY_MINUS: {
            LLVMValue a = emitExpression(out, node->unaryChild);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->determinedType)) {
                BINARY_STORE(l, "fsub", node->determinedType, getDecimalLiteral(0), a);
            } else {
                BINARY_STORE(l, "sub", node->determinedType, getIntegerLiteral(0), a);
            }

            return l;
        }
        break;
        case OP_PLUSONE: {
            LLVMValue lval = emitExpression(out, node->unaryChild);
            LLVMValue deref = getTemporaryStorage(UNIQUE_IDENTIFIER), plusone = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, deref);
            PRINT(" = load ");
            emitType(out, node->determinedType);
            PRINT("* ");
            emitValue(out, lval);
            PRINT("\n");

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(plusone, "fadd", node->determinedType, deref, getDecimalLiteral(1));
            } else {
                BINARY_STORE(plusone, "add", node->determinedType, deref, getIntegerLiteral(1));
            }

            PRINT("    store ");
            emitType(out, node->binary.left->determinedType);
            PRINT(" ");
            emitValue(out, plusone);
            PRINT(", ");
            emitType(out, node->binary.left->determinedType);
            PRINT("* ");
            emitValue(out, lval);
            PRINT("\n");
            return deref;
        }
        break;
        case OP_MINUSONE: {
            LLVMValue lval = emitExpression(out, node->unaryChild);
            LLVMValue deref = getTemporaryStorage(UNIQUE_IDENTIFIER), plusone = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, deref);
            PRINT(" = load ");
            emitType(out, node->determinedType);
            PRINT("* ");
            emitValue(out, lval);
            PRINT("\n");

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(plusone, "fsub", node->determinedType, deref, getDecimalLiteral(1));
            } else {
                BINARY_STORE(plusone, "sub", node->determinedType, deref, getIntegerLiteral(1));
            }

            PRINT("    store ");
            emitType(out, node->binary.left->determinedType);
            PRINT(" ");
            emitValue(out, plusone);
            PRINT(", ");
            emitType(out, node->binary.left->determinedType);
            PRINT("* ");
            emitValue(out, lval);
            PRINT("\n");
            return deref;
        }
        break;
        case OP_EQUALS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fcmp eq", node->binary.left->determinedType, a, b);
            } else {
                BINARY_STORE(l, "icmp eq", node->binary.left->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_NOT_EQUALS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fcmp ne", node->binary.left->determinedType, a, b);
            } else {
                BINARY_STORE(l, "icmp ne", node->binary.left->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_GRE_EQUALS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fcmp sge", node->binary.left->determinedType, a, b);
            } else {
                BINARY_STORE(l, "icmp sge", node->binary.left->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_LES_EQUALS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fcmp sle", node->binary.left->determinedType, a, b);
            } else {
                BINARY_STORE(l, "icmp sle", node->binary.left->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_GREATER: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fcmp sgt", node->binary.left->determinedType, a, b);
            } else {
                BINARY_STORE(l, "icmp sgt", node->binary.left->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_LESS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fcmp slt", node->binary.left->determinedType, a, b);
            } else {
                BINARY_STORE(l, "icmp slt", node->binary.left->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_AND: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            BINARY_STORE(l, "and", node->determinedType, a, b);
            return l;
        }
        break;
        case OP_OR: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            BINARY_STORE(l, "or", node->determinedType, a, b);
            return l;
        }
        break;
        case OP_PLUS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fadd", node->determinedType, a, b);
            } else {
                BINARY_STORE(l, "add", node->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_MINUS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fsub", node->determinedType, a, b);
            } else {
                BINARY_STORE(l, "sub", node->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_MULT: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fmul", node->determinedType, a, b);
            } else {
                BINARY_STORE(l, "mul", node->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_DIV: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "fdiv", node->determinedType, a, b);
            } else {
                BINARY_STORE(l, "sdiv", node->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_MOD: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);

            if (isDecimal(node->binary.left->determinedType)) {
                BINARY_STORE(l, "frem", node->determinedType, a, b);
            } else {
                BINARY_STORE(l, "srem", node->determinedType, a, b);
            }

            return l;
        }
        break;
        case OP_SET: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            PRINT("    store ");
            emitType(out, node->binary.left->determinedType);
            PRINT(" ");
            emitValue(out, b);
            PRINT(", ");
            emitType(out, node->binary.left->determinedType);
            PRINT("* ");
            emitValue(out, a);
            PRINT("\n");
            return b;
        }
        break;
        case OP_INSTANCEOF: {
            //todo: call fastcc GC function to find allocated type
        }
        break;
        case OP_VARIABLE: {
            return fetchVariable(node->string);
        }
        break;
        case OP_CAST: {
            LLVMValue child = emitExpression(out, node->cast.child);

            if (isNumericalType(node->cast.type)) {
                if (equalTypes(node->cast.type, node->cast.child->determinedType)) {
                    //no cast
                    return child;
                } else if (node->cast.type.typeKey > node->cast.child->determinedType.typeKey) {
                    LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
                    PRINT("    ");
                    emitValue(out, l);

                    if (isDecimal(node->cast.type)) {
                        PRINT(" = sitofp ");
                    } else {
                        PRINT(" = sext ");
                    }

                    emitType(out, node->cast.child->determinedType);
                    PRINT(" ");
                    emitValue(out, child);
                    PRINT(" to ");
                    emitType(out, node->cast.type);
                    PRINT("\n");
                    return l;
                } else {
                    LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
                    PRINT("    ");
                    emitValue(out, l);

                    if (isDecimal(node->cast.child->determinedType)) {
                        PRINT(" = fptosi ");
                    } else {
                        PRINT(" = trunc ");
                    }

                    emitType(out, node->cast.child->determinedType);
                    PRINT(" ");
                    emitValue(out, child);
                    PRINT(" to ");
                    emitType(out, node->cast.type);
                    PRINT("\n");
                    return l;
                }
            } else {
                LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
                PRINT("    ");
                emitValue(out, l);
                PRINT(" = bitcast ");
                emitType(out, node->cast.child->determinedType);
                PRINT(" ");
                emitValue(out, child);
                PRINT(" to ");
                emitType(out, node->cast.type);
                PRINT("\n");
                return l;
            }
        }
        break;
        case OP_METHOD_CALL: {
            LLVMValue l;
            int paramLength = 0;
            ExpressionList* e;
            int i;

            for (e = node->methodcall.params; e != NULL; e = e->next)
                paramLength++;

            LLVMValue fnptr = emitExpression(out, node->methodcall.callback);
            LLVMValue* parameters = malloc(sizeof(LLVMValue) * paramLength);
            CheshireType* parameterTypes = malloc(sizeof(CheshireType) * paramLength);

            for (e = node->methodcall.params, i = 0; e != NULL; e = e->next, i++) {
                parameters[i] = emitExpression(out, e->parameter);
                parameterTypes[i] = e->parameter->determinedType;
            }

            if (isVoid(node->determinedType)) {
                l.type = LVT_VOID;
                PRINT("    ");
            } else {
                l = getTemporaryStorage(UNIQUE_IDENTIFIER);
                PRINT("    ");
                emitValue(out, l);
                PRINT(" = ");
            }

            PRINT("call fastcc ");
            emitType(out, node->determinedType);
            PRINT(" ");
            emitValue(out, fnptr);
            PRINT("(");

            for (i = 0; i < paramLength; i++) {
                emitType(out, parameterTypes[i]);
                PRINT(" ");
                emitValue(out, parameters[i]);

                if (i != paramLength - 1)
                    PRINT(", ");
            }

            free(parameters);
            free(parameterTypes);
            PRINT(")\n");
            return l;
        }
        break;
        case OP_RESERVED_LITERAL: {
            switch (node->reserved) {
                case RL_TRUE:
                    return getIntegerLiteral(1);
                case RL_FALSE:
                    return getIntegerLiteral(0);
                case RL_NULL: {
                    LLVMValue l;
                    l.type = LVT_NULL;
                    return l;
                }
            }
        }
        break;
        case OP_ARRAY_ACCESS: {
            //todo: add len operator!
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            //todo: runtime array sanity check
            PRINT("    ");
            emitValue(out, l);
            PRINT(" = getelementptr ");
            emitType(out, node->binary.left->determinedType);
            PRINT(" ");
            emitValue(out, a);
            PRINT(", i32 0, i32 1, i32 ");
            emitValue(out, b);
            PRINT("\n");
            return l;
        }
        break;
        case OP_STRING: {
            FILE* oldout = out;
            out = newPreamble();
            int tempident = UNIQUE_IDENTIFIER;
            int stringlength = strlen(node->string) + 1;
            PRINT("@.tempstring%d = private unnamed_addr constant [%d x i8] c\"%s\\00\", align 1\n", tempident, stringlength, node->string);
            out = oldout;
            LLVMValue temp = getTemporaryStorage(UNIQUE_IDENTIFIER), temp2 = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, temp);
            PRINT(" = getelementptr inbounds [%d x i8]* @.tempstring%d, i32 0, i32 0\n", stringlength, tempident);
            PRINT("    ");
            emitValue(out, temp2);
            PRINT(" = bitcast i8* ");
            emitValue(out, temp);
            PRINT(" to [0 x i8]*\n");
            LLVMValue constructed = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, constructed);
            PRINT(" = call %%_Class_String* @_New_String([0 x i8]* ");
            emitValue(out, temp2);
            PRINT(", i32 %d)\n", stringlength);
            return constructed;
        }
        break;
        case OP_CLOSURE: {
            int closure_id = UNIQUE_IDENTIFIER;
            if (node->closure.usingList == NULL) { //basically just a function...
                FILE* oldout = out;
                out = newPreamble();
                PRINT("define fastcc ");
                emitType(out, node->closure.type);
                PRINT(" @_Closure_%d(", closure_id);
                ParameterList* p;

                for (p = node->closure.params; p != NULL; p = p->next) {
                    emitType(out, p->type);
                    PRINT(" ");
                    LLVMValue paramValue = getParameterStorage(p->name);
                    emitValue(out, paramValue);

                    if (p->next != NULL)
                        PRINT(", ");
                }
                PRINT(") {\n");
                raiseVariableScope();

                for (p = node->closure.params; p != NULL; p = p->next) {
                    LLVMValue l = getParameterStorage(p->name);
                    PRINT("    ");
                    LLVMValue variable = getLocalVariableStorage(p->name);
                    emitValue(out, variable);
                    PRINT(" = alloca ");
                    emitType(out, p->type);
                    PRINT("\n");
                    PRINT("    store ");
                    emitType(out, p->type);
                    PRINT(" ");
                    emitValue(out, l);
                    PRINT(", ");
                    emitType(out, p->type);
                    PRINT("* ");
                    emitValue(out, variable);
                    PRINT("\n");
                    registerVariable(p->name, variable);
                }

                emitBlock(out, node->closure.body);
                fallVariableScope();
                
                if (!isVoid(node->closure.type)) {
                    UNARY("ret", node->closure.type, isDecimal(node->closure.type) ? getDecimalLiteral(0) : getIntegerLiteral(0)); //implicit, fallthrough return in non-void function.
                } else {
                    PRINT("    ret void\n");
                }
                PRINT("}\n");
                out = oldout;
                
                return getClosureMethod(closure_id);
            } else {
                //remember to raise var scope.
            }
        }
        break;
        case OP_INSTANTIATION: {
            LLVMValue size = emitSizeOfClass(out, node->instantiate.type);
            LLVMValue mallocated = getTemporaryStorage(UNIQUE_IDENTIFIER);
            LLVMValue casted = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, mallocated);
            PRINT(" = call fastcc i8* @malloc(i32 ");
            emitValue(out, size);
            PRINT(")\n");
            PRINT("    ");
            emitValue(out, casted);
            PRINT(" = bitcast i8* ");
            emitValue(out, mallocated);
            PRINT(" to ");
            emitType(out, node->instantiate.type);
            PRINT("\n");
            int paramLength = 1;
            ExpressionList* e;

            for (e = node->instantiate.params; e != NULL; e = e->next)
                paramLength++;

            LLVMValue* parameters = malloc(sizeof(LLVMValue) * paramLength);
            CheshireType* parameterTypes = malloc(sizeof(CheshireType) * paramLength);
            parameters[0] = casted;
            parameterTypes[0] = node->instantiate.type;
            int i;

            for (e = node->instantiate.params, i = 1; e != NULL; e = e->next, i++) {
                parameters[i] = emitExpression(out, e->parameter);
                parameterTypes[i] = e->parameter->determinedType;
            }

            char* name = getNamedTypeString(node->instantiate.type);
            PRINT("    call fastcc void @_New_%s(", name);
            free(name);

            for (i = 0; i < paramLength; i++) {
                emitType(out, parameterTypes[i]);
                PRINT(" ");
                emitValue(out, parameters[i]);

                if (i != paramLength - 1)
                    PRINT(", ");
            }

            free(parameters);
            free(parameterTypes);
            PRINT(")\n");
            return casted;
        }
        break;
        case OP_OBJECT_CALL: {
            LLVMValue l;
            LLVMValue object = emitExpression(out, node->objectcall.object);
            int i;
            // -- DEALLOCATING FUNCTION POINTER FROM OBJECT -- //
            LLVMValue fnptr_ptr = getTemporaryStorage(UNIQUE_IDENTIFIER);
            LLVMValue fnptr = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, fnptr_ptr);
            PRINT(" = getelementptr ");
            emitType(out, node->objectcall.object->determinedType);
            PRINT(" ");
            emitValue(out, object);
            PRINT(", i32 0, i32 %d\n", getObjectElement(node->objectcall.object->determinedType, node->objectcall.method));
            PRINT("    ");
            emitValue(out, fnptr);
            PRINT(" = load ");
            emitType(out, getClassVariable(node->objectcall.object->determinedType, node->objectcall.method));
            PRINT("* ");
            emitValue(out, fnptr_ptr);
            PRINT("\n");
            int paramLength = 1;
            ExpressionList* e;

            for (e = node->objectcall.params; e != NULL; e = e->next)
                paramLength++;

            LLVMValue* parameters = malloc(sizeof(LLVMValue) * paramLength);
            CheshireType* parameterTypes = malloc(sizeof(CheshireType) * paramLength);
            emitNonTypecheckedUpcast(out, &(parameters[0]), &(parameterTypes[0]), object, node->objectcall.object->determinedType, getObjectSelfType(node->objectcall.object->determinedType, node->objectcall.method));

            for (e = node->objectcall.params, i = 1; e != NULL; e = e->next, i++) {
                parameters[i] = emitExpression(out, e->parameter);
                parameterTypes[i] = e->parameter->determinedType;
            }

            if (isVoid(node->determinedType)) {
                l.type = LVT_VOID;
                PRINT("    ");
            } else {
                l = getTemporaryStorage(UNIQUE_IDENTIFIER);
                PRINT("    ");
                emitValue(out, l);
                PRINT(" = ");
            }

            PRINT("call fastcc ");
            emitType(out, node->determinedType);
            PRINT(" ");
            emitValue(out, fnptr);
            PRINT("(");

            for (i = 0; i < paramLength; i++) {
                emitType(out, parameterTypes[i]);
                PRINT(" ");
                emitValue(out, parameters[i]);

                if (i != paramLength - 1)
                    PRINT(", ");
            }

            free(parameters);
            free(parameterTypes);
            PRINT(")\n");
            return l;
        }
        break;
        case OP_ACCESS: {
            // -- DEALLOCATING FUNCTION POINTER FROM OBJECT -- //
            LLVMValue object = emitExpression(out, node->access.expression);
            LLVMValue var = getTemporaryStorage(UNIQUE_IDENTIFIER);
            PRINT("    ");
            emitValue(out, var);
            PRINT(" = getelementptr ");
            emitType(out, node->access.expression->determinedType);
            PRINT(" ");
            emitValue(out, object);
            PRINT(", i32 0, i32 %d\n", getObjectElement(node->access.expression->determinedType, node->access.variable));
            return var;
        }
        break;
    }

    PANIC("Fatal error in code-emitting!");
}

void emitType(FILE* out, CheshireType type) { //object types have implicit *, remember. Object* not Object
    if (type.arrayNesting > 0) {
        type.arrayNesting--;
        PRINT("{i32, [0 x ");
        emitType(out, type); //emit with one less nesting.
        PRINT("]}*");
        return;
    }

    if (isNull(type)) {
        PRINT("i8*");
    } else if (isObjectType(type)) {
        char* name = getNamedTypeString(type);
        PRINT("%%_Class_%s*", name);
        free(name);
    } else if (isNumericalType(type) || isBoolean(type)) {
        switch (type.typeKey) {
            case 1:
                PRINT("i8");
                break;
            case 2:
                PRINT("i16");
                break;
            case 3:
                PRINT("i32");
                break;
            case 4:
                PRINT("i64");
                break;
            case 5:
                PRINT("double");
                break;
            case 6:
                PRINT("i1");
                break;
        }
    } else if (isLambdaType(type)) {
        emitLambdaType(out, type);
    } else if (isVoid(type)) {
        PRINT("void");
    } else
        PANIC("Unknown type!");
}

void emitValue(FILE* out, LLVMValue value) {
    switch (value.type) {
        case LVT_GLOBAL_VARIABLE:
            PRINT("@%s", value.name);
            break;
        case LVT_GLOBAL_METHOD:
            PRINT("@_M_%s", value.name);
            break;
        case LVT_CLOSURE_METHOD:
            PRINT("@_Closure_%lld", value.value);
            break;
        case LVT_LOCAL_VARIABLE:
            PRINT("%%%s%d", value.vardef.name, value.vardef.uid);
            break;
        case LVT_PARAMETER_VARIABLE:
            PRINT("%%_Param_%s", value.name);
            break;
        case LVT_LOCAL_VALUE:
            PRINT("%%_Value%lld", value.value);
            break;
        case LVT_INT_LITERAL:
            PRINT("%lld", value.value);
            break;
        case LVT_DOUBLE_LITERAL:
            PRINT("%.6le", value.decimal);
            break;
        case LVT_VOID:
            PRINT("void");
            break;
        case LVT_JUMPPOINT:
            PRINT("%%_Branch_%lld", value.value);
            break;
        case LVT_METHOD_EXPORT:
            PRINT("@_Method_%s", value.name);
            break;
        case LVT_CLASS_METHOD:
            PRINT("@_ClassMethod_%s_%s", value.classmethod.classname, value.classmethod.methodname);
            break;
        case LVT_NULL:
            PRINT("null");
            break;
    }
}
