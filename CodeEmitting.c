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

void emitCode(FILE* out, ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE:
            break;
        case PRT_METHOD_DECLARATION: {
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
            LLVMValue l;
            l.type = LVT_GLOBAL_VALUE;
            l.name = node->method.functionName;
            registerVariable(node->method.functionName, l);
            break;
        }
        case PRT_METHOD_DEFINITION: {
            PRINT("declare ");
            emitType(out, node->method.returnType);
            PRINT(" @%s(", node->method.functionName);
            ParameterList* p;

            for (p = node->method.params; p != NULL; p = p->next) {
                emitType(out, p->type);

                if (p->next != NULL)
                    PRINT(", ");
            }

            PRINT(") {\n");
            emitBlock(out, node->method.body);

            if (!isVoid(node->method.returnType))
                UNARY("ret", node->method.returnType, getIntegerLiteral(0)); //implicit, fallthrough return in non-void function.

            PRINT("}\n");
            LLVMValue l;
            l.type = LVT_GLOBAL_VALUE;
            l.name = node->method.functionName;
            registerVariable(node->method.functionName, l);
            break;
        }
        case PRT_VARIABLE_DECLARATION: {
            PRINT("@%s = external global ", node->variable.name);
            emitType(out, node->variable.type);
            PRINT("\n");
            LLVMValue l;
            l.type = LVT_GLOBAL_VALUE;
            l.name = node->variable.name;
            registerVariable(node->variable.name, l);
            break;
        }
        case PRT_VARIABLE_DEFINITION: {
            PRINT("@%s = common global ", node->variable.name);
            emitType(out, node->variable.type);
            PRINT(" ");
            emitValue(out, getIntegerLiteral(0));
            PRINT("\n");
            LLVMValue l;
            l.type = LVT_GLOBAL_VALUE;
            l.name = node->variable.name;
            registerVariable(node->variable.name, l);
            break;
        }
        case PRT_CLASS_DEFINITION: {
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
            registerVariable(statement->varDefinition.variable, l);
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
            PRINT(", label %%branch%d, label %%branch%d\n", labeltrue, labelfalse);
            PRINT("branch%d:\n", labeltrue);
            emitStatement(out, statement->conditional.block);
            PRINT("branch%d:\n", labelfalse);
        }
        break;
        case S_IF_ELSE: {
            LLVMValue branchfactor = emitExpression(out, statement->conditional.condition);
            int labeltrue = UNIQUE_IDENTIFIER, labelfalse = UNIQUE_IDENTIFIER, labelend = UNIQUE_IDENTIFIER;
            PRINT("    br i1 ");
            emitValue(out, branchfactor);
            PRINT(", label %%branch%d, label %%branch%d\n", labeltrue, labelfalse);
            PRINT("branch%d:\n", labeltrue);
            emitStatement(out, statement->conditional.block);
            PRINT("    br label %%branch%d\n", labelend);
            PRINT("branch%d:\n", labelfalse);
            emitStatement(out, statement->conditional.elseBlock);
            PRINT("branch%d:\n", labelend);
        }
        break;
        case S_WHILE: {
            int labelbegin = UNIQUE_IDENTIFIER, labeltrue = UNIQUE_IDENTIFIER, labelend = UNIQUE_IDENTIFIER;
            PRINT("branch%d:\n", labelbegin);
            LLVMValue branchfactor = emitExpression(out, statement->conditional.condition);
            PRINT("    br i1 ");
            emitValue(out, branchfactor);
            PRINT(", label %%branch%d, label %%branch%d\n", labeltrue, labelend);
            PRINT("branch%d:\n", labeltrue);
            emitStatement(out, statement->conditional.block);
            PRINT("branch%d:\n", labeltrue);
            PRINT("    br label %%branch%d", labelbegin);
            PRINT("branch%d:\n", labelend);
        }
        break;
        case S_RETURN: {
            LLVMValue l = emitExpression(out, statement->expression);
            PRINT("ret ");
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
        case LVT_GLOBAL_VALUE:
            PRINT("@%s", value.name);
            break;
        case LVT_LOCAL_VALUE:
            PRINT("%%value%lld", value.value);
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
            PRINT("%%branch%lld", value.value);
            break;
    }
}

LLVMValue emitExpression(FILE* out, ExpressionNode* node) {
    switch (node->type) {
        case OP_NOP:
            break;
        case OP_INTEGER: {
            LLVMValue l;
            l.type = LVT_INT_LITERAL;
            l.value = node->integer;
            return l;
        }
        break;
        case OP_DECIMAL: {
            LLVMValue l;
            l.type = LVT_DOUBLE_LITERAL;
            l.decimal = node->decimal;
            return l;
        }
        break;
        case OP_CHAR: {
            LLVMValue l;
            l.type = LVT_INT_LITERAL;
            l.value = node->character;
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
            //todo:
        }
        break;
        case OP_MINUSONE: {
            //todo:
        }
        break;
        case OP_EQUALS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
                BINARY_STORE(l, "fadd", node->determinedType, a, b);
            } else {
                BINARY_STORE(l, "add", node->determinedType, a, b);
            }
            return l;
            return l;
        }
        break;
        case OP_MINUS: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            LLVMValue l = getTemporaryStorage(UNIQUE_IDENTIFIER);
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
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
            if (isDecimal(node->binary.determinedType)) {
                BINARY_STORE(l, "frem", node->determinedType, a, b);
            } else {
                BINARY_STORE(l, "srem", node->determinedType, a, b);
            }
            return l;
        }
        break;
        case OP_SET: {
            LLVMValue a = emitExpression(out, node->binary.left), b = emitExpression(out, node->binary.right);
            PRINT("store ");
            emitType(node->binary.left.determinedType);
            PRINT(" ");
            emitValue(b);
            PRINT(", ");
            emitType(node->binary.left.determinedType);
            PRINT("* ");
            emitValue(a);
            PRINT("\n");
        }
        break;
        case OP_INSTANCEOF: {
        }
        break;
        case OP_VARIABLE: {
            return fetchVariable(node->string);
        }
        break;
        case OP_CAST: {
        }
        break;
        case OP_ACCESS: {
        }
        break;
        case OP_METHOD_CALL: {
        }
        break;
        case OP_RESERVED_LITERAL: {
        }
        break;
        case OP_ARRAY_ACCESS: {
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
    }

    PANIC("Fatal error in code-emitting!");
}

void emitType(FILE* out, CheshireType type) { //object types have implicit *, remember. Object* not Object
    int array = type.arrayNesting;
    type.arrayNesting = 0;

    if (isObjectType(type)) {
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
        //emitLambdaType(out, type); todo: emit lambdas
    } else if (isVoid(type)) {
        PRINT("void");
    } else
        PANIC("Unknown type!");

    for (; array < 0; array--)
        PRINT("*");
}