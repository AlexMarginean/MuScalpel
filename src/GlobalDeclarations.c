#include "GlobalDeclarations.h"
#include "ListOfString.h"
#include "ListOfFunctionSourceFile.h"
#include "GenTrans.h"
#include "folderContent.h"
#include "DependencyList.h"
#include "DeclarationSymbolTable.h"
#include "FunctionCallDependency.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NORMAL_DECLARATION_MARKER "NORMAL_DECLARATION\n"
#define STRUCT_UNION_MARKER "STRUCT_DECLARATION\n"
#define TYPEDEF_MARKER "TYPEDEF_DECLARATION\n"
#define STRUCT_DECL_BEGINNING_MARKER "/* STRUCT_DECL_"
#define GLOBAL_DECL_BEGINNING_MARKER "/* GLOBAL_DECL_"
#define TYPEDEF_DECL_BEGINNING_MARKER "/* TYPEDEF_DECL_"
#define ENUM_DECL_BEGINNING_MARKER "/* ENUM_DECL"

void freeListOfListOfDefinedTypesAndVars(ListOfDefinedTypesAndVars ** head) {
    ListOfDefinedTypesAndVars * aux;

    while (* head) {
        aux = *head;
        * head = (*head) -> next;
        freeListOfStrings(&aux->definedTypes);
        freeListOfStrings(&aux->definedVar);
        free(aux->lineMarker);
        aux->definedTypes = NULL;
        aux->definedVar = NULL;
        aux->lineMarker = NULL;
        aux->next = NULL;
        free(aux);
        aux = NULL;
    }
    (* head) = NULL; //just in case...
}

void addNewListOfDefinedTypesAndVars(ListOfDefinedTypesAndVars ** head, char * lineMarker,
        ListOfString * definedTypes, ListOfString * definedVars) {
    ListOfDefinedTypesAndVars * newEl = (ListOfDefinedTypesAndVars *) malloc(1 * sizeof (ListOfDefinedTypesAndVars));
    newEl->definedTypes = NULL;
    newEl->definedVar = NULL;
    newEl->lineMarker = (char *) malloc((strlen(lineMarker) + 1) * sizeof (char));
    strcpy(newEl->lineMarker, lineMarker);

    newEl->definedTypes = copyCompleteListOfString(definedTypes);
    newEl->definedVar = copyCompleteListOfString(definedVars);

    newEl->next = NULL;
    if (* head == NULL) {
        * head = newEl;
    } else {
        ListOfDefinedTypesAndVars * p;
        p = *head;
        while (p ->next) {
            p = p->next;
        }
        p ->next = newEl;
    }
}


#define DECLARED_TYPES_MARKER "DECLARED_TYPES"
#define DECLARED_GLOBALS_MARKER "DECLARED_GLOBALS"
#define GLOBAL_END_MARKER "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"

ListOfDefinedTypesAndVars * readListOfDefinedTypesAndVarsFromFile(char * inputFile) {
    ListOfDefinedTypesAndVars * result = NULL;

    FILE * fin = fopen(inputFile, "r");

    char * line;
    line = (char *) malloc(4000 * sizeof (char));
    char * currentLine;
    currentLine = (char *) malloc(4000 * sizeof (char));

    while (fgets(line, 3999, fin)) {
        strtok(line, "\n");
        strcpy(currentLine, line);
        fgets(line, 3999, fin);
        fgets(line, 3999, fin);
        ListOfString * declaredTypesList = NULL;
        ListOfString * declaredGlobalsList = NULL;

        while (!strstr(line, GLOBAL_END_MARKER) && !strstr(line, DECLARED_GLOBALS_MARKER)) {
            addNewString(&declaredTypesList, line);
            fgets(line, 3999, fin);
        }
        if (!strstr(line, GLOBAL_END_MARKER)) {
            fgets(line, 3999, fin);
            while (!strstr(line, GLOBAL_END_MARKER)) {
                addNewString(&declaredGlobalsList, line);
                fgets(line, 3999, fin);
            }
        }
        addNewListOfDefinedTypesAndVars(&result, currentLine, declaredTypesList, declaredGlobalsList);
    }
    fclose(fin);
    return result;
}


//list of structs definitions

void freeListOfStructDefinitions(ListOfStructDefinitions ** head) {
    ListOfStructDefinitions * aux;

    while (* head) {
        aux = *head;
        * head = (*head) -> next;
        freeListOfStrings(&aux->structDef);
        free(aux->headerDefinition);
        aux->headerDefinition = NULL;
        aux->structDef = NULL;
        aux->next = NULL;
        free(aux);
        aux = NULL;
    }
    (* head) = NULL; //just in case...
}

