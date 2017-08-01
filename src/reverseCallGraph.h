/* 
 * File:   reverseCallGraph.h
 * Author: alex
 *
 * Created on 19 June 2014, 01:22
 */

#ifndef REVERSECALLGRAPH_H
#define	REVERSECALLGRAPH_H


#include "ListOfString.h"

#ifdef	__cplusplus
extern "C" {
#endif

void generateReverseCallGraph(char * TXLPath, char * donorSourceFiles,
		char * transplantWorkingFolder, char * reversedCflowOutput,
		char * SkeletonInstantiateMappings, char * HostSymbolTable,
		char * CurrentExistingMappings, char * temporaryFolder,
		char * hostTransplantSourceLocation,
		char * currentCoreTransplantFunction,
		ListOfString ** listOfReverseCallGraph,
		char * outputFileLabels);



#ifdef	__cplusplus
}
#endif

#endif	/* REVERSECALLGRAPH_H */
