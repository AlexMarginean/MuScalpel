/* 
 * File:   main.c
 * Author: alex
 *
 * Created on 09 June 2014, 17:49
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GenTrans.h"
#include "reverseCallGraph.h"
#include "Utils.h"

//#include "reverseCallGraph.h"

char ** completeCallGraphs(char ** CallGraph, int n, int * newL) {
	char ** NewCallGraph = NULL;
	NewCallGraph = (char**) malloc(100000 * sizeof(char*));
	for (int i = 0; i < 100000; i++) {
		NewCallGraph[i] = (char*) malloc(4000 * sizeof(char));
	}
	int NoOfSpaces = 0;
	int k = 1;
	strcpy(NewCallGraph[0], CallGraph[0]);
	strcpy(NewCallGraph[k], "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	k = 2;
	int firstNoSpaces = 0;
	while (CallGraph[1][firstNoSpaces] == ' ') {
		firstNoSpaces++;
	}
	for (int i = 1; i < n; i++) {
		int count = 0;
		while (CallGraph[i][count] == ' ') {
			count++;
		}

		if (count > NoOfSpaces) {
			NoOfSpaces = count;
			strcpy(NewCallGraph[k], CallGraph[i]);
			k++;
		} else {
			//empty line for new graph
			strcpy(NewCallGraph[k], "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
			k++;
			int newPos = 1;
			int countForOldLines;
			countForOldLines = 0;
			int positionInNewGraph = 1;
			int tt = 1;
			while (tt < k) {
				int newCountSpaces = 0;
				while (NewCallGraph[tt][newCountSpaces] == ' ') {
					newCountSpaces++;
				}
				if (newCountSpaces == firstNoSpaces) {
					positionInNewGraph = tt;
				}
				tt++;
			}
			while (NewCallGraph[positionInNewGraph][countForOldLines] == ' ') {
				countForOldLines++;
			}
			newPos = positionInNewGraph;
			while (countForOldLines < count && newPos < k) {
				strcpy(NewCallGraph[k], NewCallGraph[newPos]);
				newPos++;
				countForOldLines = 0;
				while (NewCallGraph[newPos][countForOldLines] == ' ') {
					countForOldLines++;
				}
				k++;
			}
			NoOfSpaces = count;
			strcpy(NewCallGraph[k], CallGraph[i]);
			k++;

		}
	}
	*newL = k;
	return NewCallGraph;
}

void callCFlowReversed(char * donorSourceFiles, char * transplantWorkingFolder,
		char * outputFileFullPath) {
	char * gcov = " cflow --reverse";
	char * restOfParams = "-i _ --omit-arguments *.c >";
	char gcovCommand[400];
	sprintf(gcovCommand, "cd %s \n %s %s %s", donorSourceFiles, gcov,
			restOfParams, outputFileFullPath);

	char * copyFile = (char *) malloc(2000 * sizeof(char));
	sprintf(copyFile, "cp %s%s %s%s", donorSourceFiles, outputFileFullPath,
			TXLTemporaryFolderMainThread, outputFileFullPath);
	//printf("%s \n", gcovCommand);
	int status2 = system(gcovCommand);
	if (status2 != 0) {
		printf("ERROR");
	}
	system(copyFile);
}

void transformReverseCallGraph(char * TXLPath, char * donorSourceFiles,
		char * reversedCflowOutput, char * modifiedFunctionLOCsOutput,
		char * SkeletonInstantiateMappings, char * HostSymbolTable,
		char * CurrentExistingMappings, char * allCalledFunctionsList,
		char * outputFileLabels) {
	char * command;
	command = (char *) malloc(400 * sizeof(char));
	sprintf(command, "%sreadReverseCallGraph.x", TXLPath);
	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));

	char * txlIfDefProg;
	txlIfDefProg = (char *) malloc(1000 * sizeof(char));
	sprintf(txlIfDefProg, "%sifdef.x", TXLPath);

	sprintf(commandTXL, "%s %s %s %s %s %s %s %s %s %s %s", command,
			reversedCflowOutput, donorSourceFiles, modifiedFunctionLOCsOutput,
			SkeletonInstantiateMappings, HostSymbolTable,
			CurrentExistingMappings, txlIfDefProg, allCalledFunctionsList,
			outputFileLabels, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR read reverse call graph!\n");
		printf("Command: %s\n", commandTXL);
		fflush(stdout);
		exit(1);
		return;
	}
	free(txlIfDefProg);
	free(commandTXL);
	free(command);

    if(is_demo){
        char * cmdCpDemoFiles = (char *) malloc (1000 * sizeof(char));
        sprintf(cmdCpDemoFiles, "cp %s %sOrganSymbolTable.out && cp %s %sVeinSourceCode.c",
                HostSymbolTable, demoPath, modifiedFunctionLOCsOutput, demoPath);
        system(cmdCpDemoFiles);

#ifdef DETAILED_DEMO
		printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the organ symbol table: %sOrganSymbolTable.out; and the vein source code: %sVeinSourceCode.c . \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath, demoPath);
        getchar();
#endif
    }
}

//fix file names!!!

void generateCalledFunctionsList(char * tempFolder, char * graftMethod,
		char * donorSourceFiles, char * currentCoreTransplantFunction,
		char * resultFileFullPath) {
	char * gcovMainMethodMarker = "--main";
	char gcovMainMethod[400];
	sprintf(gcovMainMethod, "%s %s", gcovMainMethodMarker, graftMethod);
	char * outputFile = "outputCFLOWCalledFunctions.out";
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
		char * cpDemoFiles = (char *) malloc (1000 * sizeof(char));
		sprintf(cpDemoFiles, "cp %s %sAllVeinCallGraph.out", outputFileFullPath, demoPath);
		system(cpDemoFiles);


#ifdef DETAILED_DEMO
		printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the list of all veins (call graph): %sAllVeinCallGraph.out . \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath);
        getchar();
#endif
	}

	char **listOfFiles = NULL;
	listOfFiles = (char**) malloc(60000 * sizeof(char*));
	for (int i = 0; i < 59999; i++) {
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


	char **listOfSourceFiles = NULL;
	listOfSourceFiles = (char**) malloc(60000 * sizeof(char*));
	for (int i = 0; i < 59999; i++) {
		listOfSourceFiles[i] = (char*) malloc(400 * sizeof(char));
	}
	//char listOfSourceFiles[3000][200];
	int countFiles = 0;

	//char listOfCalledFunctions[3000][200];
	char **listOfCalledFunctions = NULL;
	listOfCalledFunctions = (char**) malloc(60000 * sizeof(char*));
	for (int i = 0; i < 59999; i++) {
		listOfCalledFunctions[i] = (char*) malloc(400 * sizeof(char));
	}

	int coreFunctionFound = 0;

	int shouldAddFunction = 1;
	int currentUnnededFunctionSpaceLength = 0;

	while (fgets(line, 400, fin) != NULL && !coreFunctionFound) {
		int currentSpaceLength = 0;
		char * tempLine = (char *) malloc((strlen(line) + 5) * sizeof(char));
		strcpy(tempLine, line);
		int countSpaces = 0;
		while (tempLine[countSpaces] == ' ') {
			currentSpaceLength++;
			countSpaces++;
		}

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

			if (!searchStringInListOfStrings(ListOfCoreFunctions, currentFct)) {
				if (shouldAddFunction) {

					sprintf(formattedCurrentFunction, "\"%s\" \"%s%s\"\n",
							currentFct, donorSourceFiles, line2);
					sprintf(listOfSourceFiles[countFiles], "@@%s@@ ", line2);
					sprintf(listOfCalledFunctions[countFiles], "%s",
							currentFct);
					countFiles++;
				} else {
					if (currentSpaceLength
							<= currentUnnededFunctionSpaceLength) {
						shouldAddFunction = 1;
						sprintf(formattedCurrentFunction, "\"%s\" \"%s%s\"\n",
								currentFct, donorSourceFiles, line2);
						sprintf(listOfSourceFiles[countFiles], "@@%s@@ ",
								line2);
						sprintf(listOfCalledFunctions[countFiles], "%s",
								currentFct);
						countFiles++;
					}
				}
			} else {
				if (shouldAddFunction) {
					shouldAddFunction = 0;
					currentUnnededFunctionSpaceLength = currentSpaceLength;
				}
			}
			if (!strcmp(currentFct, GraftFunctionName)) {
				coreFunctionFound = 1;
			}

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
	for (int i = 0; i < countFiles; i++) {
		fprintf(fout, "%s", listOfSourceFiles[i]);
	}
	fprintf(fout, "\n______________________________\n\n");

	fprintf(fout, "^^%s^^ %s\n", listOfCalledFunctions[0],
			listOfSourceFiles[0]);

	for (int i = 0; i < countFiles; i++) {
		fprintf(fout, "%s source_file: %s\n", listOfCalledFunctions[i],
				listOfSourceFiles[i]);
	}

	fprintf(fout, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	fclose(fout);

}

void generateReverseCallGraph(char * TXLPath, char * donorSourceFiles,
		char * transplantWorkingFolder, char * reversedCflowOutput,
		char * SkeletonInstantiateMappings, char * HostSymbolTable,
		char * CurrentExistingMappings, char * temporaryFolder,
		char * hostTransplantSourceLocation,
		char * currentCoreTransplantFunction,
		ListOfString ** listOfReverseCallGraph, char * outputFileLabels) {

	char * resultFileFullPath;
	resultFileFullPath = (char *) malloc(500 * sizeof(char));

	sprintf(resultFileFullPath, "%sgcovParsedResultsCalledFunctions.out",
			temporaryFolder);
	generateCalledFunctionsList(temporaryFolder, "main", donorSourceFiles,
			currentCoreTransplantFunction, resultFileFullPath);
	fflush(stdout);

	char * outputFileName;
	char * outputFile; // = "outputReeversedCFLOW.out";
	outputFileName = (char *) malloc(500 * sizeof(char));
	outputFile = (char *) malloc(500 * sizeof(char));

	sprintf(outputFileName, "%stemp_reverse_CFLOWs.out", temporaryFolder);
	sprintf(outputFile, "%stemp_output_reversed_CFLOW.out", temporaryFolder);

	callCFlowReversed(donorSourceFiles, transplantWorkingFolder,
			"temp_reverse_CFLOWs.out");
	FILE * fin = fopen(outputFileName, "r");
	char **listOfFiles = NULL;
	char ** fileLocations = NULL;
	fileLocations = (char**) malloc(100000 * sizeof(char*));
	for (int i = 0; i < 100000; i++) {
		fileLocations[i] = (char*) malloc(4000 * sizeof(char));
	}
	int posLocation = 0;
	listOfFiles = (char**) malloc(100000 * sizeof(char*));
	for (int i = 0; i < 100000; i++) {
		listOfFiles[i] = (char*) malloc(4000 * sizeof(char));
	}
	char * currentFct;
	currentFct = (char *) malloc(4000 * sizeof(char));
	char * fctWithSpaces;
	fctWithSpaces = (char *) malloc(5000 * sizeof(char));
	char * line;
	line = (char *) malloc(4000 * sizeof(char));
	char * line2;
	char * line3;
	char * auxLine3;
	line2 = (char *) malloc(4000 * sizeof(char));
	line3 = (char *) malloc(4000 * sizeof(char));
	int s = 0;
	int i = 0;

	auxLine3 = line3;
	while (fgets(line, 400, fin) != NULL && !s) {
		if (line[0] != ' ') {
			sscanf(line, "%s", currentFct);
			if (currentFct[strlen(currentFct) - 1] == ':') {
				currentFct[strlen(currentFct) - 3] = 0;
			} else {
				currentFct[strlen(currentFct) - 2] = 0;
			}
			//FUNCTION NAME AS PARAMETER!!!!!!
			if (strcmp(currentFct, currentCoreTransplantFunction) == 0) {
				sscanf(line, "%s", currentFct);
				if (currentFct[strlen(currentFct) - 1] == ':') {
					currentFct[strlen(currentFct) - 3] = 0;
				} else {
					currentFct[strlen(currentFct) - 2] = 0;
				}

				char * searchedStringFirst;
				searchedStringFirst = (char *) malloc(4000 * sizeof(char));
				strcpy(searchedStringFirst, " at ");
				line3 = strstr(line, searchedStringFirst);
				line3 = line3 + 4;
				strcpy(line2, "");

				int posForLocationFirst = 0;
				while (line3[posForLocationFirst] != ':') {
					line2[posForLocationFirst] = line3[posForLocationFirst];
					posForLocationFirst++;
				}
				line2[posForLocationFirst] = 0;
				strcat(line2, "@@ ");
				char * SourceFileNeededFirst;
				SourceFileNeededFirst = (char*) malloc(4000 * (sizeof(char)));
				strcpy(SourceFileNeededFirst, "@@");
				strcat(SourceFileNeededFirst, line2);

				char * formattedCurrentFunction;
				formattedCurrentFunction = (char *) malloc(500 * sizeof(char));

				sprintf(formattedCurrentFunction, "^^%s^^ %s \n", currentFct,
						SourceFileNeededFirst);

				strcpy(listOfFiles[i], formattedCurrentFunction);
				i++;
				strcpy(fileLocations[posLocation], SourceFileNeededFirst);
				posLocation++;
				while (fgets(line, 400, fin) != NULL && line[0] == ' ') {
					int spacesCount = 0;
					while (line[spacesCount] == ' ') {
						spacesCount++;
					}
					strcpy(fctWithSpaces, "");
					for (int jj = 0; jj < spacesCount; jj++) {
						strcat(fctWithSpaces, " ");
					}
					sscanf(line, "%s", currentFct);
					char * searchedString;
					searchedString = (char *) malloc(4000 * sizeof(char));
					strcpy(searchedString, " at ");
					line3 = strstr(line, searchedString);
					line3 = line3 + 4;
					strcpy(line2, "");
					int posForLocation = 0;
					while (line3[posForLocation] != ':') {
						line2[posForLocation] = line3[posForLocation];
						posForLocation++;
					}
					line2[posForLocation] = 0;
					strcat(line2, "@@ ");
					char * SourceFileNeeded;
					SourceFileNeeded = (char*) malloc(4000 * (sizeof(char)));
					strcpy(SourceFileNeeded, "@@");
					strcat(SourceFileNeeded, line2);
					strcpy(line2, SourceFileNeeded);
					if (!isInListOfNeededFiles(fileLocations, posLocation,
							line2)) {
						strcpy(fileLocations[posLocation], line2);
						posLocation++;
					}
					if (currentFct[strlen(currentFct) - 1] == ':') {
						currentFct[strlen(currentFct) - 3] = 0;
					} else {
						currentFct[strlen(currentFct) - 2] = 0;
					}
					addNewString(listOfReverseCallGraph, currentFct);
					strcat(currentFct, " source_file: ");
					strcat(currentFct, line2);
					strcat(currentFct, "\n");
					strcat(fctWithSpaces, currentFct);
					strcpy(listOfFiles[i], fctWithSpaces);
					i++;
					free(SourceFileNeeded);
					free(searchedString);
				}
				s = 1;

				free(SourceFileNeededFirst);
				free(searchedStringFirst);
				free(formattedCurrentFunction);
			}
		}
	}

	fflush(stdout);

	FILE *fout = fopen(outputFile, "w");
	char ** newCallGraph = NULL;
	int newLen;
	newCallGraph = completeCallGraphs(listOfFiles, i, &newLen);
	for (int j = 0; j < posLocation; j++) {
		fprintf(fout, "%s", fileLocations[j]);
	}
	fprintf(fout, "%s", "\n");
	fprintf(fout, "%s", "______________________________\n\n");
	newCallGraph[0][strlen(newCallGraph[0]) - 1] = 0;
	fprintf(fout, "%s\n", newCallGraph[0]);
	for (int j = newLen - 1; j > 0; j--) {
		fprintf(fout, "%s", newCallGraph[j]);
	}
	fclose(fin);
	fclose(fout);


	if(is_demo){
		char * cmdCpDemoFiles = (char *) malloc (1000 * sizeof(char));
		sprintf(cmdCpDemoFiles, "cp %s %sSelectedVein.out", outputFile, demoPath);
		system(cmdCpDemoFiles);

#ifdef DETAILED_DEMO
		printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the selected vein: %sSelectedVein.out . \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath);
        getchar();
#endif
	}

	transformReverseCallGraph(TXLPath, donorSourceFiles, outputFile,
			reversedCflowOutput, SkeletonInstantiateMappings, HostSymbolTable,
			CurrentExistingMappings, resultFileFullPath, outputFileLabels);

	fflush(stdout);

	//free memory
	for (int i = 0; i < 100000; i++) {
		free(newCallGraph[i]);
	}
	free(newCallGraph);

	//fileLocations
	for (int i = 0; i < 100000; i++) {
		free(fileLocations[i]);
	}
	free(fileLocations);

	//listOfFiles
	for (int i = 0; i < 100000; i++) {
		free(listOfFiles[i]);
	}
	free(listOfFiles);

	free(currentFct);
	free(fctWithSpaces);
	free(line);
	free(line2);
	free(auxLine3);
	free(outputFile);
	free(outputFileName);
}
