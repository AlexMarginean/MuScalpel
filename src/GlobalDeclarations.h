/* 
 * File:   GlobalDeclarations.h
 * Author: alex
 *
 * Created on 04 July 2014, 18:18
 */

#ifndef GLOBALDECLARATIONS_H
#define	GLOBALDECLARATIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ListOfString.h"
#include "DeclarationWthHeaderDefinition.h"
#include "DependencyList.h"
#include "FunctionCallDependency.h"

    struct listOfDefinedTypesAndVars {
        char * lineMarker;
        ListOfString * definedVar;
        ListOfString * definedTypes;
        struct listOfDefinedTypesAndVars * next;
    };

    typedef struct listOfDefinedTypesAndVars ListOfDefinedTypesAndVars;

    struct listOfStructDefinitions {
        ListOfString * structDef;
        char * headerDefinition;
        struct listOfStructDefinitions * next;
    };

    typedef struct listOfStructDefinitions ListOfStructDefinitions;

    struct globalDeclarations {
        ListOfStructDefinitions * normalDeclarations;
        ListOfStructDefinitions * structUnionDeclarations;
        ListOfStructDefinitions * typedefDeclarations;
    };

    typedef struct globalDeclarations GlobalDeclarations;

    GlobalDeclarations readGlobalDeclarationFromFile(char * inputFile);

    GlobalDeclarations constructListOfGlobalDeclarations(DependencyList ** globalDependencyList,
            ListOfDefinedTypesAndVars ** globalListOfDefinedTypeVar, char * tempGlobalTypeVarDeclaration,
            char * tempDefinesFile, char * tempIncludeLibFile, char * AllDefinedFunctionsInDonor,
            char * GlobalFctDependOutput, char * TXLTemporaryFolder);


    DependencyList * generateGlobalDependencyListForSkeleton(ListOfString * FinalSkeletonSourceCode,
            char * definedGlobalTypesAndVariablesFile, GlobalDeclarations ListOfFinalGlobalDeclarations,
            char * tempGlobalDeclarations, char * TXLTemporaryFolder);

    GlobalDeclarations fixGlobalDeclarationsForSkeleton(ListOfString * FinalSkeletonSourceCode,
            DependencyList ** GlobalDependencyList, DependencyList ** SkeletonDependenciesOnGlobals,
            char * tempGlobalDeclarations, char * definedGlobalTypesAndVariablesFile, char * tempDefinesFile,
            char * tempIncludeLibFile, char * AllDefinedFunctionsInDonor, char * GlobalFctDependOutput,
			char * TXLTemporaryFolder);

    void addNewStructDefinition(ListOfStructDefinitions ** head, ListOfString * structLOCs,
            char * headerDefinition);

    GlobalDeclarations returnMarkerGlobalDeclaration(GlobalDeclarations currentGlobal, ListOfString * markers);

    GlobalDeclarations extractNeededGlobalDeclarationsForSkeleton(ListOfString * SelectedMarkers,
            GlobalDeclarations ListOfFinalGlobalDeclarations,
            DependencyList * GlobalDependencyList, DependencyList * SkeletonDependenciesOnGlobals);

    void printListOfGlobalDeclarations(GlobalDeclarations ListOfDecl, char * outputFile, char * outputFileHeader);

    void addNewListOfStructToAnExistingOne(ListOfStructDefinitions ** destination, ListOfStructDefinitions * source);

    void addGlobalDeclarationListToExistingOne(GlobalDeclarations * head, GlobalDeclarations toBeAdded);

    CallDependency * extractNeededFunctionsForGlobals(GlobalDeclarations ListOfGlobalDeclarations,
        char * GlobalsDependenciesOnFunctFileName);


#ifdef	__cplusplus
}
#endif

#endif	/* GLOBALDECLARATIONS_H */