void addNewStructDefinitionAtBeggining(ListOfStructDefinitions ** head, ListOfString * structLOCs,
        char * headerDefinition) {
    ListOfStructDefinitions * newEl = (ListOfStructDefinitions *) malloc(1 * sizeof (ListOfStructDefinitions));
    newEl->structDef = NULL;

    newEl->structDef = copyCompleteListOfString(structLOCs);

    newEl->headerDefinition = NULL;

    if (headerDefinition) {
        newEl->headerDefinition = (char *) malloc((strlen(headerDefinition) + 1) * sizeof (char));
        strcpy(newEl->headerDefinition, headerDefinition);
    }

    newEl->next = (*head);
    (*head) = newEl;
}

void addNewStructDefinition(ListOfStructDefinitions ** head, ListOfString * structLOCs,
        char * headerDefinition) {
    ListOfStructDefinitions * newEl = (ListOfStructDefinitions *) malloc(1 * sizeof (ListOfStructDefinitions));
    newEl->structDef = NULL;

    newEl->structDef = copyCompleteListOfString(structLOCs);

    newEl->headerDefinition = NULL;

    if (headerDefinition) {
        newEl->headerDefinition = (char *) malloc((strlen(headerDefinition) + 1) * sizeof (char));
        strcpy(newEl->headerDefinition, headerDefinition);
    }

    newEl->next = NULL;
    if (* head == NULL) {
        * head = newEl;
    } else {
        ListOfStructDefinitions * p;
        p = *head;
        while (p ->next) {
            p = p->next;
        }
        p ->next = newEl;
    }
}

//search just by looking at the first line, since it should be unique (the name of the struct)

int searchStructInAListOfStruct(ListOfStructDefinitions * head, ListOfString * searchedStruct) {
    ListOfStructDefinitions * aux = head;

    while (aux) {
        if (!strcmp(aux->structDef->el, searchedStruct->el)) {
            return 1;
        }
        aux = aux->next;
    }
    return 0;
}

void addNewListOfStructToAnExistingOne(ListOfStructDefinitions ** destination, ListOfStructDefinitions * source) {
    ListOfStructDefinitions * aux = source;

    while (aux) {
        if (!searchStructInAListOfStruct((*destination), aux->structDef)) {
            addNewStructDefinition(&(*destination), aux->structDef, aux->headerDefinition);
        }
        aux = aux->next;
    }
}



//global declarations functions

#define HEADER_DECLARATION_MARKER "HEADER_DECL"
#define GLOBAL_DECLARATION_MARKER "/* GLOBAL_DECL_"
#define ENUM_DECLARATION_MARKER "/* ENUM_DECL_"

