/*
 * File:   main.cpp
 * Author: michael
 *
 * Created on February 27, 2013, 8:20 PM
 */

#include <cstdlib>
#include <cstdio>
#include <list>
#include <fstream>
#include "Structures.h"
#include "TypeSystem.h"

extern "C" {
#include "CheshireParser.yy.h"
#include "CheshireLexer.yy.h"
    int yyparse(ParserTopNode**, yyscan_t);
}


using namespace std;


/*
 *
 */
int main(int argc, char** argv) {
    char* source;
    initTypeSystem();
    list<ParserTopNode*> topNodes;
    CheshireScope* scope = allocateCheshireScope();
    yyscan_t scanner;
    YY_BUFFER_STATE state;
    ParserTopNode* node = NULL;

    if (yylex_init(&scanner)) {
        PANIC("Could not initialize lexer");
    }

    printf("Initialized lex, now I'm going to initialize the buffer.\n");
    state = yy_create_buffer(stdin, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);
    int ret = 0;
    printf("Initialized, waiting for input!\n");

    while (true) {
        ret = yyparse(&node, scanner);

        if (ret == 1)
            PANIC("Reached a fatal error in parsing!");

        if (ret == 2)
            break;

        defineTopNode(scope, node);
        topNodes.push_front(node);
    }

    printf("Now type checking...\n");

    for (list<ParserTopNode*>::iterator i = topNodes.begin(); i != topNodes.end(); ++i) {
        typeCheckTopNode(scope, *i);
    }

    printf("Type checked successfully!\n");
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    //todo: emit llvm here.
    deleteCheshireScope(scope);
    freeTypeSystem();
    return 0;
}

