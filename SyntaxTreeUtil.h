/* 
 * File:   SyntaxTreeUtil.h
 * Author: Michael Goulet
 * Implementation: SyntaxTreeUtil.c
 * 
 * SyntaxTreeUtil.h is meant for utilities for analyzation of syntax trees, such as a "printNode" function.
 * These methods are transient and subject to change, only for small testing and other things.
 */

#ifndef SYNTAXTREEUTIL_H
#define	SYNTAXTREEUTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "Nodes.h"

void printExpression(ExpressionNode*);
void printParameterList(ParameterList*);
void printInternalTypeNode(InternalTypeNode*);

#ifdef	__cplusplus
}
#endif

#endif	/* SYNTAXTREEUTIL_H */