GlobalDeclarations readGlobalDeclarationFromFile(char * inputFile) {
    GlobalDeclarations listOfDecl;

    listOfDecl.normalDeclarations = NULL;
    listOfDecl.structUnionDeclarations = NULL;
    listOfDecl.typedefDeclarations = NULL;

    FILE * fin = fopen(inputFile, "r");

    char * line;
    line = (char *) malloc(30000 * sizeof (char));

    //first for normal marker
    fgets(line, 29999, fin);

    fgets(line, 29999, fin);



    while (strcmp(line, STRUCT_UNION_MARKER)) {


        ListOfString * newStructDef = NULL;
        addNewString(&newStructDef, line);
        fgets(line, 2999, fin);

        while (strcmp(line, STRUCT_UNION_MARKER) && !strstr(line, GLOBAL_DECL_BEGINNING_MARKER)
                && !strstr(line, HEADER_DECLARATION_MARKER) && !strstr(line, ENUM_DECL_BEGINNING_MARKER)) {
            addNewString(&newStructDef, line);
            fgets(line, 2999, fin);
        }

        char * newHeaderDecl = NULL;
        if (strstr(line, HEADER_DECLARATION_MARKER)) {
            fgets(line, 2999, fin);
            if (!strstr(line, GLOBAL_DECL_BEGINNING_MARKER) && !strstr(line, ENUM_DECL_BEGINNING_MARKER)
                    && strcmp(line, STRUCT_UNION_MARKER)) {
                newHeaderDecl = (char *) malloc(1000 * sizeof (char));
                strcpy(newHeaderDecl, line);
                fgets(line, 2999, fin);
            }
        }

        addNewStructDefinition(&listOfDecl.normalDeclarations, newStructDef, newHeaderDecl);


        /*
        char * globalDecl;
                globalDecl = (char *) malloc(20000 * sizeof (char));
                strcpy(globalDecl, line);
                //addNewString(&listOfDecl.normalDeclarations, line);
                fgets(line, 29999, fin);
                fgets(line, 29999, fin);
                char * newHeaderDecl = NULL;
                if (!strstr(line, GLOBAL_DECLARATION_MARKER) && !strstr(line, ENUM_DECLARATION_MARKER) && strcmp(line, STRUCT_UNION_MARKER)) {
                    newHeaderDecl = (char *) malloc(10000 * sizeof (char));
                    strcpy(newHeaderDecl, line);
                    fgets(line, 29999, fin);
                }
                addNewDeclarationWithHeaderDefinition(&listOfDecl.normalDeclarations, globalDecl, newHeaderDecl);
         */
    }

    fgets(line, 2999, fin);
    while (strcmp(line, TYPEDEF_MARKER)) {
        ListOfString * newStructDef = NULL;
        addNewString(&newStructDef, line);
        fgets(line, 2999, fin);
        while (strcmp(line, TYPEDEF_MARKER) && !strstr(line, STRUCT_DECL_BEGINNING_MARKER)
                && !strstr(line, HEADER_DECLARATION_MARKER)) {
            addNewString(&newStructDef, line);
            fgets(line, 2999, fin);
        }
        char * newHeaderDecl = NULL;
        if (strstr(line, HEADER_DECLARATION_MARKER)) {
            fgets(line, 2999, fin);
            if (!strstr(line, STRUCT_DECL_BEGINNING_MARKER) && strcmp(line, TYPEDEF_MARKER)) {
                newHeaderDecl = (char *) malloc(1000 * sizeof (char));
                strcpy(newHeaderDecl, line);
                fgets(line, 2999, fin);
            }
        }
        addNewStructDefinition(&listOfDecl.structUnionDeclarations, newStructDef, newHeaderDecl);
        //addNewString(&listOfDecl.structUnionDeclarations, line);
        //fgets(line, 2999, fin);
    }


    char * testEOF = fgets(line, 299, fin);

    while (testEOF != NULL) {
        ListOfString * newStructDef = NULL;
        addNewString(&newStructDef, line);
        //fgets(line, 2999, fin);
        while ((testEOF = fgets(line, 2999, fin)) && !strstr(line, TYPEDEF_DECL_BEGINNING_MARKER)) {
            addNewString(&newStructDef, line);
            //fgets(line, 2999, fin);
        }


        addNewStructDefinition(&listOfDecl.typedefDeclarations, newStructDef, NULL);
        /*
        addNewString(&listOfDecl.typedefDeclarations, line);
         */
    }

    free(line);

    fclose(fin);
    return listOfDecl;
}

void extractAllGlobalDeclStructAndTypedefDefinitions(char * sourceAndHeaderFileNames,
        char * outputFile, char * tempGlobalDependencies, char * tempGlobalTypeVarDeclaration,
        char * ifdefFile, char * TXLProgsPath, char * tempDefinesFile, char * tempIncludeLibFile,
        char * AllDefinedFunctionsInDonor, char * GlobalFctDependOutput) {
    char * commandTXL;
    commandTXL = (char *) malloc(3000 * sizeof (char));
    sprintf(commandTXL, "%sExtractGlobalDefinitionStructDeclarations.x %s %s %s %s %s %s %s %s %s %s", TXLProgsPath, sourceAndHeaderFileNames,
            outputFile, tempGlobalDependencies, tempGlobalTypeVarDeclaration, ifdefFile, tempDefinesFile,
            tempIncludeLibFile, AllDefinedFunctionsInDonor, GlobalFctDependOutput, ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! sExtractGlobalDefinitionStructDeclarations.x\n");
    }
    free(commandTXL);
}

