/* 
 * File:   CodeEmitting.h
 * Author: Michael Goulet
 * Implementation: 
 */

#ifndef CODEEMITTING_H
#define	CODEEMITTING_H

#include "Structures.h"

#ifdef	__cplusplus
extern "C" {
#endif

    void emitCode(FILE* outfile, ParserTopNode*);

#ifdef	__cplusplus
}
#endif

#endif	/* CODEEMITTING_H */

