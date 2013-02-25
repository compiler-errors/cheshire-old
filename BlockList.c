/* File: 
 * Author: Michael Goulet
 * Implements: 
 */

#include "ParserNodes.h"

static BlockList* allocBlockList(void) {
    BlockList* node = (BlockList*) malloc(sizeof(BlockList));
    if (node == NULL)
        PANIC_OR_RETURN_NULL;
    
    node->next = NULL;
    node->statement = NULL;
    return node;
}

BlockList* linkBlockList(StatementNode* val, BlockList* next) {
    BlockList* node = allocBlockList();
    
    if (node == NULL)
        return NULL;
    
    node->statement = val;
    node->next = next;
    return node;
}

void deleteBlockList(BlockList* node) {
    if (node == NULL)
        return;
    
    deleteStatementNode(node->statement);
    deleteBlockList(node->next);
    free(node);
}