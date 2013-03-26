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
    while (!(ret = yyparse(&node, scanner))) {
        typeCheckTopNode(scope, node); //todo: don't just read it, instead take any method/classes and save it for lookahead, then check @ end!
        printf("Read a node!\n");
        topNodes.push_front(node);
    }
    printf("Broken.\n");
    
    //todo: analyze return of "ret" for errors, or just EOL.
    
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    
    //todo: emit llvm here.
    
    deleteCheshireScope(scope);
    freeTypeSystem();
    return 0;
}

