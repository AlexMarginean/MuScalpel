#include "ListOfString.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void freeListOfStrings(ListOfString ** head) {
    ListOfString * aux;

    while (* head) {
        aux = *head;
        * head = (*head) -> next;
        free(aux->el);
        aux->el = NULL;
        aux->next = NULL;
        free(aux);
        aux = NULL;
    }
    (* head) = NULL; //just in case...
}

void addNewString(ListOfString ** head, char * string) {
    ListOfString * newEl = (ListOfString *) malloc(1 * sizeof (ListOfString));
    newEl->el = (char *) malloc((strlen(string) + 1) * sizeof (char *));
    strcpy(newEl->el, string);
    newEl->next = NULL;
    if (* head == NULL) {
        * head = newEl;
    } else {
        ListOfString * p;
        p = *head;
        while (p ->next) {
            p = p->next;
        }
        p ->next = newEl;
    }
}

ListOfString * copyListOfString(ListOfString * head) {
    ListOfString * newHead = NULL;
    ListOfString * pHead = NULL;
    pHead = head;
    if (head == NULL) {
        return NULL;
    }
    //the last element is the bracket from function call
    while (pHead -> next) {
        addNewString(&newHead, pHead->el);
        pHead = pHead->next;
    }
    return newHead;
}

ListOfString * copyCompleteListOfString(ListOfString * head) {
    ListOfString * newHead = NULL;
    ListOfString * pHead = NULL;
    pHead = head;
    if (head == NULL) {
        return NULL;
    }
    while (pHead) {
        addNewString(&newHead, pHead->el);
        pHead = pHead->next;
    }
    return newHead;
}

ListOfString * copyListOfStringNoLastElement(ListOfString * head) {
    ListOfString * newHead = NULL;
    ListOfString * pHead = NULL;
    pHead = head;
    //the last element is the bracket from function call
    if (pHead->next) {
        while (pHead -> next -> next) {
            addNewString(&newHead, pHead->el);
            pHead = pHead->next;
        }
    }
    return newHead;
}

void removeFirstEl(ListOfString **head) {
    if (!(*head)) {
        return;
    } else if (!(*head)->next) {
        free((*head)->el);
        free(*head);
        (*head) = NULL;
    } else {
        ListOfString * aux;
        aux = *head;
        (*head) = (*head)->next;
        free(aux->el);
        free(aux);
        aux = NULL;
    }
}

void removeLastString(ListOfString ** head) {
    ListOfString * p;
    p = *head;
    if (!p) {
        return;
    }
    if (!p->next) {
        free(p->el);
        free(p);
        p = NULL;
        *head = NULL;
    }
    while (p->next->next) {
        p = p->next;
    }
    free(p->next->el);
    free(p->next);
    p->next = NULL;
}

