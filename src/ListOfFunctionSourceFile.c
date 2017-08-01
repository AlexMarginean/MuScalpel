#include "ListOfFunctionSourceFile.h"

#include <stdio.h>
#include <string.h>

void freeListOfFunctionSourceFile(ListOfFunctionSourceFile ** head) {
    ListOfFunctionSourceFile * aux;

    while (* head) {
        aux = *head;
        * head = (*head) -> next;
        free(aux->functionName);
        free(aux->sourceFile);
        aux->functionName = NULL;
        aux->sourceFile = NULL;
        aux->next = NULL;
        free(aux);
        aux = NULL;
    }
    (* head) = NULL; //just in case...
}

void addNewFunctionSourceFile(ListOfFunctionSourceFile ** head, char * functionName, char * sourceFile) {
    ListOfFunctionSourceFile * newEl = (ListOfFunctionSourceFile *) malloc(1 * sizeof (ListOfFunctionSourceFile));
    newEl->functionName = (char *) malloc((strlen(functionName) + 1) * sizeof (char *));
    newEl->sourceFile = (char *) malloc((strlen(sourceFile) + 1) * sizeof (char *));
    strcpy(newEl->functionName, functionName);
    strcpy(newEl->sourceFile, sourceFile);
    newEl->next = NULL;
    if (* head == NULL) {
        * head = newEl;
    } else {
        ListOfFunctionSourceFile * p;
        p = *head;
        while (p ->next) {
            p = p->next;
        }
        p ->next = newEl;
    }
}

int removeFunctionSourceFile(ListOfFunctionSourceFile ** head, char * functionName) {
    ListOfFunctionSourceFile * p;
    p = *head;
    int s = 0;
    if (!strcmp(p->functionName, functionName)) {
        *head = p->next;
        free(p->functionName);
        free(p->sourceFile);
        p->functionName = NULL;
        p->sourceFile = NULL;
        free(p);
        p = NULL;
        s = 1;
    } else {
        while (p->next && s == 0) {
            if (!strcmp(p->next->functionName, functionName)) {
                ListOfFunctionSourceFile * aux;
                aux = p->next;
                p -> next = p -> next -> next;
                free(aux->sourceFile);
                free(aux->functionName);
                aux->sourceFile = NULL;
                aux->functionName = NULL;
                free(aux);
                aux = NULL;
                s = 1;
            } else {
                p = p->next;
            }

        }
    }
    return s;
}

void printAListOfFunctionSourceFile(ListOfFunctionSourceFile * head, char * outputFile) {
    ListOfFunctionSourceFile * p;
    p = head;
    FILE * fout = fopen(outputFile, "w");
    while (p) {
        fprintf(fout, "%s %s\n", p->functionName, p->sourceFile);
        p = p->next;
    }
    fclose(fout);
}

int searchForFunction(ListOfFunctionSourceFile * head, char * searchedFunctionName) {
    while (head) {
        if (!strcmp(head->functionName, searchedFunctionName)) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

int countElementsInListOfFunctionsSourceFile(ListOfFunctionSourceFile * head) {
    int count = 0;
    while (head) {
        head = head->next;
        count++;
    }
    return count;
}


int searchFunctionInListOfFunctionsSourceFile (ListOfFunctionSourceFile * head, char * searchedFunctionName){
    while(head){
        if(!strcmp(head->functionName, searchedFunctionName)){
            return 1;
        }
        head = head->next;
    }
    return 0;
}
