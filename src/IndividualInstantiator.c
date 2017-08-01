#include "IndividualInstantiator.h"
#include "ListOfString.h"
#include "PrepareForGP.h"
#include "GenTrans.h"
#include "Skeleton.h"
#include "ListOfInt.h"
#include "FunctionElement.h"
#include "coreFunctionExtractor.h"
#include "GP.h"
#include "GPIndividual.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int readTransplantCandidadateFile(char * inputCandidate,
		Mapping ** FinalMappingsInIndividual,
		ListOfString ** MappedDeclarations,
		ListOfInt ** ListOfCandidateSelectedLines) {

	//count the number of mapped graft IDs to Host Symbol Table
	int mappedIDsToHostVariables = 0;

	FILE * fin = fopen(inputCandidate, "r");

	char * line;
	line = (char *) malloc(3000 * sizeof(char));

	//read abstract mappings
	fgets(line, 2999, fin);
	fgets(line, 2999, fin);
	while (strcmp(line, DeclarationMappingsMarker)) {

		mappedIDsToHostVariables = mappedIDsToHostVariables + 1;

		char * source;
		char * destination;
		source = (char*) malloc(500 * sizeof(char));
		destination = (char*) malloc(500 * sizeof(char));

		//+1 for the space
		sscanf(line, "%s", source);
		line += strlen(source) + 1;

		//remove end line character
		line[strlen(line) - 1] = '\0';
		strcpy(destination, line);

		//sscanf(line, "%s %s", source, destination);
		addNewMapping(&(*FinalMappingsInIndividual), source, destination);
		free(source);
		free(destination);
		fgets(line, 2999, fin);
	}

	fgets(line, 2999, fin);
	while (strcmp(line, IndividualLinesMarker)) {

		mappedIDsToHostVariables = mappedIDsToHostVariables + 1;

		char * declMarker;
		char * source;
		char * destination;

		declMarker = (char*) malloc(500 * sizeof(char));
		source = (char*) malloc(500 * sizeof(char));
		destination = (char*) malloc(500 * sizeof(char));
		char * token = strtok(line, "@");
		strcpy(declMarker, token);
		token = strtok(NULL, "@");
		strcpy(source, token);
		token = strtok(NULL, "@");
		strcpy(destination, token);
		//last character in destination is end of line
		destination[strlen(destination) - 1] = '\0';

		addNewMapping(&(*FinalMappingsInIndividual), source, destination);
		addNewString(&(*MappedDeclarations), declMarker);
		fgets(line, 2999, fin);
		free(declMarker);
		free(source);
		free(destination);
	}

	while (fgets(line, 2999, fin)) {
		int value;
		sscanf(line, "%d", &value);
		addNewInt(&(*ListOfCandidateSelectedLines), value);
	}

	//free(line);
	fclose(fin);
	return mappedIDsToHostVariables;
}

void extractCoreFunctionsCalledInIndividual(
		CallDependency * ListOfNeededFunctionsForAllLOCs,
		ListOfString * candidateLOCsInstantiated,
		char * headerFilesForCoreFunctions, char * sourceFileForCoreFunction,
		ListOfString ** FinalListOfTransplantedFunctions,
		CallDependency * calledFunctionsByGlobals,
		char * TXLTemporaryFolder) {

	//this is needed for knowing what global dependencies to transplant
	ListOfString * listOfTransplantedFunctions =
			*FinalListOfTransplantedFunctions;
	//extract core functionalities!!!
	CallDependency * auxCallDepend = ListOfNeededFunctionsForAllLOCs;
	while (auxCallDepend) {
		ListOfString * auxCandidateInstLoc = candidateLOCsInstantiated;
		int signal = 0;
		while (auxCandidateInstLoc && !signal) {
			if (strstr(auxCandidateInstLoc->el, auxCallDepend->LOCMarker)) {
				signal = 1;
			}
			auxCandidateInstLoc = auxCandidateInstLoc->next;
		}
		if (signal == 1) {
			//the line was called, so all its dependencies must be transplanted
			ListOfFunctionSourceFile * auxFunction =
					auxCallDepend->calledFunctions;
			while (auxFunction) {
				char * functionHeaderFile;
				functionHeaderFile = (char *) malloc(500 * sizeof(char));
				sprintf(functionHeaderFile, "%s%s_header_file.out",
						TransplantResultLocation, auxFunction->functionName);

				//transplanted function marker for global dependency extractor
				char * transplantedFunctionMarker;
				transplantedFunctionMarker = (char *) malloc(
						500 * sizeof(char));
				sprintf(transplantedFunctionMarker, "/* %s */",
						auxFunction->functionName);

				if (!searchStringInListOfStrings(listOfTransplantedFunctions,
						transplantedFunctionMarker)) {
					addNewString(&listOfTransplantedFunctions,
							transplantedFunctionMarker);
					extractCoreFunctionality(auxFunction->functionName,
							auxFunction->sourceFile, donorSourceFileFolder,
							transplantWorkFolderMainThread, TXLToolsPath,
							TXLTemporaryFolder, TXLTemporaryFolder,
							headerFilesForCoreFunctions,
							sourceFileForCoreFunction);
				}

				auxFunction = auxFunction->next;

				free(functionHeaderFile);
			}
		}
		auxCallDepend = auxCallDepend->next;
	}

	//now add the ones called by globals!

	ListOfFunctionSourceFile * auxFunction =
			calledFunctionsByGlobals->calledFunctions;
	while (auxFunction) {
		char * functionHeaderFile;
		functionHeaderFile = (char *) malloc(500 * sizeof(char));
		sprintf(functionHeaderFile, "%s%s_header_file.out",
				TransplantResultLocation, auxFunction->functionName);

		//transplanted function marker for global dependency extractor
		char * transplantedFunctionMarker;
		transplantedFunctionMarker = (char *) malloc(500 * sizeof(char));
		sprintf(transplantedFunctionMarker, "/* %s */",
				auxFunction->functionName);

		if (!searchStringInListOfStrings(listOfTransplantedFunctions,
				transplantedFunctionMarker)) {
			addNewString(&listOfTransplantedFunctions,
					transplantedFunctionMarker);
			extractCoreFunctionality(auxFunction->functionName,
					auxFunction->sourceFile, donorSourceFileFolder,
					transplantWorkFolderMainThread, TXLToolsPath, TXLTemporaryFolder,
					TXLTemporaryFolder, headerFilesForCoreFunctions,
					sourceFileForCoreFunction);
		}
		auxFunction = auxFunction->next;

		free(functionHeaderFile);
	}

	// * FinalListOfTransplantedFunctions = listOfTransplantedFunctions;
	//extract needed globals for the core file
}

