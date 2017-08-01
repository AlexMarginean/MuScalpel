/* 
 * File:   main.c
 * Author: alex
 *
 * Created on 12 June 2014, 04:20
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GenTrans.h"
#include "Skeleton.h"
#include "SkeletonMapping.h"
#include "ListOfString.h"
#include "FunctionElement.h"
#include "DependencyList.h"

/*
 * 
 */

int abstractIDNumber = 0;

void solveAbstractValues(SkeletonMapping * mappingsForSkeletonInstantiation,
		Mapping * localExistingMappings, char * callerName,
		SkeletonMapping * currentExistingMappings, char * currentCalled,
		Mapping ** newListOfAbstractMappings, Mapping ** newAbstractValues,
		SymbolTable * GraftSymbolTable,
		SymbolTable ** ChanghedAbstractIDSymbolTable) {
	SkeletonMapping * currentSkeletonMappings;
	currentSkeletonMappings = returnMappingsForFunction( //HERE SHOULD BE ADDED!!!
			mappingsForSkeletonInstantiation, callerName);
	SkeletonMapping * currentAbstractMappings;
	currentAbstractMappings = returnMappingsForFunction(currentExistingMappings,
			currentCalled);
	Mapping * aux = currentAbstractMappings->mappings;

	Mapping * auxLocalMappings = localExistingMappings;

	while (auxLocalMappings) {

		addNewMapping(&currentSkeletonMappings->mappings,
				auxLocalMappings->source, auxLocalMappings->destination);
		auxLocalMappings = auxLocalMappings->next;
	}

	while (aux) {
		Mapping * skeletonMappingAux = NULL;
		if (currentSkeletonMappings) {
			skeletonMappingAux = currentSkeletonMappings->mappings;
		}
		int signal = 0;
		while (skeletonMappingAux && !signal) {
			if (!strcmp(skeletonMappingAux->source, aux->destination)) {
				signal = 1;
			} else {
				skeletonMappingAux = skeletonMappingAux->next;
			}
		}
		char * newAbstractName;
		newAbstractName = (char *) malloc(500 * sizeof(char));
		//sprintf(newAbstractName, "called_from_%s_%s", callerName, aux->source);
		sprintf(newAbstractName, "$ABSTRACT_%d", abstractIDNumber);
		abstractIDNumber++;
		if (signal) {
			addNewMapping(&(*newListOfAbstractMappings), newAbstractName,
					skeletonMappingAux->destination);
		} else {
			addNewMapping(&(*newListOfAbstractMappings), newAbstractName,
					aux->destination);
		}
		addNewMapping(&(*newAbstractValues), aux->source, newAbstractName);

		char * typeOfID = searchForASymbolTableEntry(GraftSymbolTable,
				aux->source);
		if (typeOfID) {
			addNewVariableDeclaration(ChanghedAbstractIDSymbolTable, typeOfID,
					newAbstractName);
		}

		free(newAbstractName);
		aux = aux->next;
	}
}

int UniqueGlobalID = 0;

void callUniqueLocalDeclGenerator(char * tempFunctionFile,
		char * outputTempFile, char * initialFunctionName, char * uniqueId) {
	//TODO: Fix this!
	//call uniqueLocalDeclarationGeneralizer

	// Get just the ID of the current called function.

	char * token;
	char * state;
	token = strtok_r(uniqueId, initialFunctionName, &state);

	char * commandInstantiate;
	commandInstantiate = (char *) malloc(400 * sizeof(char));
	sprintf(commandInstantiate, "%suniqueLocalVarDecls.x", TXLToolsPath);

	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));
	sprintf(commandTXL, "%s %s %s %s %s", commandInstantiate, tempFunctionFile,
			outputTempFile, token, ConsoleNullOutput);
	//UniqueGlobalID ++;

	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! uniqueLocalVarDecls.x \n");
		return;
	}

}

