/* 
 * File:   TypeSystem.h
 * Author: Michael Goulet
 * Implementation: 
 */

#ifndef TYPESYSTEM_H
#define	TYPESYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef int TypeKey;

typedef struct tagCheshireType {
    TypeKey typeKey;
} CheshireType;

void initTypeSystem(void);
void freeTypeSystem(void); //free all of the char* references

TypeKey getReservedTypeKey(ReservedType);
TypeKey getTypeKey(char*);
TypeKey getLambdaTypeKey(CheshireType returnType, ParameterList* parameters);

CheshireType getType(TypeKey base, Boolean isUnsafe);

#ifdef	__cplusplus
}
#endif

#endif	/* TYPESYSTEM_H */

