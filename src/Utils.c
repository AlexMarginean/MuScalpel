#include "Utils.h"
#include "GenTrans.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define __USE_XOPEN_EXTENDED


char * removeArraySizeInString(char * originalString) {
    char * newString = (char *) malloc((strlen(originalString) + 1) * sizeof (char));
    strcpy(newString, originalString);

    int signal = 1;

    while (signal) {
        signal = 0;
        char * position = strstr(newString, "[");
        if (position) {
            if (* (position + 1) != ']') {
                signal = 1;

                while (* (position + 1) != ']') {
                    strcpy(position + 1, position + 2);
                    // position + 1 = position + 2;
                }

            }
        }

    }
    return newString;
}

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return NULL;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}


int isInListOfNeededFiles(char ** listOfFiles, int length, char * currentFile) {
    for (int i = 0; i < length; i++) {
        if (!strcmp(listOfFiles[i], currentFile)) {
            return 1;
        }
    }
    return 0;
}

//this functions will read the content of the input directory and will paste the needed files
//into OriginalHeader dir

void readDirContent(char * path, char** listOfNeededHeaderFiles, int length, char * outputFolder) {

    DIR *d;
    struct dirent *dir;
    char * newPath;
    newPath = (char*) malloc(500 * sizeof (char));
    strcpy(newPath, path);
    strcat(newPath, "/");
    unsigned char c;
    int source, destination;
    struct stat statbuff;

    d = opendir(path);
    chdir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            //printf("%s \n", dir->d_name);
            if (dir->d_type == 4 && strcmp(".", dir->d_name) && strcmp("..", dir->d_name)) {
                readDirContent(strcat(newPath, dir->d_name), listOfNeededHeaderFiles, length, outputFolder);
            } else if (isInListOfNeededFiles(listOfNeededHeaderFiles, length, dir->d_name)) {
                // printf("%s\n", dir->d_name);

                if (lstat(dir->d_name, &statbuff) < 0) {
                    return;
                }
                if ((source = open(dir->d_name, O_RDONLY)) < 0) {
                    return;
                }

                char * destinationPath;
                destinationPath = (char*) malloc(255 * sizeof (char));

                strcpy(destinationPath, outputFolder);
                strcat(destinationPath, dir->d_name);

                if ((destination = open(destinationPath, O_CREAT | O_TRUNC | O_WRONLY, (statbuff.st_mode & S_IRWXU) | (statbuff.st_mode & S_IRWXG) | (statbuff.st_mode & S_IRWXO))) < 0) {
                    return;
                }
                while (read(source, &c, 1) == 1) {
                    write(destination, &c, 1);
                }
                close(destination);
                close(source);

                free(destinationPath);
            }
        }

        closedir(d);
    }
    free(newPath);
}

//this function will copy the needed header files into OriginalHeaders

void copyNeededFiles(char * listOfNeededIncludeFiles, char * pathToDonor, char * transplantWorkingFolder,
        char * locationToCopyFiles) {
    
    
    /*
    
    
    
    FILE *fin = fopen(listOfNeededIncludeFiles, "r");
    char **listOfFiles = NULL;
    listOfFiles = (char**) malloc(5000 * sizeof (char*));
    for (int i = 0; i < 5000; i++) {
        listOfFiles[i] = (char*) malloc(5000 * sizeof (char));
    }
    char * line;
    line = (char *) malloc(5000 * sizeof (char));
    char * auxLine;
    auxLine = line;
    int count = 0;
    while (fscanf(fin, "%s", line) != EOF) {
        line++;
        line[strlen(line) - 1] = 0;
        line = strstr(line, "/") + 1;
        strcpy(listOfFiles[count], line);
        count++;
    }
    free(auxLine);
    fclose(fin);
    readDirContent(pathToDonor, listOfFiles, count, locationToCopyFiles);
    //free memory
    for (int i = 0; i < 5000; i++) {
        free(listOfFiles[i]);
    }
    free(listOfFiles);
     * 
     */ 
}


void prettyPrintSourceCode(char * TXLProgsPath, char * tempSkeleton, char * finalSkeleton) {
    char * commandTXL;
    commandTXL = (char *) malloc(500 * sizeof (char));
    sprintf(commandTXL, "%sPrettyPrintWithAnnotations.x %s %s %s", TXLProgsPath, tempSkeleton, finalSkeleton,
            ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! sPrettyPrintWithAnnotations.x\n");
    }
    free(commandTXL);
}

void prettyPrintSourceCodeNoAnnotations(char * TXLProgsPath, char * source, char * destination) {
    char * commandTXL;
    commandTXL = (char *) malloc(500 * sizeof (char));
    sprintf(commandTXL, "%sprettyPrintNoAnnotations.x %s %s %s", TXLProgsPath, source, destination,
            ConsoleNullOutput);
    int status = system(commandTXL);
    if (status != 0) {
        printf("ERROR! sprettyPrintNoAnnotations.x\n");
    }
    free(commandTXL);
}