ListOfString * instantiateCurrentIndividualInSkeleton(
		char * initialFunctionFile, char * initialMappingsFile, char * TXLTemporaryFolder) {

	//instantiate the current candidate in skeleton

	char * command;
	command = (char *) malloc(400 * sizeof(char));
	sprintf(command, "%sinstantiateFunctionsInSkeleton.x", TXLToolsPath);

	char * skeletonFctInstantiate;
	skeletonFctInstantiate = (char *) malloc(400 * sizeof(char));
	sprintf(skeletonFctInstantiate,
			"%stemp_candidate_skeleton_instantiated.out", TXLTemporaryFolder);

	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));
	sprintf(commandTXL, "%s %s %s %s %s", command, initialFunctionFile,
			initialMappingsFile, skeletonFctInstantiate, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! instantiateCurrentIndividualInSkeleton.x\n");
		return NULL;
	}
	free(commandTXL);
	FILE * resultedFunction = fopen(skeletonFctInstantiate, "r");

	ListOfString * candidateLOCsInstantiated = NULL;
	char * line = (char *) malloc(1000 * sizeof(char));
	while (fgets(line, 400, resultedFunction) != NULL) {
		if (strcmp(line, "\n")) {
			addNewString(&candidateLOCsInstantiated, line);
		}
	}
	fclose(resultedFunction);
	return candidateLOCsInstantiated;
}

ListOfString * generateNewSkeletonForCandidate(ArrayOfStrings SkeletonLOCsArray,
		ListOfString * SkeletonSourceCode, char * candidateFile,
		DependencyList * ListOfDependenciesForStatements,
		ListOfString ** TheListOfTransplantedMarkers,
		int * MappedGraftHostVariables,
		char * TXLTemporaryFolder) {

	Mapping * FinalMappingsInIndividual = NULL;
	ListOfString * MappedDeclarations = NULL;
	ListOfInt * ListOfCandidateSelectedLines = NULL;

	//mapped declarations are declarations from the skeleton, which are mapped to variables from the host,
	//so there is no need to transplant the variable declarations on which these are dependent
	*MappedGraftHostVariables = readTransplantCandidadateFile(candidateFile,
			&FinalMappingsInIndividual, &MappedDeclarations,
			&ListOfCandidateSelectedLines);

	ListOfString * neededMarkersForTransplant = NULL;

	//add all the line markers from the SkeletonLOC array, at the position selected by GP
	ListOfInt * aux = ListOfCandidateSelectedLines;
	while (aux) {
		if (!searchStringInListOfStrings(neededMarkersForTransplant,
				SkeletonLOCsArray.array[aux->value])) {
			addNewString(&neededMarkersForTransplant,
					SkeletonLOCsArray.array[aux->value]);
		}
		aux = aux->next;
	}

	//add the return marker, since it is the return of the interface and must always be presented
	addNewString(&neededMarkersForTransplant, RETURN_MARKER);

	ListOfString * auxNeededMarkers = neededMarkersForTransplant;

	//for all added line markers, add the one from dependency, if they are not mapped
	while (auxNeededMarkers) {
		//memory leak!!! returnDependencyForAMarker makes new one, while adding adds also
		ListOfString * neededDepsForCurrent =
				returnDependencyForAMarkerNewPointers(
						ListOfDependenciesForStatements, auxNeededMarkers->el);
		if (neededDepsForCurrent) {
			ListOfString * auxListString = neededDepsForCurrent;
			//maybe this is infinite.. it should be checked that the new marker is not already in the list
			while (auxListString) {
				if (!searchStringInListOfStrings(MappedDeclarations,
						auxListString->el)) {
					addNewString(&neededMarkersForTransplant,
							auxListString->el);
				}
				auxListString = auxListString->next;
			}
			freeListOfStrings(&neededDepsForCurrent);
		}
		auxNeededMarkers = auxNeededMarkers->next;
	}

	*TheListOfTransplantedMarkers = neededMarkersForTransplant;

	ListOfString * candidateLOCs = NULL;
	ListOfString * auxSkeletonCode = SkeletonSourceCode;

	while (auxSkeletonCode) {
		//add some marker for LABEL ABST!!!, not like this
		if (!strstr(auxSkeletonCode->el, "/*")
				&& !strstr(auxSkeletonCode->el, "//")
				&& !strstr(auxSkeletonCode->el, "LABEL_ABST")
				&& !strstr(auxSkeletonCode->el, "break")) {
			addNewString(&candidateLOCs, auxSkeletonCode->el);
		} else {
			auxNeededMarkers = neededMarkersForTransplant;
			while (auxNeededMarkers) {
				if (strstr(auxSkeletonCode->el, auxNeededMarkers->el)) {
					addNewString(&candidateLOCs, auxSkeletonCode->el);
				}
				auxNeededMarkers = auxNeededMarkers->next;
			}
		}
		auxSkeletonCode = auxSkeletonCode->next;
	}

	//print the initial function file, for individual instantiator
	char * initialFunctionFile;
	initialFunctionFile = (char *) malloc(500 * sizeof(char));
	sprintf(initialFunctionFile, "%stemp_individual_for_instantiation.out",
			TXLTemporaryFolder);

	printAListOfStrings(candidateLOCs, initialFunctionFile);

	//print the initial mappings file, for individual instantiator
	char * initialMappingsFile;
	initialMappingsFile = (char *) malloc(500 * sizeof(char));

	sprintf(initialMappingsFile,
			"%stemp_mappings_for_candidate_instantiation.out",
			TXLTemporaryFolder);

	FILE * symbolTableResults = fopen(initialMappingsFile, "w");
	Mapping * currentListOfMappings = FinalMappingsInIndividual;

	while (currentListOfMappings) {
		fprintf(symbolTableResults, "\"%s\" \"%s\" ",
				currentListOfMappings->source,
				currentListOfMappings->destination);
		currentListOfMappings = currentListOfMappings->next;
	}
	fclose(symbolTableResults);

	ListOfString * candidateLOCsInstantiated =
			instantiateCurrentIndividualInSkeleton(initialFunctionFile,
					initialMappingsFile, TXLTemporaryFolder);
	return candidateLOCsInstantiated;
}