ListOfString * solveCalledIDs(FunctionElement * head,
		SkeletonMapping * currentMappings, char * mappedFunctionID,
		char * command, char * initialFunctionFile, char * initialMappingsFile,
		char * skeletonFctInstantiate, SkeletonMapping * mappingsForSkeleton,
		SkeletonMapping * ListOfExistingMappings,
		Mapping ** FinalListOfAbstractMappings, SymbolTable * GraftSymbolTable,
		SymbolTable ** ChanghedAbstractIDSymbolTable, char * TXLTemporaryFolder) {
	SkeletonMapping * current;
	Mapping * newAbstractValues = NULL;
	ListOfString * aux = NULL;
	aux = head->LOCs;

	FILE * inputForInstantiation = fopen(initialFunctionFile, "w");
	printAFunctionElement(inputForInstantiation, head);
	fclose(inputForInstantiation);

	char * outputAdditionalMappingsFile = (char *) malloc(3000 * sizeof(char));
	sprintf(outputAdditionalMappingsFile, "%s%s", TXLTemporaryFolder,
			"temp_output_additional_mappings.out");
	// TODO: write the rest of the mappings from the output of the following function call:

	callUniqueLocalDeclGenerator(initialFunctionFile,
			outputAdditionalMappingsFile, head->NameOfFunction,
			mappedFunctionID);

	Mapping * localExistingMappings = NULL;

	FILE * outputLocalVarMappingsForAbstract = fopen(
			outputAdditionalMappingsFile, "r");

	char * sourceLAbst = (char *) malloc(1000 * sizeof(char));
	char * destLAbst = (char *) malloc(1000 * sizeof(char));

	while (fscanf(outputLocalVarMappingsForAbstract, "%s %s", sourceLAbst,
			destLAbst) != EOF) {
		addNewMapping(&localExistingMappings, sourceLAbst, destLAbst);
	}

	fclose(outputLocalVarMappingsForAbstract);

	while (aux) {
		char * marker;
		marker = (char *) malloc(400 * sizeof(char));
		char * uniqueFunctionName;
		uniqueFunctionName = (char *) malloc(400 * sizeof(char));
		sscanf(aux->el, "%s", marker);
		if (!strcmp(marker, FUNCTION_CALL_MARKER)) {
			char * functionName;
			functionName = (char*) malloc(400 * sizeof(char));
			sscanf(aux->el, "%s %s %s", marker, functionName,
					uniqueFunctionName);

			fflush(stdout);

			solveAbstractValues(mappingsForSkeleton, localExistingMappings,
					mappedFunctionID, ListOfExistingMappings,
					uniqueFunctionName, &(*FinalListOfAbstractMappings),
					&newAbstractValues, GraftSymbolTable,
					ChanghedAbstractIDSymbolTable);

			fflush(stdout);
		}
		aux = aux->next;
		free(marker);
		free(uniqueFunctionName);
	}

	Mapping * auxAbstract;
	auxAbstract = newAbstractValues;

	current = returnMappingsForFunction(currentMappings, mappedFunctionID);

	//fix current. Search in the list of all previous mapping, and if
	//the current destination is the source of an another mapping, replace the current destination with the other
	//mapping!!!
#if 1
	Mapping * currentExistingMappingsAux = current->mappings;
	while (currentExistingMappingsAux) {
		SkeletonMapping * auxForArgs = currentMappings;
		while (auxForArgs) {

			Mapping * auxMappingLocal = auxForArgs->mappings;
			while(auxMappingLocal) {
				if(!strcmp(currentExistingMappingsAux->destination, auxMappingLocal->source)) {
					currentExistingMappingsAux->destination = (char *) malloc (1000 * sizeof(char));
					strcpy(currentExistingMappingsAux->destination, auxMappingLocal->destination);
				}
				auxMappingLocal = auxMappingLocal->next;
			}
			auxForArgs = auxForArgs->next;
		}

		currentExistingMappingsAux = currentExistingMappingsAux->next;
	}

	fflush(stdout);
#endif

	fflush(stdout);

	FILE * symbolTableResults = fopen(initialMappingsFile, "w");
	Mapping * currentListOfMappings = NULL;
	if (current) {
		currentListOfMappings = current->mappings;
	}
	while (currentListOfMappings) {
		fprintf(symbolTableResults, "\"%s\" \"%s\" ",
				currentListOfMappings->source,
				currentListOfMappings->destination);
		currentListOfMappings = currentListOfMappings->next;
	}

	// fix label
	fprintf(symbolTableResults, "\"LABEL_%s\" \"LABEL_%s\" ",
			head->NameOfFunction, mappedFunctionID);

	// fix the return value -> add the unique ID of the function call!!!
	fprintf(symbolTableResults,
			"\"$ABSTRETVAL_ret_%s\" \"$ABSTRETVAL_ret_%s\" ",
			head->NameOfFunction, mappedFunctionID);

	FILE * outputLocalVarMappings = fopen(outputAdditionalMappingsFile, "r");

	char * sourceL = (char *) malloc(1000 * sizeof(char));
	char * destL = (char *) malloc(1000 * sizeof(char));

	while (fscanf(outputLocalVarMappings, "%s %s", sourceL, destL) != EOF) {
		fprintf(symbolTableResults, "\"%s\" \"%s\" ", sourceL, destL);
	}

	fclose(outputLocalVarMappings);

	while (auxAbstract) {
		fprintf(symbolTableResults, "\"%s\" \"%s\" ", auxAbstract->source,
				auxAbstract->destination);
		auxAbstract = auxAbstract->next;
	}
	fclose(symbolTableResults);
	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));
	sprintf(commandTXL, "%s %s %s %s %s", command, initialFunctionFile,
			initialMappingsFile, skeletonFctInstantiate, ConsoleNullOutput);
	//TODO: In initial mappings file add mappings from all the declarations to new names with the unique id!

	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! instantiateFunctionsInSkeleton.x \n");
		return NULL;
	}
	free(commandTXL);
	FILE * resultedFunction = fopen(skeletonFctInstantiate, "r");
	ListOfString * newListOfString = NULL;
	char * line = (char *) malloc(1000 * sizeof(char));
	while (fgets(line, 400, resultedFunction) != NULL) {
		if (strcmp(line, "\n")) {
			addNewString(&newListOfString, line);
		}
	}
	addNewString(&newListOfString, LINE_END);
	free(line);
	fclose(resultedFunction);

	fflush(stdout);

	return newListOfString;

}

