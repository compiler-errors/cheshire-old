#include <unordered_map>
#include <list>
#include "TypeSystemUtilities.hpp"
#include "CodeEmitting.h"

typedef std::unordered_map<char*, LLVMValue, CStrHash, CStrEql> TypeScope;
static std::list<TypeScope> scope;

extern KeyedLambdas keyedLambdas;

void initVariableScope() {
    raiseVariableScope();
}

void freeVariableScope() {
    fallVariableScope();
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