/* 
 * File:   main.cpp
 * Author: michael
 *
 * Created on February 27, 2013, 8:20 PM
 */

#include <cstdlib>
#include <cstdio>
#include "LexerUtilities.h"
#include "ParserNodes.h"
#include "SyntaxTreeUtil.h"
#include "TypeSystem.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    initTypeSystem();
    ParameterList* p = linkParameterList(
            getType(1, FALSE), 
            saveStringLiteralReturn("num"), 
            linkParameterList(
                getType(1, FALSE), 
                saveStringLiteralReturn("boo"), 
                NULL));
    CheshireType ret = getType(5, TRUE);
    CheshireType ct = getType(getLambdaTypeKey(ret, p), FALSE);
    CheshireType ct2 = getType(getLambdaTypeKey(ret, p), FALSE);
    CheshireType ct3 = getType(getLambdaTypeKey(getType(2, FALSE), p), FALSE); //different!!
    printf("Types are %d == %d != %d\n", ct.typeKey, ct2.typeKey, ct3.typeKey);
    printCheshireType(ct);
    printf("\n");
    printCheshireType(ct2);
    printf("\n");
    printCheshireType(ct3);
    printf("\n");
    freeTypeSystem();
    
    initTypeSystem();
    p = linkParameterList(
            getType(1, FALSE), 
            saveStringLiteralReturn("num"), 
            linkParameterList(
                getType(1, FALSE), 
                saveStringLiteralReturn("boo"), 
                NULL));
    ret = getType(5, TRUE);
    ct = getType(getLambdaTypeKey(getType(4, FALSE), p), FALSE);
    ct2 = getType(getLambdaTypeKey(ret, p), FALSE);
    ct3 = getType(getLambdaTypeKey(ret, linkParameterList(getType(3, FALSE), saveStringLiteralReturn("poo"), p)), FALSE); //different!!
    printf("Types are %d != %d != %d\n", ct.typeKey, ct2.typeKey, ct3.typeKey);
    printCheshireType(ct);
    printf("\n");
    printCheshireType(ct2);
    printf("\n");
    printCheshireType(ct3);
    printf("\n");
    freeTypeSystem();
    
    initTypeSystem();
    CheshireScope* scope = allocateCheshireScope();
    raiseScope(scope);
    defineVariable(scope, "a", TYPE_INT);
    printf("Variable a has type: "); printCheshireType(getVariableType(scope, "a")); printf("\n");
    raiseScope(scope);
    defineVariable(scope, "a", TYPE_OBJECT_HAT);
    printf("Variable a has type: "); printCheshireType(getVariableType(scope, "a")); printf("\n");
    fallScope(scope);
    printf("Variable a has type: "); printCheshireType(getVariableType(scope, "a")); printf("\n");
    fallScope(scope);
    return 0;
}