void solveAbstractValuesFromProgramEntryPoint(char * callerName,
		SkeletonMapping * currentExistingMappings, char * currentCalled,
		Mapping ** newListOfAbstractMappings, Mapping ** newAbstractValues,
		SymbolTable * GraftSymbolTable,
		SymbolTable ** ChanghedAbstractIDSymbolTable) {
	SkeletonMapping * currentAbstractMappings;
	currentAbstractMappings = returnMappingsForFunction(currentExistingMappings,
			currentCalled);
	Mapping * aux = currentAbstractMappings->mappings;
	while (aux) {
		char * newAbstractName;
		newAbstractName = (char *) malloc(500 * sizeof(char));
		//sprintf(newAbstractName, "called_from_%s_%s", callerName, aux->source);
		sprintf(newAbstractName, "$ABSTRACT_%d", abstractIDNumber);
		abstractIDNumber++;
		addNewMapping(&(*newListOfAbstractMappings), newAbstractName,
				aux->destination);
		addNewMapping(&(*newAbstractValues), aux->source, newAbstractName);

		char * typeOfID = searchForASymbolTableEntry(GraftSymbolTable,
				aux->source);
		if (typeOfID) {
			addNewVariableDeclaration(ChanghedAbstractIDSymbolTable, typeOfID,
					newAbstractName);
		}

		free(newAbstractName);
		aux = aux->next;
	}
}

ListOfString * fixAbstractsFromProgramEntryPoint(FunctionElement * head,
		char * mappedFunctionID, char * command, char * initialFunctionFile,
		char * initialMappingsFile, char * skeletonFctInstantiate,
		SkeletonMapping * ListOfExistingMappings,
		Mapping ** FinalListOfAbstractMappings, SymbolTable * GraftSymbolTable,
		SymbolTable ** ChanghedAbstractIDSymbolTable) {

	Mapping * newAbstractValues = NULL;
	ListOfString * aux = NULL;
	aux = head->LOCs;
	while (aux) {
		char * marker;
		marker = (char *) malloc(400 * sizeof(char));
		char * uniqueFunctionName;
		uniqueFunctionName = (char *) malloc(400 * sizeof(char));
		sscanf(aux->el, "%s", marker);
		if (!strcmp(marker, FUNCTION_CALL_MARKER)) {
			char * functionName;
			functionName = (char*) malloc(400 * sizeof(char));
			sscanf(aux->el, "%s %s %s", marker, functionName,
					uniqueFunctionName);

			solveAbstractValuesFromProgramEntryPoint(mappedFunctionID,
					ListOfExistingMappings, uniqueFunctionName,
					&(*FinalListOfAbstractMappings), &newAbstractValues,
					GraftSymbolTable, ChanghedAbstractIDSymbolTable);
		}
		aux = aux->next;
		free(marker);
		free(uniqueFunctionName);
	}
	Mapping * auxAbstract;
	auxAbstract = newAbstractValues;
	FILE * inputForInstantiation = fopen(initialFunctionFile, "w");
	printAFunctionElement(inputForInstantiation, head);
	FILE * symbolTableResults = fopen(initialMappingsFile, "w");
	while (auxAbstract) {
		// TODO: check here
		fprintf(symbolTableResults, "\"%s\" \"%s\" ", auxAbstract->source,
				auxAbstract->destination);
		auxAbstract = auxAbstract->next;
	}
	fclose(inputForInstantiation);
	fclose(symbolTableResults);
	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));
	sprintf(commandTXL, "%s %s %s %s %s", command, initialFunctionFile,
			initialMappingsFile, skeletonFctInstantiate, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! fixAbstractsFromProgramEntryPoint");
		return NULL;
	}
	free(commandTXL);
	FILE * resultedFunction = fopen(skeletonFctInstantiate, "r");
	ListOfString * newListOfString = NULL;
	char * line = (char *) malloc(1000 * sizeof(char));
	while (fgets(line, 400, resultedFunction) != NULL) {
		if (strcmp(line, "\n")) {
			addNewString(&newListOfString, line);
		}
	}
	fclose(resultedFunction);
	//addNewString(&newListOfString, LINE_END);
	free(line);
	return newListOfString;
}