void callCoreFeatureGlobalDependencySolver(char * tempSkeletonFile,
		char * tempGlobalDeclarations, char * tempGlobalTypeVarDecl,
		char * TXLProgsPath, char * temporaryOutputFile) {
	char * commandTXL;
	commandTXL = (char *) malloc(1500 * sizeof(char));
	sprintf(commandTXL,
			"%sGlobalVarTypesDeclDependencyCOREFUNCTIONS.x %s %s %s %s %s",
			TXLProgsPath, tempSkeletonFile, tempGlobalDeclarations,
			tempGlobalTypeVarDecl, temporaryOutputFile, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! sGlobalVarTypesDeclDependencyCOREFUNCTIONS.x\n");
	}
	free(commandTXL);
}

DependencyList * extractDependencyGlobalListForCoreFunctions(
		char * coreFunctionSourceFile, char * globalDeclarationFile,
		char * globalDefinedTypesVarFile, char * TXLTemporaryFolder) {

	char * tempOutputFile;
	tempOutputFile = (char *) malloc(500 * sizeof(char));
	sprintf(tempOutputFile, "%stemp_core_function_global_dependencies.out",
			TXLTemporaryFolder);

	fflush(stdout);

	callCoreFeatureGlobalDependencySolver(coreFunctionSourceFile,
			globalDeclarationFile, globalDefinedTypesVarFile, TXLToolsPath,
			tempOutputFile);

	fflush(stdout);

	DependencyList * skeletonGlobalDependencies = readDependencyListFromFile(
			tempOutputFile);
	//return skeletonGlobalDependencies;
	return skeletonGlobalDependencies;
}

void addLocsFromSourceToDestination(char * destinationFileName,
		char * sourceFileName, char * TXLProgsPath) {
	char * commandTXL;
	commandTXL = (char *) malloc(1000 * sizeof(char));
	sprintf(commandTXL,
			"%sInsertAllSourceCodeFromInputAtTheBegginingOfOutput.x %s %s %s",
			TXLProgsPath, sourceFileName, destinationFileName,
			ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! InsertAllSourceCodeFromInputAtTheBegginingOfOutput");

		int x;
		x = 10 / 0;

		fflush(stdout);
	}
	free(commandTXL);
}

void extractNeededGlobalDeclarationsAndHeaderForSkeletonAndInterface(
		GlobalDeclarations ListOfFinalGlobalDeclarations,
		DependencyList * GlobalDependencyList,
		DependencyList * SkeletonDependenciesOnGlobals,
		DependencyList * DependencyListGlobalCoreFunctions,
		ListOfString * ListOfSkeletonSelectedLOCs,
		ListOfString * ListOfSelectedFunctionNames,
		char * tempSourceCodeGlobalsContent, char * tempHeaderGlobalsContent) {

	GlobalDeclarations AllGlobalNeededDeclarationsSkeleton =
			extractNeededGlobalDeclarationsForSkeleton(
					ListOfSkeletonSelectedLOCs, ListOfFinalGlobalDeclarations,
					GlobalDependencyList, SkeletonDependenciesOnGlobals);

	GlobalDeclarations AllGlobalNeededDeclarationsCoreFunctions =
			extractNeededGlobalDeclarationsForSkeleton(
					ListOfSelectedFunctionNames, ListOfFinalGlobalDeclarations,
					GlobalDependencyList, DependencyListGlobalCoreFunctions);

	GlobalDeclarations GlobalNeededCoreAndSkeleton;
	GlobalNeededCoreAndSkeleton.normalDeclarations = NULL;
	GlobalNeededCoreAndSkeleton.structUnionDeclarations = NULL;
	GlobalNeededCoreAndSkeleton.typedefDeclarations = NULL;

	addGlobalDeclarationListToExistingOne(&GlobalNeededCoreAndSkeleton,
			AllGlobalNeededDeclarationsSkeleton);
	addGlobalDeclarationListToExistingOne(&GlobalNeededCoreAndSkeleton,
			AllGlobalNeededDeclarationsCoreFunctions);

	printListOfGlobalDeclarations(GlobalNeededCoreAndSkeleton,
			tempSourceCodeGlobalsContent, tempHeaderGlobalsContent);
}

