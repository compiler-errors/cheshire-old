/*
 * File:   LexerUtil.h
 * Author: Michael Goulet
 * Implementation: LexerUtil.c
 *
 * "LexerUtil.h" is meant for small functions that are used solely by the lexer (CheshireLexer.lex)
 * The functions are only used internally within the lexer and are subject to change,
 * and thus should not be trusted when used outside of the lexer.
 */

#ifndef EXPRESSION_H
#define	EXPRESSION_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ParserEnums.h"

    void determineReservedLiteral(const char*, ReservedLiteral* var);
    void determineOpType(const char*, OperationType* var);
    void saveIdentifier(const char*, char** var);
    void saveStringLiteral(const char*, char** var);
    char* saveStringLiteralReturn(const char*);

#ifdef	__cplusplus
}
#endif

#endif	/* EXPRESSION_H */