void ReplaceFunctionCalls(FunctionElement * Original, FunctionElement ** Output,
		SkeletonMapping * mappingsForSkeleton, char * command,
		char * initialFunctionFile, char * initialMappingsFile,
		char * skeletonFctInstantiate, SkeletonMapping * ListOfExistingMappings,
		char * callerUniqueName, Mapping ** FinalListOfAbstractMappings,
		SymbolTable * GraftSymbolTable,
		SymbolTable ** ChanghedAbstractIDSymbolTable, char * TXLTemporaryFolder) {
	FunctionElement * ExtendedSk;
	ExtendedSk = *Output;
	int signal = 0;
	ListOfString * aux = NULL;
	aux = ExtendedSk->LOCs;
	while (aux) {
		char * marker;
		marker = (char *) malloc(400 * sizeof(char));
		char * uniqueFunctionName;
		uniqueFunctionName = (char *) malloc(400 * sizeof(char));
		if (aux->el) {
			sscanf(aux->el, "%s", marker);
			if (!strcmp(marker, FUNCTION_CALL_MARKER)) {
				char * functionName;
				functionName = (char*) malloc(400 * sizeof(char));
				sscanf(aux->el, "%s %s %s", marker, functionName,
						uniqueFunctionName);
				// Do not inline the core function!
				if (strcmp(functionName, GraftFunctionName)) {
					FunctionElement * searchedEl = NULL;
					searchedEl = searchAnElement(Original, functionName);
					ListOfString * searchedListOfString = NULL;
					if (searchedEl != NULL) {
						searchedListOfString = solveCalledIDs(searchedEl,
								mappingsForSkeleton, uniqueFunctionName,
								command, initialFunctionFile,
								initialMappingsFile, skeletonFctInstantiate,
								mappingsForSkeleton, ListOfExistingMappings,
								&(*FinalListOfAbstractMappings),
								GraftSymbolTable, ChanghedAbstractIDSymbolTable,
								TXLTemporaryFolder);
						fflush(stdout);
						if (searchedListOfString != NULL) {
							//first is the function call, so not add it
							replaceOneLineWithAList(&ExtendedSk->LOCs,
									searchedListOfString->next, aux->el);
							signal = 1;
						}
					}

					strcpy(callerUniqueName, uniqueFunctionName);
					freeListOfStrings(&searchedListOfString);
				}
				free(functionName);
			}
		}
		aux = aux->next;
		free(marker);
		free(uniqueFunctionName);
	}

	fflush(stdout);

	if (signal) {
		ReplaceFunctionCalls(Original, Output, mappingsForSkeleton, command,
				initialFunctionFile, initialMappingsFile,
				skeletonFctInstantiate, ListOfExistingMappings,
				callerUniqueName, FinalListOfAbstractMappings, GraftSymbolTable,
				ChanghedAbstractIDSymbolTable, TXLTemporaryFolder);
	} else {
		return;
	}
}

SkeletonMapping * readSkeletonMappings(FILE * fin) {
	SkeletonMapping * currentMappings = NULL;
	char * source;
	char * destination;
	source = (char *) malloc(800 * sizeof(char));
	destination = (char *) malloc(800 * sizeof(char));
	char * line;
	line = (char *) malloc(800 * sizeof(char));
	char * currentName;
	currentName = (char*) malloc(800 * sizeof(char));
	while (fgets(line, 400, fin) != NULL) {
		sscanf(line, "%s", currentName);
		SkeletonMapping * current = addNewSkeletonMapping(&currentMappings,
				currentName);
		int s = 0;
		while (s == 0 && fgets(line, 400, fin) != NULL) {
			if (!strcmp(line, LINE_END)) {
				s = 1;
			} else {
				char * token;
				token = strtok(line, " ");
				strcpy(source, token);
				token = strtok(NULL, "\n");
				strcpy(destination, token);
				//sscanf(line, "%s %s", source, destination);
				addNewMapping(&current->mappings, source, destination);
			}
		}
	}
	free(source);
	free(destination);
	free(line);
	free(currentName);
	return currentMappings;
}

SkeletonMapping * readExistingMappings(FILE * fin) {
	SkeletonMapping * currentMappings = NULL;
	char * source;
	char * destination;
	source = (char *) malloc(800 * sizeof(char));
	destination = (char *) malloc(800 * sizeof(char));
	char * line;
	line = (char *) malloc(800 * sizeof(char));
	SkeletonMapping * current = addNewSkeletonMapping(&currentMappings,
			"PROGRAM");
	while (fgets(line, 400, fin) != NULL) {
		sscanf(line, "%s %s", source, destination);
		addNewMapping(&current->mappings, source, destination);
	}
	free(source);
	free(destination);
	free(line);
	return currentMappings;
}

