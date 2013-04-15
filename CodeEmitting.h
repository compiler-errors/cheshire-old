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

    void raiseVariableScope(void);
    void fallVariableScope(void);
    void registerVariable(char* name, LLVMValue);
    LLVMValue fetchVariable(char* name);

#ifdef	__cplusplus
}
#endif

#endif	/* CODEEMITTING_H */

