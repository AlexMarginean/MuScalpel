//
//  main.c
//  FolderContent
//
//  Created by Marginean Alexandru on 09/03/14.
//  Copyright (c) 2014 Marginean Alexandru. All rights reserved.
//

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "folderContent.h"
#include "Utils.h"

char isUserRule(rulename)
char rulename[256];
{
    return ((strcmp(rulename, "getid") == 0) || (strcmp(rulename, "sum") == 0));
}

void readFilesFromDir(char * path, FILE * fout, char endChar) {
    DIR *d;
    struct dirent *dir;
    char * newPath;
    newPath = (char*) malloc(500 * sizeof (char));
    strcpy(newPath, path);
    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == 4 && strcmp(".", dir->d_name) && strcmp("..", dir->d_name)) {
                
                
                //void readDirContent(char * path, char** listOfNeededHeaderFiles, int length, char * outputFolder)
                strcat(newPath, dir->d_name);
                strcat(newPath, "/");
                readFilesFromDir(newPath, fout, endChar);
            } else if (dir->d_name[strlen(dir->d_name) - 1] == endChar) {
                fprintf(fout, "\"%s%s\" ", path, dir->d_name);
                //printf("\"%s%s\" \n", path, dir->d_name);
            }
        }
        closedir(d);
    }
    free(newPath);
}