ListOfString * InstantiateACandidate(FunctionElement * head,
		Mapping * currentMappings, char * command, char * initialFunctionFile,
		char * initialMappingsFile, char * skeletonFctInstantiate) {

	if (!currentMappings) {
		return NULL;
	} else {
		FILE * inputForInstantiation = fopen(initialFunctionFile, "w");
		printAFunctionElement(inputForInstantiation, head);
		FILE * symbolTableResults = fopen(initialMappingsFile, "w");
		Mapping * currentListOfMappings = currentMappings;
		while (currentListOfMappings) {
			// TODO: check this
			//first check if the current string has quatations:
			if ((currentListOfMappings->destination[0]) == '"') {
				char * newDestination = (char *) malloc(3000 * sizeof(char));
				sprintf(newDestination, "\\%s",
						currentListOfMappings->destination);
				int originalLength = strlen(newDestination);
				newDestination[originalLength - 1] = '\\';
				newDestination[originalLength] = '"';
				newDestination[originalLength + 1] = 0;

				fprintf(symbolTableResults, "\"%s\" \"%s\" ",
						currentListOfMappings->source, newDestination);
			}

			else {
				fprintf(symbolTableResults, "\"%s\" \"%s\" ",
						currentListOfMappings->source,
						currentListOfMappings->destination);
			}
			currentListOfMappings = currentListOfMappings->next;
		}
		fclose(inputForInstantiation);
		fclose(symbolTableResults);
		char * commandTXL;
		commandTXL = (char *) malloc(5000 * sizeof(char));
		fflush(stdout);
		sprintf(commandTXL, "%s %s %s %s %s", command, initialFunctionFile,
				initialMappingsFile, skeletonFctInstantiate, ConsoleNullOutput);
		int status = system(commandTXL);
		if (status != 0) {
			printf("ERROR! Instantiate Functions in Skeleton");
			return NULL;
		}
		fflush(stdout);
		FILE * resultedFunction = fopen(skeletonFctInstantiate, "r");
		ListOfString * newListOfString = NULL;
		char * line = (char *) malloc(1000 * sizeof(char));
		while (fgets(line, 400, resultedFunction) != NULL) {
			if (strcmp(line, "\n")) {
				addNewString(&newListOfString, line);
			}
		}
		addNewString(&newListOfString, LINE_END);
		free(line);
		free(commandTXL);
		fclose(resultedFunction);
		return newListOfString;
	}
}

void generateFunctionsForSkeleton(char * graftMethod, char * donorSourceFiles,
		char * transplantWorkingFolder) {
	char sourceFiles[400];
	sprintf(sourceFiles, "%s/sourceContentFile.out", transplantWorkingFolder);
	char headerFiles[400];
	sprintf(headerFiles, "%s/headerContentFile.out", transplantWorkingFolder);
	char * graftFile = "dct.c";
	char resultFileFullPath[400];
	sprintf(resultFileFullPath, "%sgcovParsedResults.out",
			transplantWorkingFolder);
	char * gcovMainMethodMarker = "--main";
	char gcovMainMethod[400];
	sprintf(gcovMainMethod, "%s %s", gcovMainMethodMarker, graftMethod);
	char * outputFile = "outputCFLOW.out";
	char * gcov = " cflow";
	char * restOfParams = "-i _ --omit-arguments *.c >";
	char gcovCommand[400];
	char outputFileFullPath[400];
	sprintf(outputFileFullPath, "%s%s", donorSourceFiles, outputFile);
	char graftFileFullPath[400];
	sprintf(graftFileFullPath, "%s%s", donorSourceFiles, graftFile);
	sprintf(gcovCommand, "cd %s \n %s %s %s %s", donorSourceFiles, gcov,
			gcovMainMethod, restOfParams, outputFileFullPath);
	//printf("%s \n", gcovCommand);
	//printf("%s \n", outputFileFullPath);
	fflush(stdout);
	int status1 = system(gcovCommand);
	if (status1 != 0) {
		printf("ERROR");
	}
}

char * statementMarker = "/*@STM*/";
char * declMarker = "/*@DECL*/";
char * compoundStartMarker = "//COMPOUND START:";
char * compoundEndMarker = "//COMPOUND END:";
char * compoundCloseMarker = "/* COMPOUND_CLOSED */";
int currentStmNumber = 0;
int currentDeclNumber = 0;
int currentCompoundNumber = 0;

