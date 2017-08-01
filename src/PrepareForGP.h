/* 
 * File:   PrepareForGP.h
 * Author: alex
 *
 * Created on 30 June 2014, 04:06
 */

#ifndef PREPAREFORGP_H
#define	PREPAREFORGP_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "Mapping.h"
#include "DependencyList.h"
#include "MappingCandidate.h"
#include "FunctionCallDependency.h"
#include "DeclarationSymbolTable.h"

    struct arrayOfStrings {
        int count;
        char ** array;
    };

    typedef struct arrayOfStrings ArrayOfStrings;

    ListOfString * prepareForGP(Mapping ** ListOfMappingsFromCurrentDonor,
            DependencyList ** ListOfDependenciesForStatements, MappingCandidate ** MappingCandidatesForGP,
            CallDependency ** ListOfNeededFunctionsForAllLOCs, DeclarationSymbolTable ** FinalDeclSymbolTable,
            char * skeletonInterfaceSourceCodeOutput, char * skeletonInterfaceHeaderFileOutput,
            char * headerForInterfaceJustName, char * skeletonCoreFeatureHeader, char * hostWithInterfaceOutputFile,
            char * interfaceOutput, char * sourceOutputCoreFunctionality, char * graftSourceCodeFile, char * graftFunctionName,
            int * countOfHostIDs, SymbolTable ** finalHostSymbolTable, char * outputDefinedFunctions,
			char * outputFileLabels, char * TXLTemporaryFolder);

    void createFinalHostInterface(char * interfaceFile, char * coreFeatureFile, char * TXLProgsPath,
            char * finalTransplantLocation);

    ArrayOfStrings createArrayOfLineMutationCandidates(ListOfString * SkeletonSourceCode,
            ListOfString ** MarkersForLineWithAbstract, char * TXLTemporaryFolder);

    ListOfString * createTheListOfAllMarkers(ListOfString * SkeletonSourceCode, char * TXLTemporaryFolder);

    void solveDonorStaticFunctionDeclarationsUniqueNamesAndMarkersReplacement(char * donorSourceFiles, char * TXLTemporaryFolder);

    void extractStmOrCompoundMarkers(char * Skeleton, char * outputFile, char * TXLProgsPath, char * outputFileAbstractMarkers);

    ArrayOfStrings copyArrayOfStrings(ArrayOfStrings initial);




#ifdef	__cplusplus
}
#endif

#endif	/* PREPAREFORGP_H */

