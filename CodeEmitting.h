/*
 * File:   CodeEmitting.h
 * Author: Michael Goulet
 * Implementation:
 */

#ifndef CODEEMITTING_H
#define	CODEEMITTING_H

#include <stdio.h>
#include "Structures.h"

#ifdef	__cplusplus
extern "C" {
#endif

    void emitCode(FILE*, ParserTopNode*);
    void emitBlock(FILE*, BlockList*);
    void emitStatement(FILE*, StatementNode*);
    LLVMValue emitExpression(FILE*, ExpressionNode*);
    void emitValue(FILE*, LLVMValue);
    void emitType(FILE*, CheshireType);
    void emitLambdaType(FILE*, CheshireType);

    void initCodeEmitting(void);
    void freeCodeEmitting(void);
    void raiseVariableScope(void);
    void fallVariableScope(void);
    void registerVariable(char* name, LLVMValue);
    LLVMValue fetchVariable(char* name);

    ClassShape* getClassShape(CheshireType);
    int getObjectElement(CheshireType, const char* elementName);
    CheshireType getObjectSelfType(CheshireType object, const char* methodname);

#ifdef	__cplusplus
}
#endif

#endif	/* CODEEMITTING_H */