void fixLineNumbers(ListOfString * element) {
	ListOfString * aux = element;

	while (aux) {
		if (strstr(aux->el, statementMarker)) {
			char * restOfLine;
			restOfLine = strstr((aux->el), statementMarker)
					+ strlen(statementMarker) + 1;
			char * tempString = (char *) malloc(
					(strlen(aux->el) + 10) * sizeof(char));
			strcpy(tempString, restOfLine);

			sprintf(aux->el, "/* STM: %d */%s", currentStmNumber, tempString);
			currentStmNumber++;
		} else if (strstr(aux->el, declMarker)) {
			char * restOfLine;
			restOfLine = strstr((aux->el), declMarker) + strlen(declMarker) + 1;
			char * tempString = (char *) malloc(
					(strlen(aux->el) + 10) * sizeof(char));
			strcpy(tempString, restOfLine);

			sprintf(aux->el, "/* DECL: %d */%s", currentDeclNumber, tempString);
			currentDeclNumber++;
		}
		aux = aux->next;
	}

	aux = element;
	while (aux) {
		if (strstr(aux->el, compoundStartMarker)) {
			ListOfString * auxEndMarker = aux->next;
			int signal = 0;

			char * auxCurrentBeginComp;
			auxCurrentBeginComp = (char *) malloc(500 * sizeof(char));
			strcpy(auxCurrentBeginComp, aux->next->el);

			sprintf(aux->next->el, "/* COMPOUND: %d */%s",
					currentCompoundNumber, auxCurrentBeginComp);

			free(auxCurrentBeginComp);

			//this looks pretty strange... The comparison is between a pointer to the position in the line to look
			//for the end marker, the next after the end marker +1 (one space), and a pointer to the position in the
			//line of start of the compound, the next after the start marker +1 (one space). This should be the same
			//since the same number is for the beginning and the end of a compound statement
			//first check is just for checking the end marker exist, otherwise the second part would crash
			while (auxEndMarker->next && !signal) {
				//char * first = strstr(auxEndMarker->next->el, compoundEndMarker) + strlen(compoundEndMarker) + 1;
				// char * sec = strstr(aux->el, compoundStartMarker) + strlen(compoundStartMarker) + 1;

				if (strstr(auxEndMarker->next->el, compoundEndMarker)
						&& !strcmp(
								strstr(auxEndMarker->next->el,
										compoundEndMarker)
										+ strlen(compoundEndMarker) + 1,
								strstr(aux->el, compoundStartMarker)
										+ strlen(compoundStartMarker) + 1)) {

					char * auxCurrentEndComp;
					auxCurrentEndComp = (char *) malloc(500 * sizeof(char));
					strcpy(auxCurrentEndComp, auxEndMarker->el);
					sprintf(auxEndMarker->el, "/* COMPOUND: %d */%s",
							currentCompoundNumber, auxCurrentEndComp);
					free(auxCurrentEndComp);

					signal = 1;
				}
				auxEndMarker = auxEndMarker->next;
			}
			//the end was not found, so replace the last }\n found

			//       }
			//COMPOUND END: if ($_main_ofile == 0) {}

			if (!signal) {
				auxEndMarker = aux->next;
				ListOfString * foundElement = NULL;
				while (auxEndMarker) {
					if (strstr(auxEndMarker->el, compoundCloseMarker)) {
						signal = 1;
						foundElement = auxEndMarker;
					}
					auxEndMarker = auxEndMarker->next;
				}
				if (signal) {
					char * restOfLine;
					restOfLine = strstr((foundElement->el), compoundCloseMarker)
							+ strlen(compoundCloseMarker);
					sprintf(foundElement->el, "/* COMPOUND: %d */ %s",
							currentCompoundNumber, restOfLine);
				}
			}

			currentCompoundNumber++;
		}

		aux = aux->next;
	}
}

void createDependencyList(char * TXLPath, char * skeletonOutput,
		char * dependencyListOutput) {
	char * command;
	command = (char *) malloc(400 * sizeof(char));
	sprintf(command, "%sSolveDeclarationDependencies.x", TXLPath);
	char * commandTXL;
	commandTXL = (char *) malloc(500 * sizeof(char));
	sprintf(commandTXL, "%s %s %s %s", command, skeletonOutput,
			dependencyListOutput, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! SolveDeclarationDependencies\n");
		return;
	}
	free(command);
	free(commandTXL);
}

void addMarkersAtGotoExtractor(char * skeletonFile, char * markersOutput) {
	char * command;
	command = (char *) malloc(400 * sizeof(char));
	sprintf(command, "%saddMarkersAtGoto.x", TXLToolsPath);
	char * commandTXL;
	commandTXL = (char *) malloc(1000 * sizeof(char));
	sprintf(commandTXL, "%s %s %s %s", command, skeletonFile, markersOutput,
			ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! addMarkersAtGoto.x\n");
	}
	free(command);
	free(commandTXL);
}

