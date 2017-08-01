/* 
 * File:   main.c
 * Author: alex
 *
 * Created on 26 May 2014, 22:45
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "folderContent.h"
#include "coreFunctionExtractor.h"
#include "GenTrans.h"
#include "Utils.h"

//dsajdkasnjdsa dnjkas

void extractCoreFunctionality(char * graftMethod, char * graftFileFullPath,
		char * donorSourceFiles, char * transplantWorkingFolder,
		char * txlProgsPath, char * TXLTemp, char * tempFolder,
		char * headerOutputFile, char * sourceOutputFile) {

	char * sourceFiles;
	sourceFiles = (char *) malloc(500 * sizeof(char));
	sprintf(sourceFiles, "%ssourceContentFile.out", tempFolder);
	char * headerFiles;
	headerFiles = (char *) malloc(500 * sizeof(char));
	sprintf(headerFiles, "%sheaderContentFile.out", tempFolder);
	char * resultFileFullPath;
	resultFileFullPath = (char *) malloc(500 * sizeof(char));

	sprintf(resultFileFullPath, "%sgcovParsedResults.out", tempFolder);
	char * gcovMainMethodMarker = "--main";
	char gcovMainMethod[400];
	sprintf(gcovMainMethod, "%s %s", gcovMainMethodMarker, graftMethod);
	char * outputFile = "outputCFLOW.out";
	char * gcov = " cflow";
	char * restOfParams = "-i _ --omit-arguments *.c >";
	char * command;
	command = (char *) malloc(2000 * sizeof(char *));
	char gcovCommand[1000];
	char outputFileFullPath[1000];
	sprintf(outputFileFullPath, "%s%s", donorSourceFiles, outputFile);
	//sprintf(graftFileFullPath, "%s%s", donorSourceFiles, graftFile);
	sprintf(gcovCommand, "cd %s \n %s %s %s %s", donorSourceFiles, gcov,
			gcovMainMethod, restOfParams, outputFile);
	//("%s \n", gcovCommand);
	//printf("%s \n", outputFileFullPath);
	fflush(stdout);
	int status1 = system(gcovCommand);
	if (status1 != 0) {
		printf("ERROR");
	}
	if(is_demo){
        char * cmdCopyDemoFiles = (char*) malloc (1000 * sizeof(char));
        sprintf(cmdCopyDemoFiles, "cp %s %sCoreOrganCallGraph.out", outputFileFullPath, demoPath);
        system(cmdCopyDemoFiles);

#ifdef DETAILED_DEMO
        printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the core organ callgraph: %sCoreOrganCallGraph.out. \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath);
        getchar();
#endif
    }
	char **listOfFiles = NULL;
	listOfFiles = (char**) malloc(30000 * sizeof(char*));
	for (int i = 0; i < 29999; i++) {
		listOfFiles[i] = (char*) malloc(400 * sizeof(char));
	}
	char * currentFct;
	currentFct = (char *) malloc(400 * sizeof(char));
	char * line;
	line = (char *) malloc(400 * sizeof(char));
	FILE * fin = fopen(outputFileFullPath, "r");
	int n = 0;

	char * line2;
	char * line3;
	line2 = (char *) malloc(4000 * sizeof(char));
	line3 = (char *) malloc(4000 * sizeof(char));

	while (fgets(line, 400, fin) != NULL) {

		sscanf(line, "%s", currentFct);
		if (currentFct[strlen(currentFct) - 1] == ':') {
			currentFct[strlen(currentFct) - 3] = 0;
		} else {
			currentFct[strlen(currentFct) - 2] = 0;
		}

		char * formattedCurrentFunction;
		formattedCurrentFunction = (char *) malloc(500 * sizeof(char));

		char * searchedStringFirst;
		searchedStringFirst = (char *) malloc(4000 * sizeof(char));
		strcpy(searchedStringFirst, " at ");
		line3 = strstr(line, searchedStringFirst);
		if (line3) {
			line3 = line3 + 4;
			strcpy(line2, "");

			int posForLocationFirst = 0;
			while (line3[posForLocationFirst] != ':') {
				line2[posForLocationFirst] = line3[posForLocationFirst];
				posForLocationFirst++;
			}
			line2[posForLocationFirst] = 0;

			sprintf(formattedCurrentFunction, "\"%s\" \"%s%s\"\n", currentFct,
					donorSourceFiles, line2);

		} else {
			sprintf(formattedCurrentFunction, "\"%s\" \"NOT_INTERNAL\"\n",
					currentFct);
		}

		if (!isInListOfNeededFiles(listOfFiles, n, formattedCurrentFunction)) {
			strcpy(listOfFiles[n], formattedCurrentFunction);
			n++;
		}
	}
	FILE *fout = fopen(resultFileFullPath, "w");
	for (int i = 0; i < n; i++) {
		fprintf(fout, "%s", listOfFiles[i]);
	}
	fclose(fout);
	FILE * sourceFileName = fopen(sourceFiles, "w");
	FILE * headerFileName = fopen(headerFiles, "w");
	readFilesFromDir(donorSourceFiles, sourceFileName, 'c');
	readFilesFromDir(donorSourceFiles, headerFileName, 'h');
	fclose(sourceFileName);
	fclose(headerFileName);
	char * listOfNeededFilesForCoreGraftFunction;
	listOfNeededFilesForCoreGraftFunction = (char *) malloc(500 * sizeof(char));
	strcpy(listOfNeededFilesForCoreGraftFunction, TXLTemp);
	strcat(listOfNeededFilesForCoreGraftFunction,
			"temp_list_of_header_files.out");
	//first is just a program that must be correct according to the grammar; it is not used, is needed just for
	//txl program to run!!!
	char * originalHOST;
	char * ifDefProgramPath;

	if (strcmp(TXLToolsPath, "./")) {
		ifDefProgramPath = (char *) malloc(
				(strlen(TXLToolsPath) + 10) * sizeof(char));
		sprintf(ifDefProgramPath, "%sifdef.x", TXLToolsPath);
	} else {
		ifDefProgramPath = (char *) malloc(10 * sizeof(char));
		sprintf(ifDefProgramPath, "./ifdef.x");
	}
	originalHOST = transplantHostInputFile;

	sprintf(command, "%smultiplFiles.x %s %s %s %s %s %s %s %s %s%s %s",
			TXLToolsPath, originalHOST, graftFileFullPath, headerOutputFile,
			sourceOutputFile, resultFileFullPath, sourceFiles, headerFiles,
			listOfNeededFilesForCoreGraftFunction, TXLToolsPath, ifDefProgramPath,
			ConsoleNullOutput);

	int status2 = system(command);
	if (status2 != 0) {
		printf("ERROR multiplFiles !!!!!\n");
	}
	//copyNeededFiles(listOfNeededFilesForCoreGraftFunction, donorSourceFiles, transplantWorkingFolder,
	//locationToCopyFiles);
	fclose(fin);

	if(is_demo){
		char * cmdCopyDemoFiles = (char*) malloc (1000 * sizeof(char));
		sprintf(cmdCopyDemoFiles, "cp %s %sCoreOrganSourceCode.c && cp %s %sCoreOrganHeaderFile.h",
            sourceOutputFile, demoPath, headerOutputFile, demoPath);
		system(cmdCopyDemoFiles);


#ifdef DETAILED_DEMO
		printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the core organ source code: %sCoreOrganSourceCode.c; and the core organ header file: %sCoreOrganHeaderFile.h . \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath, demoPath);
        getchar();
#endif


	}

	//free memory
	for (int i = 0; i < 29999; i++) {
		free(listOfFiles[i]);
	}

	free(listOfFiles);
	free(currentFct);
	free(listOfNeededFilesForCoreGraftFunction);
	free(line);
}