GlobalDeclarations constructListOfGlobalDeclarations(DependencyList ** globalDependencyList,
        ListOfDefinedTypesAndVars ** globalListOfDefinedTypeVar, char * tempGlobalTypeVarDeclaration,
        char * tempDefinesFile, char * tempIncludeLibFile, char * AllDefinedFunctionsInDonor,
        char * GlobalFctDependOutput, char * TXLTemporaryFolder) {


    char * tempGlobalDependencies;
    tempGlobalDependencies = (char *) malloc(500 * sizeof (char));
    sprintf(tempGlobalDependencies, "%stemp_global_dependencies.out", TXLTemporaryFolder);


    char * sourceHeaderFileName;
    sourceHeaderFileName = (char *) malloc(500 * sizeof (char));

    sprintf(sourceHeaderFileName, "%stemp_list_of_source_header_files.out", TXLTemporaryFolder);

    FILE * sourceHeaderFile = fopen(sourceHeaderFileName, "w");
    readFilesFromDir(donorSourceFileFolder, sourceHeaderFile, 'c');
    readFilesFromDir(donorSourceFileFolder, sourceHeaderFile, 'h');
    fclose(sourceHeaderFile);

    char * txlIfDefProg;
    txlIfDefProg = (char *) malloc(1000 * sizeof (char));
    sprintf(txlIfDefProg, "%sifdef.x", TXLToolsPath);

    char * tempGlobalDefsOutput;
    tempGlobalDefsOutput = (char *) malloc(500 * sizeof (char));

    sprintf(tempGlobalDefsOutput, "%stemp_global_defs_output.out", TXLTemporaryFolder);

    fflush(stdout);

    extractAllGlobalDeclStructAndTypedefDefinitions(sourceHeaderFileName, tempGlobalDefsOutput,
            tempGlobalDependencies, tempGlobalTypeVarDeclaration, txlIfDefProg, TXLToolsPath,
            tempDefinesFile, tempIncludeLibFile, AllDefinedFunctionsInDonor, GlobalFctDependOutput);

    fflush(stdout);

    GlobalDeclarations finalGlobalDecl = readGlobalDeclarationFromFile(tempGlobalDefsOutput);

    * globalDependencyList = readDependencyListFromFile(tempGlobalDependencies);

    * globalListOfDefinedTypeVar = readListOfDefinedTypesAndVarsFromFile(
            tempGlobalTypeVarDeclaration);

    return finalGlobalDecl;
}

void callInterfaceGlobalDependencySolver(char * tempSkeletonFile, char * tempGlobalDeclarations, char * tempGlobalTypeVarDecl,
        char * TXLProgsPath, char * temporaryOutputFile) {
    char * commandTXL;
    commandTXL = (char *) malloc(1000 * sizeof (char));
    sprintf(commandTXL, "%sGlobalVarTypesDeclDependencyINTERFACE.x %s %s %s %s %s", TXLProgsPath, tempSkeletonFile,
            tempGlobalDeclarations, tempGlobalTypeVarDecl, temporaryOutputFile, ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! GlobalVarTypesDeclDependencyINTERFACE");
        fflush(stdout);
    }
    free(commandTXL);
}

DependencyList * generateGlobalDependencyListForSkeleton(ListOfString * FinalSkeletonSourceCode,
        char * definedGlobalTypesAndVariablesFile, GlobalDeclarations ListOfFinalGlobalDeclarations,
        char * tempGlobalDeclarations, char * TXLTemporaryFolder) {
    char * tempSkeletonSourceCode;
    tempSkeletonSourceCode = (char *) malloc(500 * sizeof (char));
    sprintf(tempSkeletonSourceCode, "%stemp_skeleton_for_global_depend", TXLTemporaryFolder);
    printAListOfStrings(FinalSkeletonSourceCode, tempSkeletonSourceCode);


    FILE * fileTempGlobalDecl = fopen(tempGlobalDeclarations, "w");
    ListOfStructDefinitions * auxDeclHeader = ListOfFinalGlobalDeclarations.normalDeclarations;


    while (auxDeclHeader) {

        ListOfString * auxMultiLineDecl = auxDeclHeader->structDef;

        while (auxMultiLineDecl) {
            fprintf(fileTempGlobalDecl, "%s ", auxMultiLineDecl->el);
            auxMultiLineDecl = auxMultiLineDecl->next;
        }

        auxDeclHeader = auxDeclHeader->next;
    }

    fclose(fileTempGlobalDecl);

    //printAListOfStrings(ListOfFinalGlobalDeclarations.normalDeclarations, tempGlobalDeclarations);




    char * tempDependOutput;
    tempDependOutput = (char *) malloc(500 * sizeof (char));
    sprintf(tempDependOutput, "%stemp_skeleton_dependency_file.out", TXLTemporaryFolder);
    callInterfaceGlobalDependencySolver(tempSkeletonSourceCode, tempGlobalDeclarations, definedGlobalTypesAndVariablesFile,
            TXLToolsPath, tempDependOutput);
    DependencyList * skeletonGlobalDependencies = readDependencyListFromFile(tempDependOutput);
    return skeletonGlobalDependencies;
}

