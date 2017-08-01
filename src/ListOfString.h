/* 
 * File:   ListOfString.h
 * Author: alex
 *
 * Created on 27 June 2014, 18:41
 */

#ifndef LISTOFSTRING_H
#define	LISTOFSTRING_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct listOfString {
        char * el;
        struct listOfString * next;
    };

    typedef struct listOfString ListOfString;

    void freeListOfStrings(ListOfString ** head);

    void addNewString(ListOfString ** head, char * string);

    ListOfString * copyListOfString(ListOfString * head);

    ListOfString * copyListOfStringNoLastElement(ListOfString * head);

    void removeFirstEl(ListOfString **head);

    void removeLastString(ListOfString ** head);

    int removeString(ListOfString ** head, char * string);

    int removeStringLabels(ListOfString ** head, char * string);

    void replaceOneLineWithAListKeepPenultimate(ListOfString ** head, ListOfString * newList, char * line);

    void replaceOneLineWithAList(ListOfString ** head, ListOfString * newList, char * line);

    void printAListOfStrings(ListOfString * head, char * outputFile);

    ListOfString * readListOfStringFromFile(char * input);

    ListOfString * readListOfStringFromFileAsWords(char * input);

    int searchStringInListOfStrings(ListOfString * head, char * searchedEl);

    int countElementsInListOfString(ListOfString * head);

    ListOfString * copyCompleteListOfString(ListOfString * head);

    void addListOfStringToExistingOneUniqueEl(ListOfString ** destination, ListOfString * source);

    char * nthElementInListString(ListOfString * head, int n);

    ListOfString * returnListOfStringFromMarkersFile(char * inputFile);
    
    void printAListOfStringsWithSpaces(ListOfString * head, char * outputFile);
    
    char * nthElementInListStringExceptingOneString(ListOfString * head, int n, char * exception);
    
    int countElementsInListOfStringNotAnElement(ListOfString * head, char * element);

    void printAListOfStringsNL(ListOfString * head, char * outputFile);

#ifdef	__cplusplus
}
#endif

#endif	/* LISTOFSTRING_H */

