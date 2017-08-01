#include "MarkFunctionCall.h"
#include "GenTrans.h"
#include "folderContent.h"
#include "FunctionCallDependency.h"
#include "Utils.h"
#include "ListOfFunctionSourceFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void createListOfDonorFunctions(char * TXLProgsPath, char * listOfSourceFiles, char * outputFile) {
    char * txlIfDefProg;
    txlIfDefProg = (char *) malloc(5000 * sizeof (char));
    sprintf(txlIfDefProg, "%sifdef.x", TXLProgsPath);


    char * commandTXL;
    commandTXL = (char *) malloc(5000 * sizeof (char));
    sprintf(commandTXL, "%sExtractAllDefinedFunctionsName.x %s %s %s %s", TXLProgsPath, listOfSourceFiles, outputFile,
            txlIfDefProg, ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! ExtractAllDefinedFunctionsName");
        fflush(stdout);
    }
    free(commandTXL);
}

void addTheMarkingsToSkeleton(char * TXLProgsPath, char * skeletonFile, char * outputFile, char * existingFctFile) {



    char * commandTXL;
    commandTXL = (char *) malloc(5000 * sizeof (char));
    sprintf(commandTXL, "%sMarkFunctionCall.x %s %s %s %s", TXLProgsPath, skeletonFile, outputFile, existingFctFile,
            ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! MarkFunctionCall.x\n");
    }
    free(commandTXL);
}

CallDependency * markFunctionCallsInSkeleton(char * skeletonFileName, char * outputDefinedFunctions,
		char * TXLTemporaryFolder) {


    char * outputFileName;
    outputFileName = (char *) malloc(500 * sizeof (char));
    sprintf(outputFileName, "%stemp_list_of_call_dependency.out", TXLTemporaryFolder);



    char * listOfSourceFilesFromDonor;
    listOfSourceFilesFromDonor = (char *) malloc(500 * sizeof (char));
    sprintf(listOfSourceFilesFromDonor, "%stemp_donor_files_for_function_list.out", TXLTemporaryFolder);

    FILE * listOfSourceFilesFromDonorFile = fopen(listOfSourceFilesFromDonor, "w");
    readFilesFromDir(donorSourceFileFolder, listOfSourceFilesFromDonorFile, 'c');
    fclose(listOfSourceFilesFromDonorFile);

    //char * listOfDonorFunctions;
    //listOfDonorFunctions = (char *) malloc(500 * sizeof (char));
    //sprintf(listOfDonorFunctions, "%stemp_list_of_function_names.out", TXLTemporaryFolder);

    createListOfDonorFunctions(TXLToolsPath, listOfSourceFilesFromDonor, outputDefinedFunctions);



    addTheMarkingsToSkeleton(TXLToolsPath, skeletonFileName, outputFileName, outputDefinedFunctions);




    CallDependency * ListOfCallDependencies = readCallDependencyFromFile(outputFileName);


    free(listOfSourceFilesFromDonor);

    return ListOfCallDependencies;
}

void constructFinalSkeletonJustWithNeededLOCs(char * skeletonFile, ListOfString * LinesToBeRemoved,
        char * FinalSkeletonOutput, char * TXLTemporaryFolder) {
    fflush(stdout);
    ListOfString * interfaceCode = NULL;
    interfaceCode = readListOfStringFromFile(skeletonFile);
    ListOfString * aux = interfaceCode->next;
    while (aux->next) {
        ListOfString * auxLineToBeRemoved = LinesToBeRemoved;
        int signal = 0;
        while (auxLineToBeRemoved && !signal) {
            if (strstr(aux->el, auxLineToBeRemoved->el)) {
                signal = 1;
            }
            auxLineToBeRemoved = auxLineToBeRemoved->next;
        }
        if (signal) {
            removeString(&interfaceCode, aux->el);
        }
        aux = aux->next;
    }
    //first write to a temp file, since it needs to be pretty printed before sending it further
    char * tempSkeleton;
    tempSkeleton = (char *) malloc(500 * sizeof (int));
    sprintf(tempSkeleton, "%stemp_skeleton_output_not_formatted.out", TXLTemporaryFolder);
    printAListOfStrings(interfaceCode, tempSkeleton);
    fflush(stdout);
    prettyPrintSourceCode(TXLToolsPath, tempSkeleton, FinalSkeletonOutput);
    fflush(stdout);
}

void removeLOCsWithUnwantedFunctions(char * skeletonFile, DependencyList * ListOfDependencies,
        CallDependency * ListOfFunctionCallDependencies, char * DesiredFunctionsFile, char * FinalSkeletonOutput,
		char * TXLTemporaryFolder) {
    ListOfString * userWantedFunction = NULL;
    userWantedFunction = readListOfStringFromFileAsWords(DesiredFunctionsFile);
    ListOfString * linesToBeRemoved = NULL;
    CallDependency * auxCallDep = ListOfFunctionCallDependencies;
    fflush(stdout);
    //remove statement, compound, and declarations that calls unwanted functions
    while (auxCallDep) {
        ListOfFunctionSourceFile * currentCalledFct = auxCallDep->calledFunctions;
        int signal = 1;
        while (currentCalledFct && signal) {
            if (exclude_functions) {
                if (searchStringInListOfStrings(userWantedFunction, currentCalledFct->functionName)) {
                    signal = 0;
                }
            }
            currentCalledFct = currentCalledFct->next;
        }
        if (!signal) {
            addNewString(&linesToBeRemoved, auxCallDep->LOCMarker);
        }
        auxCallDep = auxCallDep->next;
    }
    //remove statements, compound statements or declarations that are use removed declaration
    ListOfString * auxRemovedLocs = linesToBeRemoved;
    while (auxRemovedLocs) {
        DependencyList * auxListOfDependencies = ListOfDependencies;
        while (auxListOfDependencies) {
            if (searchStringInListOfStrings(auxListOfDependencies->ListOfDependencies, auxRemovedLocs->el)) {
                if (!searchStringInListOfStrings(linesToBeRemoved, auxListOfDependencies->Statement)) {
                    addNewString(&linesToBeRemoved, auxListOfDependencies->Statement);
                }
            }
            auxListOfDependencies = auxListOfDependencies->next;
        }
        auxRemovedLocs = auxRemovedLocs->next;
    }
    constructFinalSkeletonJustWithNeededLOCs(skeletonFile, linesToBeRemoved, FinalSkeletonOutput, TXLTemporaryFolder);
}

CallDependency * removeUnwantedFunctionsAndReturnCallDependencies(char * SkeletonFile, char * FinalSkeletonOut,
        DependencyList * ListOfDependenciesForStatements, char * outputDefinedFunctions,
		char * TXLTemporaryFolder) {
    CallDependency * finalCallDependencies = markFunctionCallsInSkeleton(SkeletonFile, outputDefinedFunctions,
    		TXLTemporaryFolder);

    //this are statements that use an unwanted function, declarations that use an unwanted
    //function, or statements that use a declaration that use an unwanted function

    fflush(stdout);
    removeLOCsWithUnwantedFunctions(SkeletonFile, ListOfDependenciesForStatements, finalCallDependencies,
            AcceptedFunctionsForGP, FinalSkeletonOut, TXLTemporaryFolder);

    fflush(stdout);
    return finalCallDependencies;

}
