/* File: LexerUtil.c
 * Author: Michael Goulet
 * Implements LexerUtil.h
 */

#include <stdlib.h>
#include <string.h>
#include "LexerUtil.h"
#include "ParserEnums.h"

void determineReservedType(const char* string, ReservedType* var) {
    switch (string[0]) {
        case 'N':
            *var = RT_NUMBER;
            break;
        case 'B':
            *var = RT_BOOLEAN;
            break;
        case 'I':
            *var = RT_INT;
            break;
        case 'D':
            *var = RT_DECIMAL;
            break;
        case 'v':
            *var = RT_VOID;
            break;
        case 'i':
            *var = RT_INFER;
            break;
        case 'O':
            *var = RT_OBJECT;
            break;
        default:
            *var = -1;
            break;
    }
}

void determineReservedLiteral(const char* string, ReservedLiteral* var) {
    switch (string[0]) {
        case 'T':
            *var = RL_TRUE;
            break;
        case 'F':
            *var = RL_FALSE;
            break;
        case 'N':
            *var = RL_NULL;
            break;
        default:
            *var = -1;
            break;
    }
}

void determineOpType(const char* string, OperationType* var) {
    switch(string[0]) {
        case 'a':
            *var = OP_AND;
            break;
        case 'c':
            *var = OP_COMPL;
            break;
        case 'n':
            *var = OP_NOT;
            break;
        case 'o':
            *var = OP_OR;
            break;
        case 's':
            *var = OP_SIZEOF;
            break;
        case '*':
            *var = OP_MULT;
            break;
        case '/':
            *var = OP_DIV;
            break;
        case '%':
            *var = OP_MOD;
            break;
        case '!':
            *var = OP_NOT_EQUALS;
            break;
        case '>':
            if (string[1] == '=')
                *var = OP_GRE_EQUALS;
            else
                *var = OP_GREATER;
            break;
        case '<':
            if (string[1] == '=')
                *var = OP_LES_EQUALS;
            else
                *var = OP_LESS;
            break;
        case '=':
            if (string[1] == '=')
                *var = OP_EQUALS;
            else
                *var = OP_SET;
            break;
        case '+':
            if (string[1] == '+')
                *var = OP_INCREMENT;
            else
                *var = OP_PLUS;
            break;
        case '-':
            if (string[1] == '-')
                *var = OP_DECREMENT;
            else
                *var = OP_MINUS;
            break;
    }
}

void saveIdentifier(const char* string, char** var) {
    int stringlen = strlen(string);
    char* cpystring = malloc(sizeof(char) * (stringlen+1));
    memcpy(cpystring, string, stringlen);
    cpystring[stringlen] = '\0';
    *var = cpystring;
}

void saveStringLiteral(const char* string, char** var) {
    int substringlen = strlen(string) - 1;
    char* substring = malloc(sizeof(char) * substringlen); //subtract 2 quotes, add 1 "\0"
    memcpy(substring, string+1, substringlen);
    substring[substringlen-1] = '\0';
    *var = substring;
}