int removeString(ListOfString ** head, char * string) {
    ListOfString * p;
    p = *head;
    int s = 0;
    if (!strcmp(p->el, string)) {
        *head = p->next;
        free(p->el);
        free(p);
        p = NULL;
        s = 1;
    } else {
        while (p->next && s == 0) {
            if (!strcmp(p->next->el, string)) {
                ListOfString * aux;
                aux = p->next;
                p -> next = p -> next -> next;
                free(aux->el);
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

int removeStringLabels(ListOfString ** head, char * string) {
    ListOfString * p;
    p = *head;
    int s = 0;

    //char * fixedElement = (char *) malloc (strlen(p->el) * sizeof(char));

    //strcpy(fixedElement, p->el + 1);
    //fixedElement[strlen(fixedElement) - 2 ] = 0;

    if (strstr(string,  p->el)) {
        *head = p->next;
        free(p->el);
       	free(p);
        p = NULL;
        s = 1;
    } else {
        while (p->next && s == 0) {
        	//fixedElement = (char *) malloc (strlen(p->next->el) * sizeof(char));
        	//strcpy(fixedElement, p->next->el + 1);
        	//fixedElement[strlen(fixedElement) - 2 ] = 0;
            if (strstr(string, p->next->el)) {
                ListOfString * aux;
                aux = p->next;
                p -> next = p -> next -> next;
                free(aux->el);
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

void replaceOneLineWithAListKeepPenultimate(ListOfString ** head, ListOfString * newList, char * line) {
    ListOfString * newCopy;
    newCopy = copyListOfString(newList);
    if (!strcmp((*head)->el, line)) {
        ListOfString * aux;
        aux = *head;
        ListOfString * aux2;
        aux2 = newCopy;
        while (aux2->next) {
            aux2 = aux2->next;
        }
        aux2->next = (*head)->next;
        (*head) = newCopy;
        free(aux->el);
        free(aux);
        aux = NULL;
    } else {
        ListOfString * p;
        p = *head;
        int s = 0;
        while (p->next && !s) {
            if (!strcmp(p->next->el, line)) {
                s = 1;
            } else {
                p = p->next;
            }
        }
        if (!s) {
            return;
        } else {
            ListOfString * aux;
            aux = p->next;

            ListOfString * aux2;
            aux2 = newCopy;
            if (aux2) {
                while (aux2->next) {
                    aux2 = aux2->next;
                }
                aux2->next = p->next->next;
                p->next = newCopy;
                free(aux->el);
                free(aux);
                aux = NULL;
            } else {
                ListOfString * auxRemoveString = p->next;
                p->next = p->next->next;
                free(auxRemoveString->el);
                free(auxRemoveString);
            }

        }
    }
}

void replaceOneLineWithAList(ListOfString ** head, ListOfString * newList, char * line) {
    ListOfString * newCopy;
    newCopy = copyListOfStringNoLastElement(newList);
    if (!strcmp((*head)->el, line)) {
        ListOfString * aux;
        aux = *head;
        ListOfString * aux2;
        aux2 = newCopy;
        while (aux2->next) {
            aux2 = aux2->next;
        }
        aux2->next = (*head)->next;
        (*head) = newCopy;
        free(aux->el);
        free(aux);
        aux = NULL;
    } else {
        ListOfString * p;
        p = *head;
        int s = 0;
        while (p->next && !s) {
            if (!strcmp(p->next->el, line)) {
                s = 1;
            } else {
                p = p->next;
            }
        }
        if (!s) {
            return;
        } else {
            ListOfString * aux;
            aux = p->next;

            ListOfString * aux2;
            aux2 = newCopy;
            while (aux2->next) {
                aux2 = aux2->next;
            }
            aux2->next = p->next->next;
            p->next = newCopy;
            //free(aux->el);
            //free(aux);
            aux = NULL;
        }
    }
}

void printAListOfStrings(ListOfString * head, char * outputFile) {
    ListOfString * p;
    p = head;
    FILE * fout = fopen(outputFile, "w");
    while (p) {
        fprintf(fout, "%s", p->el);
        p = p->next;
    }
    fclose(fout);
}

void printAListOfStringsNL(ListOfString * head, char * outputFile) {
    ListOfString * p;
    p = head;
    FILE * fout = fopen(outputFile, "w");
    while (p) {
        fprintf(fout, "%s\n", p->el);
        p = p->next;
    }
    fclose(fout);
}

void printAListOfStringsWithSpaces(ListOfString * head, char * outputFile) {
    ListOfString * p;
    p = head;
    FILE * fout = fopen(outputFile, "w");
    while (p) {
        fprintf(fout, "%s ", p->el);
        p = p->next;
    }
    fclose(fout);
}

ListOfString * readListOfStringFromFile(char * input) {
    ListOfString * head = NULL;

    FILE * fin = fopen(input, "r");
    char * line;
    line = (char *) malloc(3000 * sizeof (char));

    while (fgets(line, 2999, fin) != NULL) {
        addNewString(&head, line);
    }

    free(line);
    fclose(fin);
    return head;
}

ListOfString * readListOfStringFromFileAsWords(char * input) {
    ListOfString * head = NULL;

    FILE * fin = fopen(input, "r");
    char * word;
    word = (char *) malloc(3000 * sizeof (char));

    if(fin) {
        while (fscanf(fin, "%s", word) != EOF) {
            addNewString(&head, word);
        }
    }

    free(word);
    fclose(fin);
    return head;
}

int searchStringInListOfStrings(ListOfString * head, char * searchedEl) {
    while (head) {
        if (!strcmp(head->el, searchedEl)) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

int countElementsInListOfString(ListOfString * head) {
    int count = 0;
    while (head) {
        head = head->next;
        count++;
    }
    return count;
}

int countElementsInListOfStringNotAnElement(ListOfString * head, char * element) {
    int count = 0;
    while (head) {
        if (strcmp(head->el, element)) {
            count++;
        }
        head = head->next;

    }
    return count;
}

void addListOfStringToExistingOneUniqueEl(ListOfString ** destination, ListOfString * source) {
    ListOfString * aux = source;

    while (aux) {
        if (!searchStringInListOfStrings((*destination), aux->el)) {
            addNewString(&(*destination), aux->el);
        }
        aux = aux->next;
    }
}

char * nthElementInListString(ListOfString * head, int n) {
    while (n > 0 && head) {
        head = head->next;
        n--;
    }
    if (n == 0) {
        char * selectedMappingCandidate = NULL;
        selectedMappingCandidate = (char *) malloc((strlen(head->el) + 1) * sizeof (char));

        strcpy(selectedMappingCandidate, head->el);

        return selectedMappingCandidate;
    } else {
        return NULL;
    }
}

ListOfString * returnListOfStringFromMarkersFile(char * inputFile) {
    ListOfString * TempListOfStrings = NULL;
    FILE * fileOfMarkers = fopen(inputFile, "r");
    char * allMarkers;
    allMarkers = (char *) malloc(10000 * sizeof (char));
    if (fgets(allMarkers, 10000, fileOfMarkers) == NULL) {
        return NULL;
    }
    char * token;
    char * tokenNoBeginSpace;
    tokenNoBeginSpace = (char *) malloc(500 * sizeof (char));
    token = strtok(allMarkers, ",");
    if (token) {
        if (token[strlen(token) - 1 ] == '\n') {
            token[strlen(token) - 1 ] = '\0';
        }
        addNewString(&TempListOfStrings, token);
    }
    while (token) {
        token = strtok(NULL, ",");
        if (token) {

            strcpy(tokenNoBeginSpace, token + 1);
            if (tokenNoBeginSpace[strlen(tokenNoBeginSpace) - 1 ] == '\n') {
                tokenNoBeginSpace[strlen(tokenNoBeginSpace) - 1 ] = '\0';
            }
            if (!searchStringInListOfStrings(TempListOfStrings, tokenNoBeginSpace)) {
                addNewString(&TempListOfStrings, tokenNoBeginSpace);
            }
        }
    }
    free(allMarkers);
    fclose(fileOfMarkers);
    return TempListOfStrings;
}

char * nthElementInListStringExceptingOneString(ListOfString * head, int n, char * exception) {
    ListOfString * currentSelected = NULL;
    
    while (n >= 0 && head) {
        if (strcmp(head->el, exception)) {
            n--;
            currentSelected = head;
        }
        head = head->next;
    }
    if (n == -1) {
        char * selectedMappingCandidate = NULL;
        selectedMappingCandidate = (char *) malloc((strlen(currentSelected->el) + 1) * sizeof (char));

        strcpy(selectedMappingCandidate, currentSelected->el);

        return selectedMappingCandidate;
    } else {
        return NULL;
    }
}
