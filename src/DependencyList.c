#include "DependencyList.h"
#include "GenTrans.h"
#include "IndividualInstantiator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void freeDependencyList(DependencyList ** head) {
    DependencyList * aux;
    while (* head) {
        aux = *head;
        * head = (*head) -> next;
        freeListOfStrings(&(aux->ListOfDependencies));
        aux->ListOfDependencies = NULL;
        free(aux->Statement);
        aux->Statement = NULL;
        aux->next = NULL;
        free(aux);
        aux = NULL;
    }
    (* head) = NULL; //just in case...
}

DependencyList * searchDependenciesForAnElement(DependencyList * head, char * statement) {
    DependencyList * p;
    p = head;
    int s = 0;
    while (p && !s) {
        if (!strcmp(p->Statement, statement)) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

void addNewStatement(DependencyList ** head, char * statement) {
    DependencyList * newEl = (DependencyList *) malloc(1 * sizeof (DependencyList));
    newEl->Statement = (char *) malloc(1000 * sizeof (char));
    strcpy(newEl->Statement, statement);
    newEl->ListOfDependencies = NULL;
    newEl->next = NULL;
    if (* head == NULL) {
        * head = newEl;
    } else {
        DependencyList * p;
        p = *head;
        while (p ->next) {
            p = p->next;
        }
        p ->next = newEl;
    }
}

int addNewDependency(DependencyList ** head, char * statement, char * declaration) {
    DependencyList * p;
    p = *head;
    int s = 0;
    while (p -> next && s == 0) {
        if (!strcmp(statement, p->Statement)) {
            s = 1;
        } else {
            p = p->next;
        }
    }
    if (!strcmp(p->Statement, statement)) {
        s = 1;
    }
    if (s == 0) {
        return 0;
    } else {
        addNewString(&(p->ListOfDependencies), declaration);
        return 1;
    }
}

void printADependencyList(char * outputFile, DependencyList * head) {
    FILE * fout = fopen(outputFile, "w");
    DependencyList * p;
    p = head;
    while (p) {
        ListOfString * aux;
        aux = p->ListOfDependencies;
        fprintf(fout, "FUNCTION_NAME : \n%s\n", p->Statement);
        fprintf(fout, "DECLARATION_LIST : \n");
        while (aux) {
            fprintf(fout, "%s\n", aux->el);
            aux = aux->next;
        }
        fprintf(fout, "%s", LINE_END);
        p = p ->next;
    }
    fclose(fout);
}

DependencyList * copyDependencyList (DependencyList * initial){
	DependencyList * newDepList = NULL;
	DependencyList * aux = initial;
	while(aux){

		addNewStatement(&newDepList, aux->Statement);
		ListOfString * auxListOfStr = aux->ListOfDependencies;
		while(auxListOfStr){

			addNewDependency(&newDepList, aux->Statement, auxListOfStr->el);
			auxListOfStr = auxListOfStr->next;
		}

		aux = aux->next;
	}


	return newDepList;
}

DependencyList * readDependencyListFromFile(char * inputFile) {
    FILE * fin = fopen(inputFile, "r");
    char * line;
    //char * initialLine;
    line = (char *) malloc(10000 * sizeof (char));
    //initialLine = line;
    DependencyList * ListOfDeclarationsDependencies = NULL;
    char * currentStatement;
    currentStatement = (char*) malloc(10000 * sizeof (char));
    while (fgets(line, 9999, fin) != NULL) {
        fgets(line, 9999, fin);
        //sscanf(line, "%s", currentStatement);
        strtok(line, "\n");
        strcpy(currentStatement, line);
        addNewStatement(&ListOfDeclarationsDependencies, currentStatement);
        int s = 0;
        fgets(line, 9999, fin);
        while (s == 0 && fgets(line, 9999, fin) != NULL) {
            if (!strcmp(line, LINE_END)) {
                s = 1;
            } else {
                strtok(line, "\n");
                addNewDependency(&ListOfDeclarationsDependencies, currentStatement, line);
            }
        }


        //line = initialLine;
    }
    free(line);
    free(currentStatement);
    fclose(fin);
    return ListOfDeclarationsDependencies;
}

ListOfString * returnDependencyForAMarkerNewPointers(DependencyList * head, char * marker) {
    ListOfString * dependencyListForMarker = NULL;
    int signal = 0;
    while (head && !signal) {
        if (!strcmp(head->Statement, marker)) {
            signal = 1;
        } else {
            head = head->next;
        }
    }
    if (signal) {
        dependencyListForMarker = copyCompleteListOfString(head->ListOfDependencies);
    }
    return dependencyListForMarker;
}

void addDependencyListToAnExistingOne(DependencyList ** destination, DependencyList * source) {
    DependencyList * auxSource = source;
    while (auxSource) {
        addNewStatement(&(*destination), auxSource->Statement);

        ListOfString * auxDep = auxSource->ListOfDependencies;

        while (auxDep) {
            addNewDependency(&(*destination), auxSource->Statement, auxDep->el);
            auxDep = auxDep->next;
        }
        auxSource = auxSource->next;
    }
}

void createListOfDependenciesForDonorAbstractMappings(char * graftWithHostVarInstantiated, char * neededMarkersFile,
        char * outputFile, char * TXLProgsPath) {
    char * commandTXL;
    commandTXL = (char *) malloc(1000 * sizeof (char));
    sprintf(commandTXL, "%sGenerateDeclarationDependenciesForDonorVariablesInAbstractMappings.x %s %s %s %s",
            TXLProgsPath, graftWithHostVarInstantiated, neededMarkersFile,
            outputFile, ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! sGenerateDeclarationDependenciesForDonorVariablesInAbstractMappings.x\n");
    }
    free(commandTXL);
}

DependencyList * dependencyListForDeclWithAbstractValues(Mapping * ListOfMappingsFromCurrentDonor,
        ListOfString * FinalSkeletonSourceCode, ListOfString * ListOfMarkersForAbstractVariables,
		char * TXLTemporaryFolder) {
    char * tempMappingsFileAbstractNeededDependnecies;
    tempMappingsFileAbstractNeededDependnecies = (char *) malloc(500 * sizeof (char));
    sprintf(tempMappingsFileAbstractNeededDependnecies, "%stemp_mapping_file_abstract_dependencies.out", TXLTemporaryFolder);

    char * tempSkeletonFile;
    tempSkeletonFile = (char *) malloc(500 * sizeof (char));
    sprintf(tempSkeletonFile, "%stemp_skeleton_for_abstract_dependencies.out", TXLTemporaryFolder);

    char * tempSkeletonFileWithAllIdsFromGraftInstantiated;
    tempSkeletonFileWithAllIdsFromGraftInstantiated = (char *) malloc(500 * sizeof (char));
    sprintf(tempSkeletonFileWithAllIdsFromGraftInstantiated, "%stemp_skeleton_with_all_ids_from_donor.out", TXLTemporaryFolder);

    char * tempFileWithMarkersForAbstractIDs;
    tempFileWithMarkersForAbstractIDs = (char *) malloc(500 * sizeof (char));
    sprintf(tempFileWithMarkersForAbstractIDs, "%stemp_list_of_markers_for_abstract_ids.out", TXLTemporaryFolder);


    printAListOfMappingsStringlit(ListOfMappingsFromCurrentDonor, tempMappingsFileAbstractNeededDependnecies);
    printAListOfStrings(FinalSkeletonSourceCode, tempSkeletonFile);


    ListOfString * skeletonInstantiatedWithAbstractFromDonor = instantiateCurrentIndividualInSkeleton
            (tempSkeletonFile, tempMappingsFileAbstractNeededDependnecies, TXLTemporaryFolder);

    printAListOfStrings(skeletonInstantiatedWithAbstractFromDonor, tempSkeletonFileWithAllIdsFromGraftInstantiated);



    printAListOfStringsWithSpaces(ListOfMarkersForAbstractVariables, tempFileWithMarkersForAbstractIDs);


    char * tempOutputFileNeededDependenciesForAbstract;
    tempOutputFileNeededDependenciesForAbstract = (char *) malloc(500 * sizeof (char));
    sprintf(tempOutputFileNeededDependenciesForAbstract, "%stemp_dependencies_for_abstract.out", TXLTemporaryFolder);

    createListOfDependenciesForDonorAbstractMappings(tempSkeletonFileWithAllIdsFromGraftInstantiated,
            tempFileWithMarkersForAbstractIDs, tempOutputFileNeededDependenciesForAbstract, TXLToolsPath);

    DependencyList * dependencyListForAbstract = NULL;

    dependencyListForAbstract = readDependencyListFromFile(tempOutputFileNeededDependenciesForAbstract);

    return dependencyListForAbstract;
}