int instantiateGPCandidateWithExistingGlobalAndCoreFeature(
		ListOfString * SkeletonSourceCode, ArrayOfStrings SkeletonLOCsArray,
		char * individualFile, DependencyList * ListOfDependenciesForStatements,
		char * GraftInterfaceTempOutput,
		char * skeletonInterfaceSourceCodeOutput,
		char * finalGraftInterfaceFile,
		char * TXLTemporaryFolder) {

	int MappedHostGraftIDs;

	char * finalHostInterfaceOutput;
	finalHostInterfaceOutput = (char *) malloc(500 * sizeof(char));
	sprintf(finalHostInterfaceOutput, "%stemp_interface_source_file.out",
			TXLTemporaryFolder);
	ListOfString * TheListOfTransplantedMarkers = NULL;
	ListOfString * candidateSourceCode = generateNewSkeletonForCandidate(
			SkeletonLOCsArray, SkeletonSourceCode, individualFile,
			ListOfDependenciesForStatements, &TheListOfTransplantedMarkers,
			&MappedHostGraftIDs, TXLTemporaryFolder);
	instantiateGraftSkeletonInInterface(candidateSourceCode,
			GraftInterfaceTempOutput, skeletonInterfaceSourceCodeOutput,
			JustCoreFeatureHeaderName);

	addLocsFromSourceToDestination(skeletonInterfaceSourceCodeOutput,
			SourceOutputCoreFunctionalities, TXLToolsPath);
	addLocsFromSourceToDestination(finalGraftInterfaceFile,
			SkeletonCoreFeatureHeader, TXLToolsPath);

	return MappedHostGraftIDs;
}

void readGPIndividual(GPIndividual * candidate,
		Mapping ** FinalMappingsInIndividual,
		ListOfString ** MappedDeclarations,
		ListOfInt ** ListOfCandidateSelectedLines) {

	char * line;
	line = (char *) malloc(3000 * sizeof(char));

	Mapping * auxCandidateAbstractMappings = candidate->listOfAbstractMappings;

	while (auxCandidateAbstractMappings) {

		addNewMapping(&(*FinalMappingsInIndividual),
				auxCandidateAbstractMappings->source,
				auxCandidateAbstractMappings->destination);
		auxCandidateAbstractMappings = auxCandidateAbstractMappings->next;
	}

	ListOfString * auxDeclMappings = candidate->MappedDeclarations;
	while (auxDeclMappings) {

		strcpy(line, auxDeclMappings->el);

		char * declMarker;
		char * source;
		char * destination;

		declMarker = (char*) malloc(1500 * sizeof(char));
		source = (char*) malloc(1500 * sizeof(char));
		destination = (char*) malloc(1500 * sizeof(char));
		char * token = strtok(line, "@");
		strcpy(declMarker, token);
		token = strtok(NULL, "@");
		strcpy(source, token);
		token = strtok(NULL, "@");

		strcpy(destination, token);
		//last character in destination is end of line .... It is not the case here, since
		//it was generated from the variables in the individual
		//destination[strlen(destination) - 1] = '\0';

		addNewMapping(&(*FinalMappingsInIndividual), source, destination);
		addNewString(&(*MappedDeclarations), declMarker);

		free(declMarker);
		free(source);
		free(destination);

		auxDeclMappings = auxDeclMappings->next;
	}

	ListOfInt * auxSourceCodeLines = candidate->selectedLOCs;
	while (auxSourceCodeLines) {
		addNewInt(&(*ListOfCandidateSelectedLines), auxSourceCodeLines->value);
		auxSourceCodeLines = auxSourceCodeLines->next;
	}
	free(line);
}

ListOfString * returnDependencyListWithIDForAMarkerNewPointersTTT(
		DependencyListWithID * head, char * ID, char ** dependencyLineMarker) {
	ListOfString * dependencyListForMarker = NULL;
	int signal = 0;
	while (head && !signal) {
		if (strstr(ID, head->DependendID)/* !strcmp(head->DependendID, ID)*/) {
			signal = 1;
		} else {
			head = head->next;
		}
	}
	if (signal) {
		free(*dependencyLineMarker);
		*dependencyLineMarker = (char *) malloc(
				(sizeof(head->Statement) + 1) * sizeof(char));
		strcpy(*dependencyLineMarker, head->Statement);
		dependencyListForMarker = copyCompleteListOfString(
				head->ListOfDependencies);
	}
	return dependencyListForMarker;
}

DependencyList * generateAdditionalsDeclarationsDependenciesForAbstractInstantiationsToDonorIDs(
		DependencyListWithID * DonorIdsDeclarationDependences,
		Mapping * FinalMappingsInGraft) {

	DependencyList * additionalDependencies = NULL;

	Mapping * auxMapping = FinalMappingsInGraft;

	while (auxMapping) {
		char * dependencyLineMarker = NULL;

		ListOfString * dependencyListForMarker = NULL;
		DependencyListWithID * auxDependList = DonorIdsDeclarationDependences;

		while (auxDependList) {
			if (strstr(auxMapping->destination,
					auxDependList->DependendID)/* !strcmp(head->DependendID, ID)*/) {
				dependencyLineMarker = (char *) malloc(
						1000 * sizeof(char));
				strcpy(dependencyLineMarker, auxDependList->Statement);
				dependencyListForMarker = copyCompleteListOfString(
						auxDependList->ListOfDependencies);

				addNewStatement(&additionalDependencies, dependencyLineMarker);

				while (dependencyListForMarker) {
					addNewDependency(&additionalDependencies,
							dependencyLineMarker, dependencyListForMarker->el);
					dependencyListForMarker = dependencyListForMarker->next;
				}

			}
			auxDependList = auxDependList -> next;
		}

		/*
		ListOfString * auxListOfStrings =
				returnDependencyListWithIDForAMarkerNewPointers(
						DonorIdsDeclarationDependences, auxMapping->destination,
						&dependencyLineMarker);

		if (auxListOfStrings && dependencyLineMarker) {

			addNewStatement(&additionalDependencies, dependencyLineMarker);

			while (auxListOfStrings) {
				addNewDependency(&additionalDependencies, dependencyLineMarker,
						auxListOfStrings->el);
				auxListOfStrings = auxListOfStrings->next;
			}
		}
		*/

		auxMapping = auxMapping->next;
	}

	return additionalDependencies;
}

