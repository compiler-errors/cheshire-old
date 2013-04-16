/* File:
 * Author: Michael Goulet
 * Implements:
 */

#include <stdlib.h>
#include "CodeEmitting.h"
#include "TypeSystem.h"
#include "ParserEnums.h"
#include "Structures.h"

#define PRINT(str, args...) fprintf(out, str , ##args)

#define UNARY(instruction, type, a) \
    { \
        PRINT("%s ", instruction); \
        emitType(out, type); \
        PRINT(" "); \
        emitValue(out, a); \
        PRINT("\n"); \
    }

#define BINARY(instruction, type, a, b) \
    { \
        PRINT("%s ", instruction); \
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

static inline LLVMValue getGlobalStorage(char* name) {
    LLVMValue l;
    l.type = LVT_GLOBAL_VARIABLE;
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

static inline LLVMValue getMethodExport(char* name) {
    LLVMValue l;
    l.type = LVT_METHOD_EXPORT;
    l.name = name;
    return l;
}

void emitCode(FILE* out, ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            break;
        case PRT_METHOD_DECLARATION: {
            LLVMValue exportedMethod = getMethodExport(node->method.functionName); //register before definition so it is usable.
            registerVariable(node->method.functionName, exportedMethod);LLVMValue l = getGlobalStorage(node->method.functionName);
            PRINT("@_Method_%s = global ", node->method.functionName);
            emitType(out, getLambdaType(node->method.returnType, node->method.params));
            PRINT(" ");
            emitValue(out, l);
            PRINT("\n");
            
            PRINT("declare ");
            emitType(out, node->method.returnType);
            PRINT(" @%s(", node->method.functionName);
            ParameterList* p;

            for (p = node->method.params; p != NULL; p = p->next) {
                emitType(out, p->type);

                if (p->next != NULL)
                    PRINT(", ");
            }

            PRINT(")\n");
            break;
        }
        case PRT_METHOD_DEFINITION: {
            LLVMValue exportedMethod = getMethodExport(node->method.functionName); //register before definition so it is usable.
            registerVariable(node->method.functionName, exportedMethod);LLVMValue l = getGlobalStorage(node->method.functionName);
            PRINT("@_Method_%s = global ", node->method.functionName);
            emitType(out, getLambdaType(node->method.returnType, node->method.params));
            PRINT(" ");
            emitValue(out, l);
            PRINT("\n");
            
            PRINT("define ");
            emitType(out, node->method.returnType);
            PRINT(" @%s(", node->method.functionName);
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

            if (!isVoid(node->method.returnType))
                UNARY("    ret", node->method.returnType, getIntegerLiteral(0)); //implicit, fallthrough return in non-void function.

            PRINT("}\n");
            break;
        }
        case PRT_VARIABLE_DECLARATION: {
            PRINT("@%s = external global ", node->variable.name);
            emitType(out, node->variable.type);
            PRINT("\n");
            LLVMValue l = getGlobalStorage(node->variable.name);
            break;
        }
        case PRT_VARIABLE_DEFINITION: {
            PRINT("@%s = common global ", node->variable.name);
            emitType(out, node->variable.type);
            PRINT(" ");
            emitValue(out, getIntegerLiteral(0));
            PRINT("\n");
            LLVMValue l = getGlobalStorage(node->variable.name);
            registerVariable(node->variable.name, l);
            break;
        }
        case PRT_CLASS_DEFINITION: {
            /*PRINT("class_%s = type {", node->classdef.name);
            Boolean predecessor = FALSE;
            for (ClassList* c = node->classdef.classlist; c != NULL; c = c->next) {
                if (!predecessor)
                    PRINT(", ");
                switch (c->type) {
                    case CLT_METHOD:
                        emitType(getLambdaType(c->method.returnType, c->method.params));
                        predecessor = TRUE;
                        break;
                    case CLT_VARIABLE:
                        emitType(node->variable.type);
                        predecessor = TRUE;
                        break;
                    default: break;
                }
            }
            PRINT("}");
            
            Boolean constructed = FALSE;
            for (ClassList* c = node->classdef.classlist; c != NULL; c = c->next) {
                switch (c->type) {
                    case CLT_METHOD:
                        //export method, include self reference -- wait NO I ALREADY ADDED!!!!
                        break;
                    case CLT_CONSTRUCTOR:
                        constructed = TRUE;
                        //construct, include self reference.
                        //super call, assign defaults, then block.
                        break;
                    default: break;
                }
            }
            
            if (!constructed) {
                //default constructor.
            }
            */
            break;
        }
    }
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
            PRINT(" = alloca "); //todo: move alloca out of inner loop!!
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
            //todo: assert. call __assert(bool) function?
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
            LLVMValue l = emitExpression(out, statement->expression);
            PRINT("    ret ");
            emitType(out, statement->expression->determinedType);
            PRINT(" ");
            emitValue(out, l);
            PRINT("\n");
        }
        break;
    }
}

void emitValue(FILE* out, LLVMValue value) {
    switch (value.type) {
        case LVT_GLOBAL_VARIABLE:
            PRINT("@%s", value.name);
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
            PRINT("%lf", value.decimal);
            break;
        case LVT_VOID:
            PRINT("void");
            break;
        case LVT_JUMPPOINT:
            PRINT("%%_Branch%lld", value.value);
            break;
        case LVT_METHOD_EXPORT:
            PRINT("@_Method_%s", value.name);
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
                BINARY_STORE(l, "fsub", node->determinedType, getIntegerLiteral(0), a);
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
            //todo: call GC function to find allocated type
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
            PRINT("call ");
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
                case RL_NULL:
                    return getIntegerLiteral(0);
            }
        }
        break;
        case OP_ARRAY_ACCESS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            
            PRINT("    ");
            emitValue(out, l);
            PRINT(" = getelementptr inbounds ");
            emitType(out, node->binary.left->determinedType);
            PRINT(" ");
            emitValue(out, a);
            PRINT(", i32 ");
            emitValue(out, b);
            PRINT("\n");
            return l;
        }
        break;
        case OP_STRING: {
            
        }
        break;
        case OP_CLOSURE: {
        }
        break;
        case OP_INSTANTIATION: {
        }
        break;
        case OP_OBJECT_CALL: {
        }
        break;
        case OP_ACCESS: {
        }
        break;
    }

    PANIC("Fatal error in code-emitting!");
}

void emitType(FILE* out, CheshireType type) { //object types have implicit *, remember. Object* not Object
    int array = type.arrayNesting;
    type.arrayNesting = 0;

    if (isNull(type)) {
        PRINT("i8*");
    } else if (isObjectType(type)) {
        char* name = getNamedTypeString(type);
        PRINT("class_%s*", name);
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

    for (; array > 0; array--)
        PRINT("*");
}