GlobalDeclarations fixGlobalDeclarationsForSkeleton(ListOfString * FinalSkeletonSourceCode,
        DependencyList ** GlobalDependencyList, DependencyList ** SkeletonDependenciesOnGlobals,
        char * tempGlobalDeclarations, char * definedGlobalTypesAndVariablesFile, char * tempDefinesFile,
        char * tempIncludeLibFile, char * AllDefinedFunctionsInDonor, char * GlobalFctDependOutput,
		char * TXLTemporaryFolder) {
    //not used for the moment
    ListOfDefinedTypesAndVars * GlobalListOfDefinedTypeVar;

    GlobalDeclarations ListOfFinalGlobalDeclarations = constructListOfGlobalDeclarations(&(*GlobalDependencyList),
            &GlobalListOfDefinedTypeVar, definedGlobalTypesAndVariablesFile,
            tempDefinesFile, tempIncludeLibFile, AllDefinedFunctionsInDonor, GlobalFctDependOutput, TXLTemporaryFolder);

    fflush(stdout);

    * SkeletonDependenciesOnGlobals = generateGlobalDependencyListForSkeleton(FinalSkeletonSourceCode,
            definedGlobalTypesAndVariablesFile, ListOfFinalGlobalDeclarations, tempGlobalDeclarations, TXLTemporaryFolder);

    fflush(stdout);

    return ListOfFinalGlobalDeclarations;
}




GlobalDeclarations returnMarkerGlobalDeclarationALL(GlobalDeclarations currentGlobal, ListOfString * markers) {
    ListOfString * auxMarkers = markers;

    GlobalDeclarations neededGlobals;
    neededGlobals.normalDeclarations = NULL;
    neededGlobals.structUnionDeclarations = NULL;
    neededGlobals.typedefDeclarations = NULL;

    while (auxMarkers) {
        ListOfStructDefinitions * auxNormalDecl = currentGlobal.normalDeclarations;
        ListOfStructDefinitions * auxStructDecl = currentGlobal.structUnionDeclarations;
        ListOfStructDefinitions * auxTypedefDecl = currentGlobal.typedefDeclarations;

        while (auxNormalDecl) {

                addNewStructDefinition(&neededGlobals.normalDeclarations, auxNormalDecl->structDef,
                        auxNormalDecl->headerDefinition);
            auxNormalDecl = auxNormalDecl->next;


            /*
            if (strstr(auxNormalDecl->declaration, auxMarkers->el)) {
                addNewDeclarationWithHeaderDefinition(&neededGlobals.normalDeclarations, auxNormalDecl->declaration,
                        auxNormalDecl->headerDefinition);
            }
            auxNormalDecl = auxNormalDecl->next;
             */
        }

        while (auxStructDecl) {
                addNewStructDefinition(&neededGlobals.structUnionDeclarations, auxStructDecl->structDef,
                        auxStructDecl->headerDefinition);
            auxStructDecl = auxStructDecl->next;
        }

        while (auxTypedefDecl) {

                addNewStructDefinition(&neededGlobals.typedefDeclarations, auxTypedefDecl->structDef,
                        auxTypedefDecl->headerDefinition);
            auxTypedefDecl = auxTypedefDecl->next;

            /*
            if (strstr(auxTypedefDecl->el, auxMarkers->el)) {
                addNewString(&neededGlobals.typedefDeclarations, auxTypedefDecl->el);
            }
            auxTypedefDecl = auxTypedefDecl->next;
             */
        }

        auxMarkers = auxMarkers->next;
    }

    return neededGlobals;
}







GlobalDeclarations returnMarkerGlobalDeclaration(GlobalDeclarations currentGlobal, ListOfString * markers) {
    ListOfString * auxMarkers = markers;

    GlobalDeclarations neededGlobals;
    neededGlobals.normalDeclarations = NULL;
    neededGlobals.structUnionDeclarations = NULL;
    neededGlobals.typedefDeclarations = NULL;

    while (auxMarkers) {
        ListOfStructDefinitions * auxNormalDecl = currentGlobal.normalDeclarations;
        ListOfStructDefinitions * auxStructDecl = currentGlobal.structUnionDeclarations;
        ListOfStructDefinitions * auxTypedefDecl = currentGlobal.typedefDeclarations;

        while (auxNormalDecl) {

            if (strstr(auxNormalDecl->structDef->el, auxMarkers->el)) {
                addNewStructDefinition(&neededGlobals.normalDeclarations, auxNormalDecl->structDef,
                        auxNormalDecl->headerDefinition);
            }
            auxNormalDecl = auxNormalDecl->next;


            /*
            if (strstr(auxNormalDecl->declaration, auxMarkers->el)) {
                addNewDeclarationWithHeaderDefinition(&neededGlobals.normalDeclarations, auxNormalDecl->declaration,
                        auxNormalDecl->headerDefinition);
            }
            auxNormalDecl = auxNormalDecl->next;
             */
        }

        while (auxStructDecl) {
            if (strstr(auxStructDecl->structDef->el, auxMarkers->el)) {
                addNewStructDefinition(&neededGlobals.structUnionDeclarations, auxStructDecl->structDef,
                        auxStructDecl->headerDefinition);
            }
            auxStructDecl = auxStructDecl->next;
        }

        while (auxTypedefDecl) {

            if (strstr(auxTypedefDecl->structDef->el, auxMarkers->el)) {
                addNewStructDefinition(&neededGlobals.typedefDeclarations, auxTypedefDecl->structDef,
                        auxTypedefDecl->headerDefinition);
            }
            auxTypedefDecl = auxTypedefDecl->next;

            /*
            if (strstr(auxTypedefDecl->el, auxMarkers->el)) {
                addNewString(&neededGlobals.typedefDeclarations, auxTypedefDecl->el);
            }
            auxTypedefDecl = auxTypedefDecl->next;
             */
        }

        auxMarkers = auxMarkers->next;
    }

    return neededGlobals;
}