ListOfString * generateNewSkeletonForCandidateFromGPIndividual(
		ArrayOfStrings SkeletonLOCsArray, ListOfString * SkeletonSourceCode,
		GPIndividual * individual,
		DependencyList * ListOfDependenciesForStatements,
		ListOfString ** TheListOfTransplantedMarkers,
		DependencyListWithID * dependencyListForAbstract,
		char * TXLTemporaryFolder) {

	Mapping * FinalMappingsInIndividual = NULL;
	ListOfString * MappedDeclarations = NULL;
	ListOfInt * ListOfCandidateSelectedLines = NULL;

	//mapped declarations are declarations from the skeleton, which are mapped to variables from the host,
	//so there is no need to transplant the variable declarations on which these are dependent
	readGPIndividual(individual, &FinalMappingsInIndividual,
			&MappedDeclarations, &ListOfCandidateSelectedLines);

	DependencyList * additionalDependenciesForAbstract =
			generateAdditionalsDeclarationsDependenciesForAbstractInstantiationsToDonorIDs(
					dependencyListForAbstract, FinalMappingsInIndividual);

	ListOfString * neededMarkersForTransplant = NULL;

	//add all the line markers from the SkeletonLOC array, at the position selected by GP
	ListOfInt * aux = ListOfCandidateSelectedLines;
	while (aux) {
		if (!searchStringInListOfStrings(neededMarkersForTransplant,
				SkeletonLOCsArray.array[aux->value])) {
			addNewString(&neededMarkersForTransplant,
					SkeletonLOCsArray.array[aux->value]);
		}
		aux = aux->next;
	}

	//add the return marker, since it is the return of the interface and must always be presented
	addNewString(&neededMarkersForTransplant, RETURN_MARKER);

	ListOfString * auxNeededMarkers = neededMarkersForTransplant;

	//generate the complete list of dependencies into new one
	DependencyList * completeListOfDependencies = NULL;
	addDependencyListToAnExistingOne(&completeListOfDependencies,
			ListOfDependenciesForStatements);
	addDependencyListToAnExistingOne(&completeListOfDependencies,
			additionalDependenciesForAbstract);

	//for all added line markers, add the one from dependency, if they are not mapped
	//BUGGGGGGGGGGG!!!!!!!!!!!
	// TODO: check here
	while (auxNeededMarkers) {
		//memory leak!!! returnDependencyForAMarker makes new one, while adding adds also
		ListOfString * neededDepsForCurrent =
				returnDependencyForAMarkerNewPointers(
						completeListOfDependencies, auxNeededMarkers->el);
		if (neededDepsForCurrent) {
			ListOfString * auxListString = neededDepsForCurrent;
			//maybe this is infinite.. it should be checked that the new marker is not already in the list
			while (auxListString) {
				if (!searchStringInListOfStrings(MappedDeclarations,
						auxListString->el)
						&& !searchStringInListOfStrings(
								neededMarkersForTransplant,
								auxListString->el)) {
					addNewString(&neededMarkersForTransplant,
							auxListString->el);
				}
				auxListString = auxListString->next;
			}
			freeListOfStrings(&neededDepsForCurrent);
		}
		auxNeededMarkers = auxNeededMarkers->next;
	}

	*TheListOfTransplantedMarkers = neededMarkersForTransplant;

	ListOfString * candidateLOCs = NULL;
	ListOfString * auxSkeletonCode = SkeletonSourceCode;

	while (auxSkeletonCode) {
		//add some marker for LABEL ABST!!!, not like this
		if (!strstr(auxSkeletonCode->el, "/*")
				&& !strstr(auxSkeletonCode->el, "//")
				&& !strstr(auxSkeletonCode->el, "LABEL_ABST")
				&& !strstr(auxSkeletonCode->el, "break")) {
			addNewString(&candidateLOCs, auxSkeletonCode->el);
		} else {
			auxNeededMarkers = neededMarkersForTransplant;
			//add just once every line, even if it is needed for more then one declaration
			int signal = 0;
			while (auxNeededMarkers && !signal) {
				if (strstr(auxSkeletonCode->el, auxNeededMarkers->el)) {
					//the current line in skeleton was identified, so it should be added just once
					signal = 1;
					addNewString(&candidateLOCs, auxSkeletonCode->el);
					//printf("%s\n", auxSkeletonCode->el);

				}
				auxNeededMarkers = auxNeededMarkers->next;
			}
		}
		auxSkeletonCode = auxSkeletonCode->next;
	}

	//print the initial function file, for individual instantiator
	char * initialFunctionFile;
	initialFunctionFile = (char *) malloc(500 * sizeof(char));
	sprintf(initialFunctionFile, "%stemp_individual_for_instantiation.out",
			TXLTemporaryFolder);

	printAListOfStrings(candidateLOCs, initialFunctionFile);

	//print the initial mappings file, for individual instantiator
	char * initialMappingsFile;
	initialMappingsFile = (char *) malloc(500 * sizeof(char));

	sprintf(initialMappingsFile,
			"%stemp_mappings_for_candidate_instantiation.out",
			TXLTemporaryFolder);

	FILE * symbolTableResults = fopen(initialMappingsFile, "w");
	Mapping * currentListOfMappings = FinalMappingsInIndividual;

	while (currentListOfMappings) {
		fprintf(symbolTableResults, "\"%s\" \"%s\" ",
				currentListOfMappings->source,
				currentListOfMappings->destination);
		currentListOfMappings = currentListOfMappings->next;
	}
	fclose(symbolTableResults);

	ListOfString * candidateLOCsInstantiated =
			instantiateCurrentIndividualInSkeleton(initialFunctionFile,
					initialMappingsFile, TXLTemporaryFolder);
	return candidateLOCsInstantiated;
}

