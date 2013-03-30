#include <cstdio>
#include <unordered_map>
#include <list>
#include "ParserEnums.h"
#include "Structures.h"
#include "TypeSystem.h"
#include "CodeEmitting.h"

using std::fprintf;
void emitCheshireType(FILE*, CheshireType);
void emitBlock(FILE*, BlockList*);
void emitStatement(FILE*, StatementNode*);
int emitExpression(FILE*, ExpressionNode*);

#define EMIT(format, args...) { fprintf(outfile, format , ##args); }
#define EMIT_NEWLN() { fprintf(outfile, "\n%s", tabbing); }

/////////////////////////////////////////
extern KeyedLambdas keyedLambdas;
static std::list<std::unordered_map<const char*, int, CStrHash, CStrEql> > scope;
static const char* tabbing = "";
static CheshireType expectedType = TYPE_VOID;
int unique_identifier = 0;
/////////////////////////////////////////

void emitCode(FILE* outfile, ParserTopNode* node) {
    switch (node->type) {
        case PRT_NONE: break;
        case PRT_METHOD_DECLARATION:
            EMIT("declare ");
            emitCheshireType(outfile, node->method.type);
            EMIT(" @%s(", node->method.functionName);
            for (ParameterList* n = node->method.params; n != NULL; n = n->next) {
                emitCheshireType(outfile, n->type);
                if (n->next != NULL)
                    EMIT(", ");
            }
            EMIT(")");
            EMIT_NEWLN();
            break;
        case PRT_METHOD_DEFINITION:
            EMIT("define ");
            emitCheshireType(outfile, node->method.type);
            EMIT(" @%s(", node->method.functionName);
            for (ParameterList* n = node->method.params; n != NULL; n = n->next) {
                emitCheshireType(outfile, n->type);
                if (n->next != NULL)
                    EMIT(", ");
            }
            EMIT(") {");
            tabbing = "\t";
            EMIT_NEWLN();
            expectedType = node->method.type;
            emitBlock(outfile, node->method.body);
            //todo: default return type for the the method, for ignored returns!
            tabbing = "";
            EMIT_NEWLN();
            EMIT("}");
            EMIT_NEWLN();
            break;
        case PRT_VARIABLE_DECLARATION:
            break;
        case PRT_VARIABLE_DEFINITION:
            break;
        case PRT_CLASS_DEFINITION:
            
            break;
    }
}

void emitCheshireType(FILE* outfile, CheshireType type) {
    int arrayNesting = type.arrayNesting;
    type.arrayNesting = 0;
    if (isNumericalType(type)) {
        switch (type.typeKey) {
            case 1: //Number
                EMIT("%%Number");
                break;
            case 2: //Int
                EMIT("i32");
                break;
            case 3: //Decimal
                EMIT("double");
                break;
        }
    } else if (isBoolean(type)) {
        EMIT("i1");
    } else if (isVoid(type)) {
        EMIT("void");
    } else if (isObjectType(type)) {
        EMIT("%%");
        type.isUnsafe = FALSE;
        type.arrayNesting = 0;
        printCheshireType(type);
        EMIT("*");
    } else if (isLambdaType(type)) {
        type.arrayNesting = 0;
        LambdaType method = keyedLambdas[type];
        emitCheshireType(outfile, method.first);
        EMIT("(");
        for (unsigned int i = 0; i < method.second.size(); i++) {
            if (i != 0)
                EMIT(", ");
            emitCheshireType(outfile, method.second[i]);
        }
        EMIT(")*");
    }
    for (int i = 0; i < arrayNesting; i++) 
        EMIT("*");
}

void emitBlock(FILE* outfile, BlockList* block) {
    scope.push_front(std::unordered_map<const char*, int, CStrHash, CStrEql>());
    if (block == NULL)
        return;
    
    emitStatement(outfile, block->statement);
    if (block->next != NULL) EMIT_NEWLN();
    emitBlock(outfile, block->next);
    scope.pop_front();
}

void emitStatement(FILE* outfile, StatementNode* statement) {
    switch (statement->type) {
        case S_NOP: break;
        case S_VARIABLE_DEF:
        case S_INFER_DEF:{
            int id = unique_identifier++;
            int child = emitExpression(outfile, statement->varDefinition.value);
            EMIT_NEWLN();
            EMIT("%%val%d = alloca ", id);
            emitCheshireType(outfile, statement->varDefinition.type);
            EMIT(", align 4; variable: %s", statement->varDefinition.variable); //todo: edit this?
            EMIT_NEWLN();
            EMIT("store ")
            emitCheshireType(outfile, statement->varDefinition.value->determinedType);
            EMIT(" %%val%d, ", child);
            emitCheshireType(outfile, statement->varDefinition.type);
            EMIT("* %%val%d", id);
            scope.front()[statement->varDefinition.variable] = id;
        }
        break;
        case S_EXPRESSION: {
            int ret = emitExpression(outfile, statement->expression);
        }
        break;
        case S_ASSERT: {
            
        }
        break;
        case S_BLOCK: {
            emitBlock(outfile, statement->block);
        }
        break;
        case S_IF: {
            int ifbranch = unique_identifier++, endlabel = unique_identifier++;
            int boolean = emitExpression(outfile, statement->conditional.condition);
            EMIT_NEWLN();
            EMIT("br i1 %%val%d, label %%branch%d, label %%branch%d", boolean, ifbranch, endlabel);
            EMIT("\nbranch%d:", ifbranch);
            EMIT_NEWLN();
            emitStatement(outfile, statement->conditional.block);
            EMIT_NEWLN();
            EMIT("br label %%branch%d", endlabel);
            EMIT("\nbranch%d:", endlabel);
        }
        break;
        case S_IF_ELSE: {
            int ifbranch = unique_identifier++, elsebranch = unique_identifier++, endlabel = unique_identifier++;
            int boolean = emitExpression(outfile, statement->conditional.condition);
            EMIT_NEWLN();
            EMIT("br i1 %%val%d, label %%branch%d, label %%branch%d", boolean, ifbranch, elsebranch);
            EMIT("\nbranch%d:", ifbranch);
            EMIT_NEWLN();
            emitStatement(outfile, statement->conditional.block);
            EMIT_NEWLN();
            EMIT("br label %%branch%d", endlabel);
            EMIT("\nbranch%d:", elsebranch);
            EMIT_NEWLN();
            emitStatement(outfile, statement->conditional.elseBlock);
            EMIT_NEWLN();
            EMIT("br label %%branch%d", endlabel);
            EMIT("\nbranch%d:", endlabel);
        }
        break;
        case S_WHILE: {
            int beginlabel, whilebranch = unique_identifier++, endlabel = unique_identifier++;
            int boolean = emitExpression(outfile, statement->conditional.condition);
            EMIT_NEWLN();
            EMIT("\n; <label>:%d", beginlabel);
            EMIT_NEWLN();
            EMIT("br i1 %d, label %%branch%d, label %%branch%d", boolean, whilebranch, endlabel);
            EMIT("\n; <label>:%d", whilebranch);
            EMIT_NEWLN();
            emitStatement(outfile, statement->conditional.block);
            EMIT_NEWLN();
            EMIT("br label %%branch%d", beginlabel);
            EMIT("\n; <label>:%d", endlabel);
        }
        break;
        case S_DELETE_HEAP: {
            
        }
        break;
        case S_RETURN: {
            if (equalTypes(statement->expression->determinedType, TYPE_NULL)) {
                EMIT("ret void");
            } else {
                int ret = emitExpression(outfile, statement->expression);
                EMIT_NEWLN();
                EMIT("ret ");
                emitCheshireType(outfile, statement->expression->determinedType);
                EMIT(" %%val%d", ret);
            }
        }
        break;
    }
}