GlobalDeclarations extractNeededGlobalDeclarationsForSkeleton(ListOfString * SelectedMarkers,
        GlobalDeclarations ListOfFinalGlobalDeclarations,
        DependencyList * GlobalDependencyList, DependencyList * SkeletonDependenciesOnGlobals) {

    //Mapped declarations are globals which are mapped to the values in the host
    //these are not used for the moment
    ListOfString * MappedDeclarations = NULL;
    ListOfString * GlobalMarkersForTransplant = NULL;
    DependencyList * AllGlobalDependencies = NULL;
    addDependencyListToAnExistingOne(&AllGlobalDependencies, SkeletonDependenciesOnGlobals);
    addDependencyListToAnExistingOne(&AllGlobalDependencies, GlobalDependencyList);
    //first create the initial list of needed global markers, by the list of all
    //needed markers from the selected LOCs of the Skeleton
    ListOfString * auxSelectedMarkers = SelectedMarkers;
    while (auxSelectedMarkers) {
        ListOfString * neededDepsForCurrent = returnDependencyForAMarkerNewPointers(AllGlobalDependencies,
                auxSelectedMarkers->el);
        if (neededDepsForCurrent) {
            ListOfString * auxListString = neededDepsForCurrent;
            while (auxListString) {
                if (!searchStringInListOfStrings(MappedDeclarations, auxListString->el) &&
                        !searchStringInListOfStrings(GlobalMarkersForTransplant, auxListString->el)) {
                    addNewString(&GlobalMarkersForTransplant, auxListString->el);
                }
                auxListString = auxListString->next;
            }
            freeListOfStrings(&neededDepsForCurrent);
        }
        auxSelectedMarkers = auxSelectedMarkers->next;
    }
    ListOfString * auxNeededMarkers = GlobalMarkersForTransplant;
    //for all added line markers, add the one from dependency, if they are not mapped
    while (auxNeededMarkers) {
        //memory leak!!! returnDependencyForAMarker makes new one, while adding adds also
        ListOfString * neededDepsForCurrent = returnDependencyForAMarkerNewPointers(AllGlobalDependencies,
                auxNeededMarkers->el);
        if (neededDepsForCurrent) {
            ListOfString * auxListString = neededDepsForCurrent;
            while (auxListString) {
                if (!searchStringInListOfStrings(GlobalMarkersForTransplant, auxListString->el)) {
                    addNewString(&GlobalMarkersForTransplant, auxListString->el);
                }
                auxListString = auxListString->next;
            }
            freeListOfStrings(&neededDepsForCurrent);
        }
        auxNeededMarkers = auxNeededMarkers->next;
    }

    //temporary add all the global declarations!!!

    // TODO: CHECK HERE
    GlobalDeclarations NeededGlobalDeclarations = returnMarkerGlobalDeclaration(ListOfFinalGlobalDeclarations,
            GlobalMarkersForTransplant);

    //printListOfGlobalDeclarations(NeededGlobalDeclarations, "/home/alex/workspace/testGLOBALSnewSource.out", 
    // "/home/alex/workspace/testGLOBALSnewHeader.out");


    return NeededGlobalDeclarations;

}

ListOfStructDefinitions * reverseListOfStructDefinitions(ListOfStructDefinitions * originalList) {
    ListOfStructDefinitions * newListOfStructDefinitions = NULL;
    while (originalList) {
        addNewStructDefinitionAtBeggining(& newListOfStructDefinitions, originalList->structDef, originalList->headerDefinition);
        originalList = originalList->next;
    }
    return newListOfStructDefinitions;
}



