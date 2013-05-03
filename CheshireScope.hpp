/*
 * File:   CheshireScope.hpp
 * Author: Michael Goulet
 * Implementation:
 */

#ifndef CHESHIRESCOPE_HPP
#define	CHESHIRESCOPE_HPP

#ifdef __cplusplus

#include <list>
#include <unordered_map>
#include "Structures.h"
#include "TypeSystemUtilities.hpp"

using std::unordered_map;
using std::list;

extern "C" {

    struct tagCheshireScope;
    struct tagVariableScope;

    typedef struct tagCheshireScope {
        struct tagVariableScope* highestScope;
    } CheshireScope;

    typedef struct tagVariableScope {
        unordered_map<const char*, CheshireType, CStrHash, CStrEql> variables;
        struct tagVariableScope* parentScope;
    } VariableScope;

}

#else /* __cplusplus */

#define CheshireScope void
#define VariableScope void

#endif /* __cplusplus */

#endif /* CHESHIRESCOPE_HPP */
