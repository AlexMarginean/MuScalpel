/* 
 * File:   folderContent.h
 * Author: alex
 *
 * Created on 18 June 2014, 23:47
 */

#ifndef FOLDERCONTENT_H
#define	FOLDERCONTENT_H

#ifdef	__cplusplus
extern "C" {
#endif

void readFilesFromDir (char * path, FILE * fout, char endChar);

int isInListOfNeededFiles(char ** listOfFiles, int length, char * currentFile);


#ifdef	__cplusplus
}
#endif

#endif	/* FOLDERCONTENT_H */