#define ENUM_MARKER "/* ENUM_DECL_"
//to be changhed!!!
#define TYPEDEF_MARKER_TEMP "typedef"
#define STRUCT_ARRAY_HEADER_DECLARATION "/* STRUCT_WITH_ARRAY */" 

void printListOfGlobalDeclarations(GlobalDeclarations ListOfDecl, char * outputFileSurce,
        char * outputFileHeader) {
    FILE * fout = fopen(outputFileSurce, "w");
    FILE * foutHeader = fopen(outputFileHeader, "w");


    //first time, print in the source file the struct declarations with marker, from the header!!!

    ListOfStructDefinitions * reversedNormalDecl = reverseListOfStructDefinitions(ListOfDecl.normalDeclarations);

    ListOfStructDefinitions * auxNormalDeclJustHeader = reversedNormalDecl;
    while (auxNormalDeclJustHeader) {
        ListOfString * auxNormalElements = auxNormalDeclJustHeader->structDef;
        if (!strstr(auxNormalElements->el, ENUM_MARKER)) {
            //nothing here, after structs!
        } else {
            while (auxNormalElements) {
                fprintf(foutHeader, "%s", auxNormalElements->el);
                auxNormalElements = auxNormalElements->next;
            }
        }

        ///ListOfString * auxNormalElementsForStructArray = auxNormalDeclJustHeader->headerDefinition;
        if (auxNormalDeclJustHeader->headerDefinition && strstr(auxNormalDeclJustHeader->headerDefinition, STRUCT_ARRAY_HEADER_DECLARATION)) {
            //fprintf(fout, "%s", auxNormalDeclJustHeader->headerDefinition);

        }


        auxNormalDeclJustHeader = auxNormalDeclJustHeader->next;
    }




    ListOfStructDefinitions * reversedStructDecl = reverseListOfStructDefinitions(ListOfDecl.structUnionDeclarations);

    ListOfStructDefinitions * auxStructDecl = reversedStructDecl;
    while (auxStructDecl) {
        ListOfString * auxStructElements = auxStructDecl->structDef;


        if (!strstr(auxStructElements->el, TYPEDEF_MARKER_TEMP)) {
            while (auxStructElements) {
                fprintf(fout, "%s", auxStructElements->el);
                auxStructElements = auxStructElements->next;
            }



            fprintf(fout, "\n\n");
            if (auxStructDecl->headerDefinition) {
                fprintf(foutHeader, "%s\n\n", auxStructDecl->headerDefinition);

            }
        } else {
            while (auxStructElements) {
                fprintf(foutHeader, "%s", auxStructElements->el);
                auxStructElements = auxStructElements->next;
            }



            fprintf(fout, "\n\n");
        }
        auxStructDecl = auxStructDecl->next;
    }



    ListOfStructDefinitions * reversedTypeDefDecl = reverseListOfStructDefinitions(ListOfDecl.typedefDeclarations);
    ListOfStructDefinitions * auxTypeDecl = reversedTypeDefDecl;
    while (auxTypeDecl) {
        ListOfString * auxStructElements = auxTypeDecl->structDef;
        while (auxStructElements) {
            fprintf(foutHeader, "%s", auxStructElements->el);
            auxStructElements = auxStructElements->next;
        }

        //fprintf(foutHeader, "%s\n\n", auxTypeDecl->el);
        auxTypeDecl = auxTypeDecl->next;
    }





    ListOfStructDefinitions * auxNormalDecl = reversedNormalDecl;
    while (auxNormalDecl) {
        ListOfString * auxNormalElements = auxNormalDecl->structDef;
        if (!strstr(auxNormalElements->el, ENUM_MARKER)) {
            //fprintf(fout, "%s\n\n", auxNormalElements->el);
            while (auxNormalElements) {
                fprintf(fout, "%s", auxNormalElements->el);
                auxNormalElements = auxNormalElements->next;
            }
            fprintf(fout, "\n\n");
            if (auxNormalDecl->headerDefinition && !strstr(auxNormalDecl->headerDefinition, STRUCT_ARRAY_HEADER_DECLARATION)) {
                fprintf(foutHeader, "%s\n\n", auxNormalDecl->headerDefinition);
            }
        } else {
            //nothing here, it was added before the structs!
        }

        /*
        if (!strstr(auxNormalDecl->declaration, ENUM_MARKER)) {
            fprintf(fout, "%s\n\n", auxNormalDecl->declaration);
        } else {
            fprintf(foutHeader, "%s\n\n", auxNormalDecl->declaration);
        }
        if (auxNormalDecl->headerDefinition) {
            fprintf(foutHeader, "%s\n\n", auxNormalDecl->headerDefinition);
        }
         */
        auxNormalDecl = auxNormalDecl->next;
    }










    fclose(fout);
    fclose(foutHeader);
}

