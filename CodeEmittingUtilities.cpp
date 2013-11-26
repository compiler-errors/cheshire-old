#include <unordered_map>
#include <list>
#include "TypeSystem.h"
#include "TypeSystemUtilities.hpp"
#include "CodeEmitting.h"

typedef std::unordered_map<char*, LLVMValue, CStrHash, CStrEql> TypeScope;
typedef std::unordered_map<CheshireType, ClassShape*, CheshireTypeHash, CheshireTypeEql> ClassShapes;
static std::list<TypeScope> scope;
static std::list<FILE*> preambleList;

ClassShapes classShapes;
extern ObjectMapping objectMapping;
extern AncestryMap ancestryMap;
extern KeyedLambdas keyedLambdas;

static ClassShape* allocClassShape(CheshireType type, const char* name) {
    ClassShape* ret = (ClassShape*) malloc(sizeof(ClassShape));

    if (ret == NULL)
        PANIC_OR_RETURN_NULL;

    ret->name = name;
    ret->type = type;
    ret->next = NULL;
    return ret;
}

static ClassShape* cloneClassShape(ClassShape* node) {
    if (node == NULL)
        return NULL;

    ClassShape* ret = allocClassShape(TYPE_VOID, "?!");
    ret->name = node->name;
    ret->type = node->type;
    ret->next = cloneClassShape(node->next);
    return ret;
}

static Boolean existsInClassShape(ClassShape* c, const char* name) {
    CStrEql streql;

    for (; c != NULL; c = c->next) {
        if (streql(c->name, name))
            return TRUE;
    }

    return FALSE;
}

void initCodeEmitting() {
    raiseVariableScope();
}

void freeCodeEmitting() {
    fallVariableScope();

    for (ClassShapes::iterator i = classShapes.begin(); i != classShapes.end(); ++i) {
        deleteClassShape(i->second);
    }
}

void raiseVariableScope() {
    scope.push_front(TypeScope());
}

void fallVariableScope(void) {
    scope.pop_front();
}

void registerVariable(char* name, LLVMValue value) {
    //printf("registered variable %s", name);
    scope.front()[name] = value;
}

LLVMValue fetchVariable(char* name) {
    for (auto i = scope.begin(); i != scope.end(); ++i) {
        if (i->find(name) != i->end()) {
            return (*i)[name];
        }
    }

    PANIC("Could not find variable %s!", name);
}

void emitLambdaType(FILE* out, CheshireType type) {
    LambdaType l = keyedLambdas[type];
    emitType(out, l.first);
    fprintf(out, "(");
    unsigned int i = 0;

    for (i = 0; i < l.second.size(); i++) {
        emitType(out, l.second[i]);

        if (i != l.second.size() - 1)
            fprintf(out, ", ");
    }

    fprintf(out, ")*");
}

ClassShape* getClassShape(CheshireType type) {
    if (equalTypes(type, TYPE_OBJECT))
        return NULL; //todo: maybe not?

    if (classShapes.find(type) != classShapes.end())
        return classShapes[type];

    ClassShape* shape = cloneClassShape(getClassShape(((CheshireType) {
        ancestryMap[type.typeKey], 0
    })));
    ClassList* object = objectMapping[type.typeKey];
    ClassShape** bottom = &shape;

    for (ClassShape* temp = shape; temp != NULL; temp = temp->next) bottom = &(temp->next);

    for (ClassList* c = object; c != NULL; c = c->next) {
        switch (c->type) {
            case CLT_CONSTRUCTOR:
                break;
            case CLT_VARIABLE:
                *bottom = allocClassShape(c->variable.type, c->variable.name);
                bottom = &((*bottom)->next);
                break;
            case CLT_METHOD:

                if (!existsInClassShape(shape, c->method.name)) {
                    *bottom = allocClassShape(getLambdaType(c->method.returnType, c->method.params), c->method.name);
                    bottom = &((*bottom)->next);
                }

                break;
        }
    }

    classShapes[type] = shape;
    return shape;
}

int getObjectElement(CheshireType type, const char* elementName) {
    ClassShape* shape;

    if (classShapes.find(type) == classShapes.end())
        shape = getClassShape(type);
    else
        shape = classShapes[type];

    CStrEql streql;
    int count = 0;

    for (ClassShape* c = shape; c != NULL; c = c->next, count++)
        if (streql(c->name, elementName))
            return count;

    PANIC("Could not find element %s", elementName);
}

CheshireType getObjectSelfType(CheshireType object, const char* methodname) {
    ClassShape* classShape = getClassShape(object);
    CStrEql streql;

    for (ClassShape* c = classShape; c != NULL; c = c->next) {
        if (streql(methodname, c->name)) {
            ERROR_IF(!isLambdaType(c->type), "Invalid lambda type!");
            LambdaType l = keyedLambdas[c->type];
            ERROR_IF(l.second.size() <= 0, "Error: not object call.");
            return l.second[0];
        }
    }

    PANIC("No such self type!");
}

void deleteClassShape(ClassShape* node) {
    if (node == NULL)
        return;

    deleteClassShape(node->next);
    free(node);
}

FILE* newPreamble(void) {
    FILE* preamble = tmpfile();
    preambleList.push_front(preamble);
    return preamble;
}

void flushPreambles(FILE* out) {
    for (auto i = preambleList.begin(); i != preambleList.end(); ++i) {
        FILE* file = *i;
        fseek(file, 0, SEEK_SET);
        char c;

        while ((c = fgetc(file)) != EOF) {
            //char c = fgetc(file);
            fprintf(out, "%c", c);
        }

        fclose(file);
    }

    preambleList.clear();
}