int emitExpression(FILE* outfile, ExpressionNode* node) {
    //emit code %<val> = 
    switch (node->type) {
        case OP_NOP: break;
        break;
        case OP_LARGE_INTEGER: {
            //todo: numbers
            int id = unique_identifier++;
            EMIT("%%val%d = IMPLEMENT ME", id/*, node->integer*/);
            return id;
        }
        break;
        case OP_INTEGER: {
            int id = unique_identifier++;
            EMIT("%%val%d = add i32 %ld, 0 ; literal", id, node->integer);
            return id;
        }
        break;
        case OP_DECIMAL: {
            int id = unique_identifier++;
            EMIT("%%val%d = fadd double %lf, 0 ; literal", id, node->decimal);
            return id;
        }
        break;
        case OP_DEREFERENCE: {
            int id = unique_identifier++;
            int child = emitExpression(outfile, node->unaryChild);
            EMIT_NEWLN();
            EMIT("%%val%d = load ", id);
            emitCheshireType(outfile, node->determinedType);
            EMIT("* %%val%d", child);
            return id;
        }
        break;
        case OP_NOT:
        case OP_COMPL: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int child = emitExpression(outfile, node->unaryChild);
            EMIT_NEWLN();
            EMIT("%%val%d = xor ", id);
            emitCheshireType(outfile, node->unaryChild->determinedType);
            EMIT("%%val%d, -1", child);
            return id;
        }
        break;
        case OP_UNARY_MINUS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int child = emitExpression(outfile, node->unaryChild);
            EMIT_NEWLN();
            EMIT("%%val%d = mul ", id); // -x => x * -1
            emitCheshireType(outfile, node->unaryChild->determinedType);
            EMIT(" %%val%d, -1", child);
            return id;
        }
        break;
        case OP_PLUSONE: {
            //todo: add Number type support.
            int dereferenceid = unique_identifier++, addid = unique_identifier++;
            int child = emitExpression(outfile, node->unaryChild);
            EMIT_NEWLN();
            EMIT("%%val%d = load ", dereferenceid);
            emitCheshireType(outfile, node->determinedType);
            EMIT("* %%val%d", child);
            EMIT_NEWLN();
            EMIT("%%val%d = add ", addid);
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, 1", dereferenceid);
            EMIT_NEWLN();
            EMIT("store ")
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, ", addid);
            emitCheshireType(outfile, node->determinedType);
            EMIT("* %%val%d", child);
            return dereferenceid;
        }
        break;
        case OP_MINUSONE: {
            //todo: add Number type support.
            //todo: add Number type support.
            int dereferenceid = unique_identifier++, addid = unique_identifier++;
            int child = emitExpression(outfile, node->unaryChild);
            EMIT_NEWLN();
            EMIT("%%val%d = load ", dereferenceid);
            emitCheshireType(outfile, node->determinedType);
            EMIT("* %%val%d", child);
            EMIT_NEWLN();
            EMIT("%%val%d = add ", addid);
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, -1", dereferenceid);
            EMIT_NEWLN();
            EMIT("store ")
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, ", addid);
            emitCheshireType(outfile, node->determinedType);
            EMIT("* %%val%d", child);
            return dereferenceid;
        }
        break;
        case OP_EQUALS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = icmp eq ", id);
            emitCheshireType(outfile, node->binary.left->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_NOT_EQUALS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = icmp ne ", id);
            emitCheshireType(outfile, node->binary.left->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_GRE_EQUALS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = icmp sge ", id);
            emitCheshireType(outfile, node->binary.left->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_LES_EQUALS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = icmp sle ", id);
            emitCheshireType(outfile, node->binary.left->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_GREATER: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = icmp sgt ", id);
            emitCheshireType(outfile, node->binary.left->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_LESS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = icmp sle ", id);
            emitCheshireType(outfile, node->binary.left->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_AND: {
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = and i1 %%val%d, %%val%d", id, left, right);
            return id;
        }
        break;
        case OP_OR: {
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = or i1 %%val%d, %%val%d", id, left, right);
            return id;
        }
        break;
        case OP_PLUS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = add ", id);
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_MINUS: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = sub ", id);
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_MULT: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = mul ", id);
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_DIV: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = sdiv ", id);
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_MOD: {
            //todo: add Number type support.
            int id = unique_identifier++;
            int left = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int right = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("%%val%d = srem ", id);
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, %%val%d", left, right);
            return id;
        }
        break;
        case OP_SET: {
            int lval = emitExpression(outfile, node->binary.left);
            EMIT_NEWLN();
            int rval = emitExpression(outfile, node->binary.right);
            EMIT_NEWLN();
            EMIT("store ")
            emitCheshireType(outfile, node->determinedType);
            EMIT(" %%val%d, ", rval);
            emitCheshireType(outfile, node->determinedType);
            EMIT("* %%val%d", lval);
            return rval;
        }
        break;
        case OP_INSTANCEOF: {
            
        }
        break;
        case OP_VARIABLE: {
            int varkey = -1;
            for (auto i = scope.begin(); i != scope.end(); ++i) {
                if (i->find(node->string) == i->end())
                    continue;
                varkey = (*i)[node->string];
                break;
            }
            return varkey;
        }
        break;
        case OP_CAST: {
            
        }
        break;
        case OP_NEW_GC: {
            
        }
        break;
        case OP_NEW_HEAP: {
            
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
        case OP_LENGTH: {
            
        }
        break;
        case OP_CLOSURE: {
            
        }
        break;
    }
    PANIC("FATAL ERROR IN CODE EMITTER.");
}