void instantiateGPCandidateWithExistingGlobalAndCoreFeatureGromGPIndividual(
		ListOfString * SkeletonSourceCode, ArrayOfStrings SkeletonLOCsArray,
		GPIndividual * individual,
		DependencyList * ListOfDependenciesForStatements,
		char * GraftInterfaceTempOutput,
		char * skeletonInterfaceSourceCodeOutput,
		char * finalGraftInterfaceFile,
		DependencyListWithID * dependencyListForAbstract,
		char * TXLTemporaryFolder) {

	//Mapped Host Graft IDs used here just for compiliance with the instantiate a candidate with just its own needed
	//global declarations, where this will be also changed

	char * finalHostInterfaceOutput;
	finalHostInterfaceOutput = (char *) malloc(500 * sizeof(char));
	sprintf(finalHostInterfaceOutput, "%stemp_interface_source_file.out",
			TXLTemporaryFolder);
	ListOfString * TheListOfTransplantedMarkers = NULL;
	fflush(stdout);
	//bug place!!!!
	ListOfString * candidateSourceCode =
			generateNewSkeletonForCandidateFromGPIndividual(SkeletonLOCsArray,
					SkeletonSourceCode, individual,
					ListOfDependenciesForStatements,
					&TheListOfTransplantedMarkers, dependencyListForAbstract,
					TXLTemporaryFolder);

	fflush(stdout);
	instantiateGraftSkeletonInInterface(candidateSourceCode,
			GraftInterfaceTempOutput, skeletonInterfaceSourceCodeOutput,
			JustCoreFeatureHeaderName);
	fflush(stdout);

	char * localSourceOutputCoreFunctionalities = (char *) malloc (1000 * sizeof(char));
	sprintf(localSourceOutputCoreFunctionalities, "%sNeededSourceCode.c", TXLTemporaryFolder);

	addLocsFromSourceToDestination(skeletonInterfaceSourceCodeOutput,
			localSourceOutputCoreFunctionalities, TXLToolsPath);

	char * localSkeletonCoreFeatureHeader = (char *) malloc (1000 * sizeof(char));
	sprintf(localSkeletonCoreFeatureHeader, "%sNeededFunctionsHeader.h", TXLTemporaryFolder);

	//HERE write to interface
	addLocsFromSourceToDestination(finalGraftInterfaceFile,
			localSkeletonCoreFeatureHeader, TXLToolsPath);
	fflush(stdout);
}

void extractFunctionMarkers(char * sourceFile, char * outputFile,
		char * TXLProgsPath) {
	char * commandTXL;
	commandTXL = (char *) malloc(500 * sizeof(char));
	sprintf(commandTXL, "%sExtractAllFunctionsNameInAFile.x %s %s %s",
			TXLProgsPath, sourceFile, outputFile, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! sExtractAllFunctionsNameInAFile\n");
	}
	free(commandTXL);
}

ListOfString * extractFunctionsAsMarkersFromSourceFile(char * sourceFile, char * TXLTemporaryFolder) {

	char * outputTempFile;
	outputTempFile = (char *) malloc(500 * sizeof(char));
	sprintf(outputTempFile, "%stemp_needed_functions_markers.out",
			TXLTemporaryFolder);

	extractFunctionMarkers(sourceFile, outputTempFile, TXLToolsPath);
	ListOfString * result = returnListOfStringFromMarkersFile(outputTempFile);

	return result;
}

void extractNeededFunctionsAndGlobalDeclarationsForSelectedLOCs(
		char * sourceOutputCoreFunctionality,
		CallDependency * ListOfNeededFunctionsForAllLOCs,
		GlobalDeclarations ListOfFinalGlobalDeclarations,
		DependencyList * GlobalDependencyList,
		DependencyList * SkeletonDependenciesOnGlobals,
		char * tempGlobalDeclarations,
		char * definedGlobalTypesAndVariablesFile,
		ListOfString * candidateSourceCode,
		ListOfString * TheListOfTransplantedMarkers,
		char * outputFileGlobalDependenciesOnFunctions,
		char * TXLTemporaryFolder) {
	//needed for extraction of global dependencies for core feature
	ListOfString * FinalListOfTransplantedFunctionsAsMarkersOld = NULL;
	//first add the feature markerd for extraction, since globals for this too must be added
	char * GraftFunctionMarker;
	GraftFunctionMarker = (char *) malloc(500 * sizeof(char));
	sprintf(GraftFunctionMarker, "/* %s */", GraftFunctionName);
	addNewString(&FinalListOfTransplantedFunctionsAsMarkersOld,
			GraftFunctionMarker);

	//to be checked not to add all functions for all globals, but just for one possible to be transplanted!!!
	CallDependency * ListOfNeededFunctionsForGlobals =
			extractNeededFunctionsForGlobals(ListOfFinalGlobalDeclarations,
					outputFileGlobalDependenciesOnFunctions);

	extractCoreFunctionsCalledInIndividual(ListOfNeededFunctionsForAllLOCs,
			candidateSourceCode, SkeletonCoreFeatureHeader,
			sourceOutputCoreFunctionality,
			&FinalListOfTransplantedFunctionsAsMarkersOld,
			ListOfNeededFunctionsForGlobals, TXLTemporaryFolder);

	fflush(stdout);

	ListOfString * FinalListOfTransplantedFunctionsAsMarkers =
			extractFunctionsAsMarkersFromSourceFile(
					sourceOutputCoreFunctionality, TXLTemporaryFolder);

	//for the source core output file extract all the called files
	//this will be the final list of transplated functions as markers

	//ListOfString * TempListOfStrings = returnListOfStringFromMarkersFile(tempListOfMarkers);

	//FOR ALL EXTRACTED FUNCTIONS, ADD THE DEPENDENCIES!!!

	fflush(stdout);
	//maybe HERE!!!!
	DependencyList * DependencyListGlobalCoreFunctions =
			extractDependencyGlobalListForCoreFunctions(
					SourceOutputCoreFunctionalities, tempGlobalDeclarations,
					definedGlobalTypesAndVariablesFile, TXLTemporaryFolder);
	char * tempHeaderGlobalsContent;
	tempHeaderGlobalsContent = (char *) malloc(500 * sizeof(char));
	sprintf(tempHeaderGlobalsContent, "%stemp_header_decl_for_globals.out",
			TXLTemporaryFolder);
	char * tempSourceCodeGlobalsContent;
	tempSourceCodeGlobalsContent = (char *) malloc(500 * sizeof(char));
	sprintf(tempSourceCodeGlobalsContent, "%stemp_source_for_globals.out",
			TXLTemporaryFolder);

	fflush(stdout);

	extractNeededGlobalDeclarationsAndHeaderForSkeletonAndInterface(
			ListOfFinalGlobalDeclarations, GlobalDependencyList,
			SkeletonDependenciesOnGlobals, DependencyListGlobalCoreFunctions,
			TheListOfTransplantedMarkers,
			FinalListOfTransplantedFunctionsAsMarkers,
			tempSourceCodeGlobalsContent, tempHeaderGlobalsContent);

	fflush(stdout);

	addLocsFromSourceToDestination(sourceOutputCoreFunctionality,
			tempSourceCodeGlobalsContent, TXLToolsPath);
	addLocsFromSourceToDestination(SkeletonCoreFeatureHeader,
			tempHeaderGlobalsContent, TXLToolsPath);
}

