/* 
 * File:   Utils.h
 * Author: alex
 *
 * Created on 27 June 2014, 18:56
 */

#ifndef UTILS_H
#define	UTILS_H

#ifdef	__cplusplus
extern "C" {
#endif

    int isInListOfNeededFiles(char ** listOfFiles, int length, char * currentFile);

    void readDirContent(char * path, char** listOfNeededHeaderFiles, int length, char * outputFolder);

    void copyNeededFiles(char * listOfNeededIncludeFiles, char * pathToDonor, char * transplantWorkingFolder,
            char * locationToCopyFiles);

    void prettyPrintSourceCode(char * TXLProgsPath, char * tempSkeleton, char * finalSkeleton);
    
    void prettyPrintSourceCodeNoAnnotations(char * TXLProgsPath, char * source, char * destination);
   

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */

