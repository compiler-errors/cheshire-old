/*
 * File:   TypeSystem.h
 * Author: Michael Goulet
 * Implementation: TypeSystem.c
 */

#ifndef TYPESYSTEM_H
#define	TYPESYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "Enums.h"
#include "ParserNodes.h"

struct tagType;

typedef struct tagType {
} type;

/**
 * type t is up-castable to type ancestor.
 *
 * @param t -- Type to test
 * @param ancestor -- The ancestor type
 * @return (type t) <= (type ancestor) -- "t" is up-castable to "ancestor".
 */
Boolean isUpcast(Type t, Type ancestor);

/**
 * Fits generic "NumberNode"(s) into their expected numerical type.
 *
 * Automatically converts the "double"-typed NumberNode such that it may
 * fit the size that it is expected. (double -> int, double -> Number type).
 * @return NULL if no change, new ExpressionNode pointer if changed.
 * Automatically deletes the old node if necessary.
 */
ExpressionNode* autotypeNumber(NumberNode*, Type expectedType);

#ifdef	__cplusplus
}
#endif

#endif	/* TYPESYSTEM_H */

