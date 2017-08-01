#include "GenTrans.h"
#include "SymbolTable.h"
#include "ListOfString.h"
#include "IndividualInstantiator.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void symbolTableToArgDecls(SymbolTable * head, char * outputFileName) {
    FILE * fout = fopen(outputFileName, "w");
    SymbolTable * p;
    p = head;
    if(!p){
        return;
    }
    while (p->next) {
        char * arrayMarker;
        arrayMarker = (char *) malloc(5 * sizeof (char));
        strcpy(arrayMarker, "");
        char * fixedVarType;
        fixedVarType = (char *) malloc(300 * sizeof (char));
        strcpy(fixedVarType, p->TypeOfVar);
        if ((p->TypeOfVar)[strlen(p->TypeOfVar) - 1] == ']') {
        	char * posStartArray = strstr(fixedVarType, "[");
        	posStartArray[0] = 0;
            //fixedVarType [strlen(fixedVarType) - 2] = 0;
            strcpy(arrayMarker, " []");
        }
        IdOfVariable * aux = p->ListOfIdentifiers;
        while (aux) {
            fprintf(fout, "%s %s%s , ", fixedVarType, aux->ID, arrayMarker);
            aux = aux->next;
        }
        p = p->next;
        free(fixedVarType);
        free(arrayMarker);
    }
    if (p) {
        char * arrayMarker;
        arrayMarker = (char *) malloc(5 * sizeof (char));
        strcpy(arrayMarker, "");
        char * fixedVarType;
        fixedVarType = (char *) malloc(300 * sizeof (char));
        strcpy(fixedVarType, p->TypeOfVar);
        if ((p->TypeOfVar)[strlen(p->TypeOfVar) - 1] == ']') {
        	while(fixedVarType[strlen(fixedVarType) - 1] != '['){
        		fixedVarType[strlen(fixedVarType) -1] = 0;
        	}
            fixedVarType [strlen(fixedVarType) - 1] = 0;
            strcpy(arrayMarker, " []");
        }
        IdOfVariable * aux = p->ListOfIdentifiers;
        while (aux->next) {
            fprintf(fout, "%s %s%s , ", fixedVarType, aux->ID, arrayMarker);
            aux = aux->next;
        }
        fprintf(fout, "%s %s%s", fixedVarType, aux->ID, arrayMarker);
        p = p->next;
        free(fixedVarType);
        free(arrayMarker);
    }
    fclose(fout);
}

void generateInterfaceDefinition(char * TXLPath, char * theGraftFctName, char * graftFile, char * interfaceName, 
        char * newVariableName, char * argumentsFilePath, char * outputFilePath, char * interfaceHeaderFile, 
        char * JustInterfaceFileName, char * hostFileSourceFile) {
    char * command;
    command = (char *) malloc(1000 * sizeof (char));
    sprintf(command, "%screateInterface.x", TXLPath);
    char * commandTXL;
    commandTXL = (char *) malloc(5000 * sizeof (char));
    sprintf(commandTXL, "%s %s %s %s %s %s %s %s %s %s %s %s %s", command, transplantHostInputFile /*any syntax correct c program*/,
            graftFile, theGraftFctName, interfaceName,
            newVariableName, argumentsFilePath, outputFilePath, interfaceHeaderFile, JustInterfaceFileName, 
            hostFileSourceFile, IfDefProgramPath, ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
    	printf("%s\n", commandTXL);
        printf("ERROR generateInterfaceDefinition!\n");
    }
    free(command);
    free(commandTXL);

    if(is_demo){
        char * cmdCopyDemoFiles = (char *) malloc (1000 * sizeof(char));
        sprintf(cmdCopyDemoFiles, "cp %s %sGraftDeclaration.h",  interfaceHeaderFile, demoPath);
        system(cmdCopyDemoFiles);


#ifdef DETAILED_DEMO
        printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the graft declaration header file: %sGraftDeclaration.h . \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath);
        getchar();
#endif

    }
}



void extractHostIncludes(char * TXLProgsPath, char * source, char * destination) {
    char * commandTXL;
    commandTXL = (char *) malloc(500 * sizeof (char));
    sprintf(commandTXL, "%sextractHostIncludes.x %s %s %s", TXLProgsPath, source, destination,
            ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR extract host includes!");
    }
    free(commandTXL);
}

void generateNewDonorAndInterface(char * TXLPath, char * hostTargetPlaceFile, char * tempInterfaceDecl,
        char * outputFileName, char * newInterfaceFileName, char * interfaceHeaderFile) {

    char * command;
    command = (char *) malloc(400 * sizeof (char));
    sprintf(command, "%saddInterfaceToHost.x", TXLPath);
    char * commandTXL;
    commandTXL = (char *) malloc(2000 * sizeof (char));
    sprintf(commandTXL, "%s %s %s %s %s %s %s", command, hostTargetPlaceFile, tempInterfaceDecl,
            outputFileName, newInterfaceFileName, interfaceHeaderFile, ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! generateNewDonorAndInterface\n");
    }
    free(command);
    free(commandTXL);

    if(is_demo){
        char * cmdCopyDemoFiles = (char *) malloc (1000 * sizeof(char));
        sprintf(cmdCopyDemoFiles, "cp %s %sGraftDefinition.c && cp %s %sHostWithInterface.c", newInterfaceFileName,
                demoPath, outputFileName, demoPath);
        system(cmdCopyDemoFiles);


#ifdef DETAILED_DEMO
        printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the graft source definition file: %sGraftDefinition.c; and the host with interface instantiated file: %sHostWithInterface.c . \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath, demoPath);
        getchar();
#endif

    }
    
    //add the includes in host to this interface
    /*
    char * tempIncludeFile = (char *) malloc (5000 * sizeof(char));
    sprintf(tempIncludeFile, "%stemp_includes_from_host.out", TXLTemporaryFolder);
    
    extractHostIncludes(TXLPath, transplantHostInputFile, tempIncludeFile);
    readListOfStringFromFile(tempIncludeFile);
    
    addLocsFromSourceToDestination(newInterfaceFileName, tempIncludeFile, TXLPath);
     */ 
}

void generateHostInterface(SymbolTable * head, char * tempFilesPath,
        char * TXLPath, char * graftFile, char * interfaceName, char * newVariableName, char * outputFileName,
        char * interfaceHeaderFile, char * hostTargetPlaceFile, char * newInterfaceFileName, 
        char * justInterfaceFileName, char * theGraftFunctionName) {
    char * argumentsFileOutput;
    argumentsFileOutput = (char *) malloc(500 * sizeof (char));
    strcpy(argumentsFileOutput, tempFilesPath);
    strcat(argumentsFileOutput, "temp_argTable.out");
    char * tempInterfaceDecl;
    tempInterfaceDecl = (char *) malloc(500 * sizeof (char));
    strcpy(tempInterfaceDecl, tempFilesPath);
    strcat(tempInterfaceDecl, "temp_decl_from_interface.out");
    symbolTableToArgDecls(head, argumentsFileOutput);
    generateInterfaceDefinition(TXLPath, theGraftFunctionName, graftFile, interfaceName, newVariableName, 
            argumentsFileOutput, tempInterfaceDecl, interfaceHeaderFile, justInterfaceFileName, hostTargetPlaceFile);
    generateNewDonorAndInterface(TXLPath, hostTargetPlaceFile, tempInterfaceDecl, outputFileName,
            newInterfaceFileName, interfaceHeaderFile);
    free(argumentsFileOutput);
    free(tempInterfaceDecl);
}