Mapping * createGraftSkeleton(char * transformedFctLOCs,
		char * instantiatonMappings, char * TXLPath, char * skeletonOutput,
		char * existingMappings, char * temporaryFolder,
		char * HostSymbolTableFile, SymbolTable ** FinalGraftSymbolTable,
		DependencyList ** GraftStatementsDependencies,
		ListOfString * reverseCallGraph, char * TXLTemporaryFolder) {

	SymbolTable * entryPointSymbolTable = NULL;

	FILE * finHostSymbolTable = fopen(HostSymbolTableFile, "r");
	SymbolTable * GraftSymbolTable = readSymbolTableAddedEntryPoint(
			finHostSymbolTable, &entryPointSymbolTable);
	//fclose(finHostSymbolTable);
	char * initialFunctionFile;
	char * initialMappingsFile;
	char * skeletonFctInstantiate;

	initialFunctionFile = (char *) malloc(500 * sizeof(char));
	initialMappingsFile = (char *) malloc(500 * sizeof(char));
	skeletonFctInstantiate = (char *) malloc(500 * sizeof(char));

	sprintf(initialFunctionFile, "%stemp_initial_function.out",
			temporaryFolder);
	sprintf(initialMappingsFile, "%stemp_initial_mappings.out",
			temporaryFolder);
	sprintf(skeletonFctInstantiate, "%stemp_skeleton_fct_instantiate.out",
			temporaryFolder);

	//FILE * fin = fopen(transformedFctLOCs, "r");
	FILE * SkeletonInstantiationMappings = fopen(instantiatonMappings, "r");
	char * commandInstantiate;
	commandInstantiate = (char *) malloc(400 * sizeof(char));
	sprintf(commandInstantiate, "%sinstantiateFunctionsInSkeleton.x", TXLPath);
	FILE * currentExistingMappings = fopen(existingMappings, "r");
	SkeletonMapping * currentMappings;
	currentMappings = readSkeletonMappings(SkeletonInstantiationMappings);
	fclose(SkeletonInstantiationMappings);
	SkeletonMapping * MappingCandidates = NULL;
	MappingCandidates = readSkeletonMappings(currentExistingMappings); //readExistingMappings(currentExistingMappings);
	fclose(currentExistingMappings);

	fflush(stdout);

	FunctionElement * listOfFunctions = NULL;
	FunctionElement * extendedSkeleton = NULL;
	listOfFunctions = readFunctionElemenentListFromFile(transformedFctLOCs,
			reverseCallGraph);

	addNewFunctionElement(&extendedSkeleton, listOfFunctions);

	char * uniqueCallerName;
	uniqueCallerName = (char *) malloc(1000 * sizeof(char));

	strcpy(uniqueCallerName, extendedSkeleton->NameOfFunction);
	Mapping * finalListOfMappings = NULL;

	//SymbolTable * ChanghedAbstractIDSymbolTable = NULL;

	ListOfString * fixedAbstractFirst = fixAbstractsFromProgramEntryPoint(
			extendedSkeleton, extendedSkeleton->NameOfFunction,
			commandInstantiate, initialFunctionFile, initialMappingsFile,
			skeletonFctInstantiate, MappingCandidates, &finalListOfMappings,
			GraftSymbolTable, &(*FinalGraftSymbolTable));

	fflush(stdout);

	freeListOfStrings(&(extendedSkeleton->LOCs));
	extendedSkeleton->LOCs = fixedAbstractFirst;

	ReplaceFunctionCalls(listOfFunctions, &extendedSkeleton, currentMappings,
			commandInstantiate, initialFunctionFile, initialMappingsFile,
			skeletonFctInstantiate, MappingCandidates, uniqueCallerName,
			&finalListOfMappings, GraftSymbolTable, &(*FinalGraftSymbolTable),
			TXLTemporaryFolder);
	free(uniqueCallerName);

	fflush(stdout);

	//printSymbolTable(ChanghedAbstractIDSymbolTable, "/home/alex/workspace/TESTSYMBOLTABLE.out");
	//to be removed!!!
	//printAListOfStrings(extendedSkeleton->LOCs, "/home/alex/workspace/TestTXL/testSkeleton.out");

	ListOfString * instantiatedCandidate = InstantiateACandidate(
			extendedSkeleton, finalListOfMappings, commandInstantiate,
			initialFunctionFile, initialMappingsFile, skeletonFctInstantiate);

	fflush(stdout);

	ListOfString * finalSkeletonLOCs = extendedSkeleton->LOCs;
	fixLineNumbers(finalSkeletonLOCs);

	FILE * transplantCandidate = fopen(skeletonOutput, "w");
	//the last line is just the end candidate marker or not any more...
	ListOfString * aux;
	aux = finalSkeletonLOCs;
	while (aux) {
		fprintf(transplantCandidate, "%s", aux->el);
		aux = aux->next;
	}
	fclose(transplantCandidate);

	// Add markers for the goto statements !!!

	//call the AllLineMarkersExtractor
	char * tempGotoMarkers = (char *) malloc(500 * sizeof(char));
	sprintf(tempGotoMarkers, "%stemps_goto_markers.our", TXLTemporaryFolder);
	addMarkersAtGotoExtractor(skeletonOutput, tempGotoMarkers);

	FILE * finGoto = fopen(tempGotoMarkers, "r");
	char * line = (char *) malloc(1000 * sizeof(char));

	while (fgets(line, 1000, finGoto) != NULL) {
		char * initialLabel = (char *) malloc(300 * sizeof(char));
		char * gotoReplacement = (char *) malloc(10 * sizeof(char));

		sscanf(line, "%s %s", gotoReplacement, initialLabel);
		char * newAnnotatedLine = (char *) malloc(1000 * sizeof(char));

		fgets(newAnnotatedLine, 1000, finGoto);

		ListOfString * auxSkeleton = finalSkeletonLOCs;
		while (auxSkeleton) {

			if (strstr(auxSkeleton->el, gotoReplacement)
					&& strstr(auxSkeleton->el, initialLabel)) {
				//identified the current label
				free(auxSkeleton->el);
				auxSkeleton->el = (char *) malloc(
						(strlen(newAnnotatedLine) + 1) * sizeof(char));
				strcpy(auxSkeleton->el, newAnnotatedLine);
			}

			auxSkeleton = auxSkeleton->next;
		}

	}

	transplantCandidate = fopen(skeletonOutput, "w");
	aux = finalSkeletonLOCs;
	while (aux) {
		fprintf(transplantCandidate, "%s", aux->el);
		aux = aux->next;
	}
	fclose(transplantCandidate);

	char * dependencyListTempFile;
	dependencyListTempFile = (char *) malloc(500 * sizeof(char));
	sprintf(dependencyListTempFile, "%stemp_dependency_list.out",
			temporaryFolder);
	createDependencyList(TXLPath, skeletonOutput, dependencyListTempFile);

	fflush(stdout);
	*GraftStatementsDependencies = readDependencyListFromFile(
			dependencyListTempFile);
	fflush(stdout);

	//free memory
	free(dependencyListTempFile);
	freeListOfFunctionElement(&listOfFunctions);
	freeListOfFunctionElement(&extendedSkeleton);
	freeListOfStrings(&instantiatedCandidate);
	freeSkeletonMapping(&currentMappings);
	freeSkeletonMapping(&MappingCandidates);

	free(initialFunctionFile);
	free(initialMappingsFile);
	free(skeletonFctInstantiate);
	free(commandInstantiate);

	addOneSymbolTableToTheOther(&(*FinalGraftSymbolTable),
			entryPointSymbolTable);

	return finalListOfMappings;

}