/*
 double computeFitness(char * IndividualMakeFileTarget, int TotalIDsInHostSymbolTable, int MappedHostIDsInIndividual) {
 double fitness;
 double fitnessForCompile;
 double fitnessForTestCases = 0;
 double fitnessForMappedIDs;

 fitnessForMappedIDs = 0.3 * MappedHostIDsInIndividual / TotalIDsInHostSymbolTable;

 char * command;
 command = (char *) malloc(1000 * sizeof (char));
 sprintf(command, "cd %s \nmake %s", TransplantResultLocation, IndividualMakeFileTarget);

 int result = system(command);
 if (result != 0) {
 fitnessForCompile = 0;
 } else {
 fitnessForCompile = 0.35;

 //just if it compiled is sense in running the test suite!
 char * commandRunTestCases;
 commandRunTestCases = (char *) malloc(500 * sizeof (char));

 sprintf(commandRunTestCases, "%s%s", TransplantResultLocation, IndividualMakeFileTarget);


 FILE *fp;
 char * line;
 line = (char *) malloc(1000 * sizeof (char));

 fp = popen(commandRunTestCases, "r");
 if (fp == NULL) {
 printf("Failed to run command\n");
 fitnessForTestCases = 0;
 } else {
 while (fgets(line, 1000, fp) != NULL) {
 printf("%s", line);
 }
 pclose(fp);

 int fitnessPercentage;
 sscanf(line, "%d", &fitnessPercentage);

 fitnessForTestCases = 0.35 * (fitnessPercentage / 100);

 }
 }
 fitness = fitnessForCompile + fitnessForTestCases + fitnessForMappedIDs;
 return fitness;
 }
 */

void instantiateAnIndividual(ListOfString * SkeletonSourceCode,
		char * GraftInterfaceTempOutput, char * sourceOutputCoreFunctionality,
		char * skeletonInterfaceSourceCodeOutput,
		ArrayOfStrings SkeletonLOCsArray,
		DependencyList * ListOfDependenciesForStatements,
		CallDependency * ListOfNeededFunctionsForAllLOCs,
		GlobalDeclarations ListOfFinalGlobalDeclarations,
		DependencyList * GlobalDependencyList,
		DependencyList * SkeletonDependenciesOnGlobals,
		char * tempGlobalDeclarations,
		char * definedGlobalTypesAndVariablesFile, char * GraftFunctionName,
		char * finalGraftInterfaceFile, int TotalIDsInHostSymbolTable,
		char * individualFileName, char * GlobalFctDependOutput,
		char * TXLTemporaryFolder) {

	int MappedGraftHostIDs;

	char * finalHostInterfaceOutput;
	finalHostInterfaceOutput = (char *) malloc(500 * sizeof(char));
	sprintf(finalHostInterfaceOutput, "%stemp_interface_source_file.out",
			TXLTemporaryFolder);
	ListOfString * TheListOfTransplantedMarkers = NULL;
	ListOfString * candidateSourceCode = generateNewSkeletonForCandidate(
			SkeletonLOCsArray, SkeletonSourceCode, individualFileName,
			ListOfDependenciesForStatements, &TheListOfTransplantedMarkers,
			&MappedGraftHostIDs, TXLTemporaryFolder);

	extractNeededFunctionsAndGlobalDeclarationsForSelectedLOCs(
			sourceOutputCoreFunctionality, ListOfNeededFunctionsForAllLOCs,
			ListOfFinalGlobalDeclarations, GlobalDependencyList,
			SkeletonDependenciesOnGlobals, tempGlobalDeclarations,
			definedGlobalTypesAndVariablesFile, candidateSourceCode,
			TheListOfTransplantedMarkers, GlobalFctDependOutput, TXLTemporaryFolder);

	instantiateGraftSkeletonInInterface(candidateSourceCode,
			GraftInterfaceTempOutput, skeletonInterfaceSourceCodeOutput,
			JustCoreFeatureHeaderName);

	addLocsFromSourceToDestination(skeletonInterfaceSourceCodeOutput,
			SourceOutputCoreFunctionalities, TXLToolsPath);
	addLocsFromSourceToDestination(finalGraftInterfaceFile,
			SkeletonCoreFeatureHeader, TXLToolsPath);

	double fitness = computeFitness("Individual.x", TotalIDsInHostSymbolTable,
			MappedGraftHostIDs, 0, TransplantResultLocation);
	printf("Fitness for individual: %s is : %0.2f\n", "Individual.x", fitness);
	fflush(stdout);
}

