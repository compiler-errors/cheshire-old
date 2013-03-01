#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

void initTypeSystem(void);
void freeTypeSystem(void); //frees all of the char* references

Boolean isType(const char*);
TypeKey getTypeKey(const char*);
TypeKey getLambdaTypeKey(CheshireType returnType, struct tagParameterList* parameters);
CheshireType getType(TypeKey base, Boolean isUnsafe);

Boolean isValidObjectType(CheshireType);
Boolean isValidLambdaType(CheshireType);

#ifdef __cplusplus
}
#endif

#endif /* TYPE_SYSTEM_H */