void instantiateGraftSkeletonInInterface(ListOfString * Skeleton,
		char * InterfaceSourceCode, char * finalHostInterfaceOutput,
		char * headerFileForCoreFeature) {
	ListOfString * interfaceCode = NULL;
	ListOfString * skeletonCode = NULL;

	interfaceCode = readListOfStringFromFile(InterfaceSourceCode);
	skeletonCode = Skeleton; //readListOfStringFromFile(Skeleton);

	removeFirstEl(&skeletonCode);
	//removeLastString(&skeletonCode);

	int signal = 0;

	ListOfString * aux;
	aux = interfaceCode;

	while (aux && !signal) {
		char * lineString;
		lineString = (char *) malloc(200 * sizeof(char));
		sscanf(aux->el, "%s", lineString);
		if (!strcmp(lineString, "__INTERFACE_FUNCTION")) {
			signal = 1;
			replaceOneLineWithAListKeepPenultimate(&interfaceCode, skeletonCode,
					aux->el);
		} else {
			aux = aux->next;
		}
		free(lineString);
	}

	//not needed any more, since all the declarations and header are now in the same place
	char * includeCoreFeatureHeader;
	includeCoreFeatureHeader = (char *) malloc(1000 * sizeof(char));

	sprintf(includeCoreFeatureHeader, "#include \"%s\"\n",
			headerFileForCoreFeature);
	//put this as the first Line;

	//ListOfString * finalHostInterfaceOutputWithInclude = NULL;
	//addNewString(&finalHostInterfaceOutputWithInclude,includeCoreFeatureHeader);

	//finalHostInterfaceOutputWithInclude->next = interfaceCode;
	//UNTIL HERE it was the previous version, with the header file included

	ListOfString * finalHostInterfaceOutputWithInclude = interfaceCode;

	printAListOfStrings(finalHostInterfaceOutputWithInclude,
			finalHostInterfaceOutput);

	//free memory
	//sfreeListOfStrings(&interfaceCode);
	freeListOfStrings(&skeletonCode);
	//free(finalHostInterfaceOutput);
	freeListOfStrings(&finalHostInterfaceOutputWithInclude);

}
