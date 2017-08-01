#include "FunctionCallDependency.h"
#include "ListOfString.h"
#include "GenTrans.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void freeListOfCallDependency(CallDependency ** head) {
    CallDependency * aux;
    while (* head) {
        aux = *head;
        * head = (*head) -> next;
        freeListOfFunctionSourceFile(&(aux->calledFunctions));
        aux->calledFunctions = NULL;
        free(aux->LOCMarker);
        aux->LOCMarker = NULL;
        aux->next = NULL;
        free(aux);
        aux = NULL;
    }
    (* head) = NULL; //just in case...
}

CallDependency * searchCallDependencyForALOC(CallDependency * head, char * locName) {
    CallDependency * p;
    p = head;
    int s = 0;
    while (p && !s) {
        if (!strcmp(p->LOCMarker, locName)) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

void addNewLOCMarker(CallDependency ** head, char * locName) {
    CallDependency * newEl = (CallDependency *) malloc(1 * sizeof (CallDependency));
    newEl->LOCMarker = (char *) malloc((strlen(locName) + 1) * sizeof (char));
    strcpy(newEl->LOCMarker, locName);
    newEl->calledFunctions = NULL;
    newEl->next = NULL;
    if (* head == NULL) {
        * head = newEl;
    } else {
        CallDependency * p;
        p = *head;
        while (p ->next) {
            p = p->next;
        }
        p ->next = newEl;
    }
}

int addCalledFunction(CallDependency ** head, char * locName, char * calledFunction, char * functionSourceFile) {
    CallDependency * p;
    p = *head;
    int s = 0;
    while (p -> next && s == 0) {
        if (!strcmp(locName, p->LOCMarker)) {
            s = 1;
        } else {
            p = p->next;
        }
    }
    if (!strcmp(p->LOCMarker, locName)) {
        s = 1;
    }
    if (s == 0) {
        return 0;
    } else {
        addNewFunctionSourceFile(&(p->calledFunctions), calledFunction, functionSourceFile);
        return 1;
    }
}

CallDependency * readCallDependencyFromFile(char * inputFile) {

    FILE * fin = fopen(inputFile, "r");

    char * line;
    line = (char *) malloc(800 * sizeof (char));
    CallDependency * ListOfCallDependencies = NULL;
    char * currentName;
    currentName = (char*) malloc(800 * sizeof (char));
    while (fgets(line, 400, fin) != NULL) {


        char * locMarker;
        locMarker = (char *) malloc(500 * sizeof (char));

        strcpy(locMarker, line);
        locMarker[strlen(locMarker) - 1] = 0;
        
        addNewLOCMarker(&ListOfCallDependencies, locMarker);
        
        while (strcmp(fgets(line, 400, fin), LINE_END)) {
            char * calledFunctionsString;
            calledFunctionsString = (char *) malloc(1000 * sizeof (char));
            char * calledFunctionSourceFile;
            calledFunctionSourceFile = (char *) malloc(3000 * sizeof (char));

            sscanf(line, "%s %s", calledFunctionsString, calledFunctionSourceFile);
            
            
            addCalledFunction(&ListOfCallDependencies, locMarker, calledFunctionsString, calledFunctionSourceFile);
            
            free(calledFunctionSourceFile);
            free(calledFunctionsString);
        }
    }
    free(line);
    free(currentName);
    fclose(fin);
    return ListOfCallDependencies;
}
