#include <stdio.h>
#include <string.h>
#include "Nodes.h"
#include "CheshireParser.yy.h"
#include "CheshireLexer.yy.h"
#include "SyntaxTreeUtil.h"

int yyparse(ExpressionNode**, yyscan_t);

ExpressionNode* parseString(const char* str) {
    ExpressionNode* expression;
    yyscan_t scanner;
    YY_BUFFER_STATE state;
 
    if (yylex_init(&scanner))
    {
        // couldn't initialize
        return NULL;
    }
 
    state = yy_scan_string(str, scanner);
 
    if (yyparse(&expression, scanner))
    {
        // error parsing
        return NULL;
    }
 
    yy_delete_buffer(state, scanner);
 
    yylex_destroy(scanner);
 
    return expression;
}

int main(void) {
    char test[256];
    double result = 0;
    while (1) {
        printf("Please provide an expression, or \"exit\" to quit: ");
        fgets(test, 256, stdin);
        test[strlen(test)-1] = '\0';
        test[100] = '\0';
        if (strcmp(test, "exit") == 0)
            break;

        ExpressionNode* e = parseString(test);
        printf("## EVALUATED ##\n%s\n## INTO ##\n", test);
        printExpression(e);
        printf("\n");
        deleteExpressionNode(e);
    }
    printf("Goodbye!\n");
    return 0;
}