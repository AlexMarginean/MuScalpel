/* 
 * File:   FunctionCallDependency.h
 * Author: alex
 *
 * Created on 01 July 2014, 18:49
 */

#ifndef FUNCTIONCALLDEPENDENCY_H
#define	FUNCTIONCALLDEPENDENCY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ListOfFunctionSourceFile.h"
#include <stdio.h>

    struct callDependency {
        char * LOCMarker;
        ListOfFunctionSourceFile * calledFunctions;
        struct callDependency *next;
    };

    typedef struct callDependency CallDependency;

    CallDependency * readCallDependencyFromFile(char * inputFile);


#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONCALLDEPENDENCY_H */

