/* 
 * File:   ListOfFunctionSourceFile.h
 * Author: alex
 *
 * Created on 03 July 2014, 19:12
 */

#ifndef LISTOFFUNCTIONSOURCEFILE_H
#define	LISTOFFUNCTIONSOURCEFILE_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct listOfFunctionSourceFile {
        char * functionName;
        char * sourceFile;
        struct listOfFunctionSourceFile * next;
    };

    typedef struct listOfFunctionSourceFile ListOfFunctionSourceFile;

    void freeListOfFunctionSourceFile(ListOfFunctionSourceFile ** head);
    
    void addNewFunctionSourceFile(ListOfFunctionSourceFile ** head, char * functionName, char * sourceFile);
    
    int removeFunctionSourceFile(ListOfFunctionSourceFile ** head, char * functionName);
    
    void printAListOfFunctionSourceFile(ListOfFunctionSourceFile * head, char * outputFile);
    
    int searchForFunction(ListOfFunctionSourceFile * head, char * searchedFunctionName);
    
    int countElementsInListOfFunctionsSourceFile(ListOfFunctionSourceFile * head);


#ifdef	__cplusplus
}
#endif

#endif	/* LISTOFFUNCTIONSOURCEFILE_H */