void instantiateAnIndividualFromGPIndividual(ListOfString * SkeletonSourceCode,
		char * GraftInterfaceTempOutput, char * sourceOutputCoreFunctionality,
		char * skeletonInterfaceSourceCodeOutput,
		ArrayOfStrings SkeletonLOCsArray,
		DependencyList * ListOfDependenciesForStatements,
		CallDependency * ListOfNeededFunctionsForAllLOCs,
		GlobalDeclarations ListOfFinalGlobalDeclarations,
		DependencyList * GlobalDependencyList,
		DependencyList * SkeletonDependenciesOnGlobals,
		char * tempGlobalDeclarations,
		char * definedGlobalTypesAndVariablesFile, char * GraftFunctionName,
		char * finalGraftInterfaceFile, int TotalIDsInHostSymbolTable,
		GPIndividual * individual,
		DependencyListWithID * dependencyListForAbstract,
		char * GlobalDependenciesOnFunctions, char * TXLTemporaryFolder) {

	//int MappedGraftHostIDs;

	char * finalHostInterfaceOutput;
	finalHostInterfaceOutput = (char *) malloc(500 * sizeof(char));
	sprintf(finalHostInterfaceOutput, "%stemp_interface_source_file.out",
			TXLTemporaryFolder);
	ListOfString * TheListOfTransplantedMarkers = NULL;
	ListOfString * candidateSourceCode =
			generateNewSkeletonForCandidateFromGPIndividual(SkeletonLOCsArray,
					SkeletonSourceCode, individual,
					ListOfDependenciesForStatements,
					&TheListOfTransplantedMarkers, dependencyListForAbstract, TXLTemporaryFolder);

	extractNeededFunctionsAndGlobalDeclarationsForSelectedLOCs(
			sourceOutputCoreFunctionality, ListOfNeededFunctionsForAllLOCs,
			ListOfFinalGlobalDeclarations, GlobalDependencyList,
			SkeletonDependenciesOnGlobals, tempGlobalDeclarations,
			definedGlobalTypesAndVariablesFile, candidateSourceCode,
			TheListOfTransplantedMarkers, GlobalDependenciesOnFunctions,
			TXLTemporaryFolder);

	instantiateGraftSkeletonInInterface(candidateSourceCode,
			GraftInterfaceTempOutput, skeletonInterfaceSourceCodeOutput,
			JustCoreFeatureHeaderName);

	addLocsFromSourceToDestination(skeletonInterfaceSourceCodeOutput,
			SourceOutputCoreFunctionalities, TXLToolsPath);
	addLocsFromSourceToDestination(finalGraftInterfaceFile,
			SkeletonCoreFeatureHeader, TXLToolsPath);

	double fitness = computeFitness("Individual.x", TotalIDsInHostSymbolTable,
			0, 0, TransplantResultLocation);
	printf("Fitness for individual: %s is : %0.2f\n", "Individual.x", fitness);
	fflush(stdout);
}

void completeInstantiation(ListOfString * SkeletonSourceCode,
		char * GraftInterfaceTempOutput, char * sourceOutputCoreFunctionality,
		char * skeletonInterfaceSourceCodeOutput,
		ArrayOfStrings SkeletonLOCsArray,
		DependencyList * ListOfDependenciesForStatements,
		CallDependency * ListOfNeededFunctionsForAllLOCs,
		GlobalDeclarations ListOfFinalGlobalDeclarations,
		DependencyList * GlobalDependencyList,
		DependencyList * SkeletonDependenciesOnGlobals,
		char * tempGlobalDeclarations,
		char * definedGlobalTypesAndVariablesFile, char * GraftFunctionName,
		char * finalGraftInterfaceFile, int TotalIDsInHostSymbolTable,
		char * GlobalDependenciesOnFunctions,
		char * TXLTemporaryFolder) {

	ListOfString * TheListOfTransplantedMarkers = createTheListOfAllMarkers(
			SkeletonSourceCode, TXLTemporaryFolder);

	extractNeededFunctionsAndGlobalDeclarationsForSelectedLOCs(
			sourceOutputCoreFunctionality, ListOfNeededFunctionsForAllLOCs,
			ListOfFinalGlobalDeclarations, GlobalDependencyList,
			SkeletonDependenciesOnGlobals, tempGlobalDeclarations,
			definedGlobalTypesAndVariablesFile, SkeletonSourceCode,
			TheListOfTransplantedMarkers, GlobalDependenciesOnFunctions,
			TXLTemporaryFolder);

	//this is the place for GP!!!
	//for loop instantiation for all population, params for instantiateGPCandidateWithExistingGlobalAndCoreFeature
	//are send to GP algorithm

	int MappedGraftHostIDs;
	MappedGraftHostIDs = instantiateGPCandidateWithExistingGlobalAndCoreFeature(
			SkeletonSourceCode, SkeletonLOCsArray, "individual.in",
			ListOfDependenciesForStatements, GraftInterfaceTempOutput,
			skeletonInterfaceSourceCodeOutput, finalGraftInterfaceFile, TXLTemporaryFolder);

	double fitness = computeFitness("Individual.x", TotalIDsInHostSymbolTable,
			MappedGraftHostIDs, 0, TransplantResultLocation);
	printf("Fitness for individual: %s is : %0.2f\n", "Individual.x", fitness);
	fflush(stdout);
}
