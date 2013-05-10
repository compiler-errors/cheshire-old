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
#include "CodeEmitting.h"

extern "C" {
#include "CheshireParser.yy.h"
#include "CheshireLexer.yy.h"
    int yyparse(ParserTopNode**, yyscan_t);
}


using namespace std;

//todo: think of a "parameter_list -> expression" syntax, ex ((Int a, Int b) -> a + b)(1, 2) would result in 3.
//todo: using list captures

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

    //printf("Initialized lex, now I'm going to initialize the buffer.\n");
    state = yy_create_buffer(stdin, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);
    int ret = 0;
    //printf("Initialized, waiting for input!\n");

    while (true) {
        ret = yyparse(&node, scanner);

        if (ret == 1)
            PANIC("Reached a fatal error in parsing!");

        if (ret == -2) //-2 = EOF.
            break;

        if (node != NULL) {
            defineTopNode(scope, node);
            topNodes.push_back(node);
        }
    }

    //printf("Now type checking...\n");

    for (list<ParserTopNode*>::iterator i = topNodes.begin(); i != topNodes.end(); ++i) {
        typeCheckTopNode(scope, *i);
    }

    //printf("Type checked successfully! Code emitting: \n");
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    initCodeEmitting();

    for (list<ParserTopNode*>::iterator i = topNodes.begin(); i != topNodes.end(); ++i) {
        forwardDefinition(*i);
    }

    for (list<ParserTopNode*>::iterator i = topNodes.begin(); i != topNodes.end(); ++i) {
        emitCode(stdout, *i);
    }

    for (list<ParserTopNode*>::iterator i = topNodes.begin(); i != topNodes.end(); ++i) {
        deleteParserTopNode(*i);
    }

    freeCodeEmitting();
    deleteCheshireScope(scope);
    freeTypeSystem();
    return 0;
}