void addGlobalDeclarationListToExistingOne(GlobalDeclarations * head, GlobalDeclarations toBeAdded) {
    addNewListOfStructToAnExistingOne(&((*head).normalDeclarations),
            toBeAdded.normalDeclarations);
    addNewListOfStructToAnExistingOne(&((*head).typedefDeclarations), toBeAdded.typedefDeclarations);
    addNewListOfStructToAnExistingOne(&((*head).structUnionDeclarations), toBeAdded.structUnionDeclarations);
}

int searchForGlobalMarker(GlobalDeclarations ListOfGlobalDeclarations, char * marker) {
    ListOfStructDefinitions * auxNormalDecl = ListOfGlobalDeclarations.normalDeclarations;
    ListOfStructDefinitions * auxStructDecl = ListOfGlobalDeclarations.structUnionDeclarations;
    ListOfStructDefinitions * auxTypedefDecl = ListOfGlobalDeclarations.typedefDeclarations;
    while (auxNormalDecl) {
        if (strstr(auxNormalDecl->structDef->el, marker)) {
            return 1;
        }
        auxNormalDecl = auxNormalDecl->next;
    }
    while (auxStructDecl) {
        if (strstr(auxStructDecl->structDef->el, marker)) {
            return 1;
        }
        auxStructDecl = auxStructDecl->next;
    }
    while (auxTypedefDecl) {
        if (strstr(auxTypedefDecl->structDef->el, marker)) {
            return 1;
        }
        auxTypedefDecl = auxTypedefDecl->next;
    }
    return 0;
}









#define DEPENDENCIES_FOR_GLOBALS_MARKER "DEPENDENCIES_FOR_GLOBAL"

CallDependency * extractNeededFunctionsForGlobals(GlobalDeclarations ListOfGlobalDeclarations,
        char * GlobalsDependenciesOnFunctFileName) {

    FILE * fin = fopen(GlobalsDependenciesOnFunctFileName, "r");
    ListOfString * finalNeededFunctionsAsMarkers = NULL;

    CallDependency * calledFunctionsByGlobals = (CallDependency *) malloc(1 * sizeof (CallDependency));
    calledFunctionsByGlobals->LOCMarker = (char *) malloc(500 * sizeof (char));
    strcpy(calledFunctionsByGlobals->LOCMarker, "/* GLOBAL_CALLED */");

    ListOfFunctionSourceFile * listOfFctForGlobals = NULL;


    char * line;
    line = (char *) malloc(10000 * sizeof (char));

    char * StreamEnd = fgets(line, 4999, fin);

    while (StreamEnd != NULL) {

        char * declMarker = (char *) malloc(1000 * sizeof (char));
        fgets(line, 4999, fin);
        line[strlen(line) - 1] = '\0';
        strcpy(declMarker, line);
        if (searchForGlobalMarker(ListOfGlobalDeclarations, declMarker)) {
            while ((StreamEnd = fgets(line, 4999, fin)) != NULL && !strstr(line, DEPENDENCIES_FOR_GLOBALS_MARKER)) {
                char * functionName = (char *) malloc(1000 * sizeof (char));
                sscanf(line, "%s", functionName);

                //eliminate the function name, the space and the quotes after it
                line += strlen(functionName) + 2;
                //eliminate the final quotes and the new line symbol
                line[strlen(line) - 2] = 0;
                char * sourceFile = (char *) malloc(2000 * sizeof (char));
                strcpy(sourceFile, line);
                char * FunctionMarker = (char *) malloc(500 * sizeof (char));
                //sprintf(FunctionMarker, "/* %s */", functionName);
                if (!searchStringInListOfStrings(finalNeededFunctionsAsMarkers, FunctionMarker)) {
                    addNewFunctionSourceFile(&listOfFctForGlobals, functionName, sourceFile);
                }
            }
        } else {
            while ((StreamEnd = fgets(line, 4999, fin)) != NULL && !strstr(line, DEPENDENCIES_FOR_GLOBALS_MARKER)) {
            }
        }
    }

    calledFunctionsByGlobals->calledFunctions = listOfFctForGlobals;
    calledFunctionsByGlobals->next = NULL;

    return calledFunctionsByGlobals;
}
