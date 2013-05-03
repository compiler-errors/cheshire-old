/* File: UsingList.c
 * Author: Michael Goulet
 * Implements: ParserNodes.h
 */

#include "ParserNodes.h"

static UsingList* allocUsingList(void) {
    UsingList* node = (UsingList*) malloc(sizeof(UsingList));

    if (node == NULL)
        PANIC_OR_RETURN_NULL;

    node->next = NULL;
    node->variable = NULL;
    return node;
}

UsingList* linkUsingList(char* variable, UsingList* next) {
    UsingList* node = allocUsingList();

    if (node == NULL)
        return NULL;

    node->variable = variable;
    node->next = next;
    return node;
}

void deleteUsingList(UsingList* node) {
    if (node == NULL)
        return;

    free(node->variable);
    deleteUsingList(node->next);
    free(node);
}
