#include "PrepareForGP.h"
#include "Mapping.h"
#include "MappingCandidate.h"
#include "DependencyList.h"
#include "GenTrans.h"
#include "FeatureInterface.h"
#include "Skeleton.h"
#include "SymbolTable.h"
#include "coreFunctionExtractor.h"
#include "reverseCallGraph.h"
#include "folderContent.h"
#include "MarkFunctionCall.h"
#include "ListOfString.h"
#include "FunctionCallDependency.h"
#include "Utils.h"
#include "FunctionElement.h"
#include "DeclarationSymbolTable.h"
#include "GlobalDeclarations.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//send the command for generating the simbol table, and then call the previous method for putting
//it in the SymbolTable structure

SymbolTable * readFromFileHostSymbolTable(char * inputFile, char * txlFile,
		char * symbolTableFile) {
	char * command;
	command = (char *) malloc(500 * sizeof(char));
	sprintf(command, "%s%s %s %s %s", TXLToolsPath, txlFile, inputFile,
			symbolTableFile, ConsoleNullOutput);
	int status = system(command);
	if (status != 0) {
		printf("ERROR! readFromFileHostSymbolTable \n");
		return NULL;
	}
	FILE *fin = fopen(symbolTableFile, "r");
	SymbolTable * symbolTable = readSymbolTableForHost(fin);
	fclose(fin);
	free(command);

    if(is_demo){
        char * cmdCopySymbolTable = (char*) malloc (1000 * sizeof(char));
        sprintf(cmdCopySymbolTable, "cp %s %sHostSymbolTable.out", symbolTableFile, demoPath);
		system(cmdCopySymbolTable);

#ifdef DETAILED_DEMO
		printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the host symbol table: %sHostSymbolTable.out. \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath);
        getchar();
#endif

    }

	return symbolTable;
}

//send the command for generating the simbol table, and then call the previous method for putting
//it in the SymbolTable structure

SymbolTable * readFromGraftSymbolTable(char * inputFile, char * txlFile,
		char * symbolTableFile, char * graftFunctionName) {
	char * ifdefFile;
	char * hostFile;

	if (strcmp(TXLToolsPath, "./")) {
		ifdefFile = (char *) malloc((strlen(TXLToolsPath) + 10) * sizeof(char));
		sprintf(ifdefFile, "%sifdef.x", TXLToolsPath);
	} else {
		ifdefFile = (char *) malloc(10 * sizeof(char));
		sprintf(ifdefFile, "./ifdef.x");
	}
	hostFile = transplantHostInputFile;

	char command[1000];
	sprintf(command, "%s%s %s %s %s %s %s %s", TXLToolsPath, txlFile, hostFile,
			graftFunctionName, symbolTableFile, ifdefFile, inputFile,
			ConsoleNullOutput);

	int status = system(command);
	if (status != 0) {
		printf("ERROR! readFromGraftSymbolTable\n");
		return NULL;
	}
	FILE *fin = fopen(symbolTableFile, "r");
	SymbolTable * symbolTable = readSymbolTable(fin);
	fclose(fin);

	return symbolTable;
}

void createFinalHostInterface(char * interfaceFile, char * coreFeatureFile,
		char * TXLProgsPath, char * finalTransplantLocation) {
	char * commandTXL;
	commandTXL = (char *) malloc(500 * sizeof(char));
	sprintf(commandTXL, "%saddCoreFeatureToInterface.x %s %s %s %s",
			TXLProgsPath, interfaceFile, coreFeatureFile,
			finalTransplantLocation, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! MutationCandidateExtractor.c\n");
	}
	free(commandTXL);
}

char * statementBeginning = "/* STM:";
char * declBeginning = "/* DECL:";
char * compoundBeginning = "/* COMPOUND:";

void addMissingLabels(ListOfString * skeletonSourceCode, char * allLabelsFile) {

	ListOfString * listOfUsedLabels = NULL;

	ListOfString * listOfDefinedLabels = NULL;
	//not used any more the list from the TXL program.
	// compute here the list of used labels!!!

	listOfDefinedLabels = readListOfStringFromFile(allLabelsFile);
	//remove the ones that are already existing in the skeleton
	ListOfString * aux = skeletonSourceCode;
	while (aux) {
		//this statement contains a label
		if (strstr(aux->el, "goto LABEL_")) {

			//first case when we have a marker
			if (strstr(aux->el, "/* COMPOUND:")) {
				char * gotoMarker = (char *) malloc(10 * sizeof(char));
				char * labelMarker = (char *) malloc(2000 * sizeof(char));
				int compNo;
				sscanf(aux->el, "%*s %*s %*s %*s %s %s ", gotoMarker,
						labelMarker);

				//sscanf(aux->el, "%s %s", gotoMarker, labelMarker);
				labelMarker[strlen(labelMarker) - 1] = 0;
				sprintf(labelMarker, "%s :", labelMarker);

				if (!searchStringInListOfStrings(listOfUsedLabels,
						labelMarker)) {
					addNewString(&listOfUsedLabels, labelMarker);
				}
			} else {
				// we do not have a marker
				char * gotoMarker = (char *) malloc(10 * sizeof(char));
				char * labelMarker = (char *) malloc(2000 * sizeof(char));

				sscanf(aux->el, "%s %s", gotoMarker, labelMarker);
				labelMarker[strlen(labelMarker) - 1] = 0;
				sprintf(labelMarker, "%s :", labelMarker);

				if (!searchStringInListOfStrings(listOfUsedLabels,

				labelMarker)) {

					addNewString(&listOfUsedLabels, labelMarker);

				}
			}

		}

		if (listOfUsedLabels) {
			removeStringLabels(&listOfUsedLabels, aux->el);
		}
		aux = aux->next;
	}

	int signal = 0;
	ListOfString * auxSkeleton = skeletonSourceCode;

	if (listOfUsedLabels) {
		// No need to check here if auxSkeleton->next exist. If there was no problem up to here it should always exist
		while (auxSkeleton->next && !signal) {
			if (strstr(auxSkeleton->next->el, RETURN_MARKER)) {
				aux = auxSkeleton->next;
				auxSkeleton->next = listOfUsedLabels;
				while (listOfUsedLabels->next) {
					listOfUsedLabels = listOfUsedLabels->next;
				}
				listOfUsedLabels->next = aux;
				signal = 1;
			}

			auxSkeleton = auxSkeleton->next;
		}
	}
}

void solveDonorStaticFunctionDeclarationsUniqueNamesAndMarkersReplacement(
		char * donorSourceFiles, char * TXLTemporaryFolder) {
	//AcceptedFunctionsForGP GraftFunctionName

	char * tempSourceFilesOutput = (char *) malloc(1000 * sizeof(char));

	sprintf(tempSourceFilesOutput, "%stemp_donor_source_file_for_static.out",
			TXLTemporaryFolder);
	FILE * outputDonorSourceFiles = fopen(tempSourceFilesOutput, "w");

	readFilesFromDir(donorSourceFiles, outputDonorSourceFiles, 'c');
	fclose(outputDonorSourceFiles);

	char * tempCoreFctOutputFile = (char *) malloc(1000 * sizeof(char));
	sprintf(tempCoreFctOutputFile, "%stemp_new_core_fct_name.out",
			TXLTemporaryFolder);
	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));
	sprintf(commandTXL, "%sreplaceStaticFunctions.x %s %s %s %s %s %s",
			TXLToolsPath, tempSourceFilesOutput, AcceptedFunctionsForGP,
			AcceptedFunctionsForGP, GraftFunctionName, tempCoreFctOutputFile,
			ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR replace static function!");
	}
	free(commandTXL);
	char * newTargetFunctionName = (char *) malloc(100 * sizeof(char));
	FILE * finCoreFctName = fopen(tempCoreFctOutputFile, "r");
	fscanf(finCoreFctName, "%s", newTargetFunctionName);
	free(GraftFunctionName);
	GraftFunctionName = newTargetFunctionName;
}

ListOfString * prepareForGP(Mapping ** ListOfMappingsFromCurrentDonor,
		DependencyList ** ListOfDependenciesForStatements,
		MappingCandidate ** MappingCandidatesForGP,
		CallDependency ** ListOfNeededFunctionsForAllLOCs,
		DeclarationSymbolTable ** FinalDeclSymbolTable,
		char * skeletonInterfaceSourceCodeOutput,
		char * skeletonInterfaceHeaderFileOutput,
		char * headerForInterfaceJustName, char * skeletonCoreFeatureHeader,
		char * hostWithInterfaceOutputFile, char * interfaceOutput,
		char * sourceOutputCoreFunctionality, char * graftSourceCodeFile,
		char * graftFunctionName, int * countOfHostIDs,
		SymbolTable ** finalHostSymbolTable, char * outputDefinedFunctions,
		char * outputFileLabels, char * TXLTemporaryFolder) {

	//first prepare the donor, by solving the static functions declarations!

	SymbolTable * HostSymbolTable = NULL;

	//char * graftFunctionName = NULL;
	char * symbolTableFileHost;
	char * symbolTableFileGraft;
	symbolTableFileHost = (char *) malloc(500 * sizeof(char));
	symbolTableFileGraft = (char *) malloc(500 * sizeof(char));

	sprintf(symbolTableFileHost, "%stemp_symbol_table_host.out",
			TXLTemporaryFolder);
	sprintf(symbolTableFileGraft, "%stemp_symbol_table_graft.out",
			TXLTemporaryFolder);

	HostSymbolTable = readFromFileHostSymbolTable(transplantHostInputFile,
			txlFileHost, symbolTableFileHost);

	//count the IDs in host symbol table for fitness computation
	*countOfHostIDs = countIDsInSymbolTable(HostSymbolTable);
	//SymbolTable * GraftSymbolTable = readFromGraftSymbolTable(inputFileGraft, txlFileGraft, symbolTableFileGraft,
	//      graftFunctionName);

	fflush(stdout);

	generateHostInterface(HostSymbolTable, TXLTemporaryFolder, TXLToolsPath,
			inputFileGraft, "GRAFT_INTERFACE", "TRANSPLANT_RESULT",
			hostWithInterfaceOutputFile, skeletonInterfaceHeaderFileOutput,
			transplantHostInputFile, interfaceOutput,
			headerForInterfaceJustName, graftFunctionName);

	fflush(stdout);

	//char parametersTransplant [500];
	//strcpy(parametersTransplant, inputFileGraft);
	//strcat(parametersTransplant, " % ");
	//strcat(parametersTransplant, generateParametersForTransplant(HostSymbolTable, GraftSymbolTable));
	extractCoreFunctionality(graftFunctionName, graftSourceCodeFile,
			donorSourceFileFolder, transplantWorkFolderMainThread, TXLToolsPath,
			TXLTemporaryFolder, TXLTemporaryFolder, skeletonCoreFeatureHeader,
			sourceOutputCoreFunctionality);

	fflush(stdout);

	char * listOfNeededFilesForCoreGraftFunction;
	listOfNeededFilesForCoreGraftFunction = (char *) malloc(500 * sizeof(char));
	strcpy(listOfNeededFilesForCoreGraftFunction, TXLTemporaryFolder);
	strcat(listOfNeededFilesForCoreGraftFunction,
			"temp_list_of_header_files.out");
	//generateNewProgram(transplantHostInputFile, txlFileTransplant, parametersTransplant,
	//      justPathForFileLocation, listOfNeededFilesForCoreGraftFunction);

	char * modifiedFunctionLOCs = (char *) malloc(
			(strlen(TXLTemporaryFolder) + 20) * sizeof(char));
	sprintf(modifiedFunctionLOCs, "%soutputFCTLOCS.out", TXLTemporaryFolder);

	char * skeletonInstantiationMappings = (char *) malloc(
			(strlen(TXLTemporaryFolder) + 40) * sizeof(char));
	sprintf(skeletonInstantiationMappings, "%sSkeletonInstantiateMappings.out",
			TXLTemporaryFolder);

	char * HostSymbolTableFile = (char *) malloc(
			(strlen(TXLTemporaryFolder) + 40) * sizeof(char));
	sprintf(HostSymbolTableFile, "%sHostSymbolTable.out", TXLTemporaryFolder);

	char * CurrentExistingMappings = (char *) malloc(
			(strlen(TXLTemporaryFolder) + 40) * sizeof(char));
	sprintf(CurrentExistingMappings, "%sCurrentExistingMappings.out",
			TXLTemporaryFolder);

	ListOfString * reverseCallGraph = NULL;

	fflush(stdout);

	generateReverseCallGraph(TXLToolsPath, donorSourceFileFolder,
			transplantWorkFolderMainThread, modifiedFunctionLOCs,
			skeletonInstantiationMappings, HostSymbolTableFile,
			CurrentExistingMappings, TXLTemporaryFolder,
			transplantHostInputFile, graftFunctionName, &reverseCallGraph,
			outputFileLabels);

	fflush(stdout);

	//final symbol table from the graft
	SymbolTable * finalGraftSymbolTable = NULL;
	*ListOfMappingsFromCurrentDonor = createGraftSkeleton(modifiedFunctionLOCs,
			skeletonInstantiationMappings, TXLToolsPath, SkeletonFile,
			CurrentExistingMappings, TXLTemporaryFolder, HostSymbolTableFile,
			&finalGraftSymbolTable, &(*ListOfDependenciesForStatements),
			reverseCallGraph, TXLTemporaryFolder);

	fflush(stdout);

	*MappingCandidatesForGP = createMappingCandidateFromSymbolTablesAndMappings(
			finalGraftSymbolTable, HostSymbolTable,
			*ListOfMappingsFromCurrentDonor);

	fflush(stdout);

	char * FinalSkeletonOutput;
	FinalSkeletonOutput = (char *) malloc(500 * sizeof(char));
	sprintf(FinalSkeletonOutput, "%stemp_final_skeleton_output.out",
			TXLTemporaryFolder);

	fflush(stdout);
	//remove all LOCs for which the called function were not specified by the user for transplant
	*ListOfNeededFunctionsForAllLOCs =
			removeUnwantedFunctionsAndReturnCallDependencies(SkeletonFile,
					FinalSkeletonOutput, *ListOfDependenciesForStatements,
					outputDefinedFunctions, TXLTemporaryFolder);

	fflush(stdout);

	ListOfString * finalListOfSkeletonSourceCode = readListOfStringFromFile(
			FinalSkeletonOutput);

    if(is_demo){
        char * cmdCpDemoFiles = (char *) malloc (1000 * sizeof(char));
        sprintf(cmdCpDemoFiles,"cp %s %sInlinedVeinSkeleton.c", FinalSkeletonOutput, demoPath);
        system(cmdCpDemoFiles);

		char * outFileMappings = (char *) malloc (1000 * sizeof(char));
		sprintf(outFileMappings, "%sOrganMatchingTable.out", demoPath);

		printAMappingCandidateList(outFileMappings, *MappingCandidatesForGP);


#ifdef DETAILED_DEMO
		printf("You are running MuScalpel in detailed demo mode. \nMuScalpel generated the Inlined Vein Skeleton source file: %sInlinedVeinSkeleton.c; and the organ matching table file: %sOrganMatchingTable.out . \nPress return when you are ready to run the GP algorithm.\n",
        	demoPath, demoPath);
        getchar();
#endif

    }


	//compileAndExecuteNewProgram(compileCommand, outputFileName, transplantCFile);

	fflush(stdout);

	*FinalDeclSymbolTable = GenerateDeclarationsSymbolTable(
			finalListOfSkeletonSourceCode, *ListOfDependenciesForStatements,
			HostSymbolTable, TXLTemporaryFolder);

	*finalHostSymbolTable = HostSymbolTable;

	freeSymbolTableMemory();
	//free(graftFunctionName);
	free(symbolTableFileHost);
	free(symbolTableFileGraft);
	free(listOfNeededFilesForCoreGraftFunction);
	//freeListOfSymbolTable(&HostSymbolTable);
	//freeListOfSymbolTable(&GraftSymbolTable);

	//add the missing labels in the skeleton

	addMissingLabels(finalListOfSkeletonSourceCode, outputFileLabels);

	return finalListOfSkeletonSourceCode;
}

void extractStmOrCompoundMarkers(char * Skeleton, char * outputFile,
		char * TXLProgsPath, char * outputFileAbstractMarkers) {
	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));
	sprintf(commandTXL, "%sMutationCandidateExtractor.x %s %s %s %s",
			TXLProgsPath, Skeleton, outputFile, outputFileAbstractMarkers,
			ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! sMutationCandidateExtractor.x\n");
	}
	free(commandTXL);
}

void extractAllLineMarkersMarkers(char * Skeleton, char * outputFile,
		char * TXLProgsPath) {
	char * commandTXL;
	commandTXL = (char *) malloc(5000 * sizeof(char));
	sprintf(commandTXL, "%sAllLineMarkersExtractor.x %s %s %s", TXLProgsPath,
			Skeleton, outputFile, ConsoleNullOutput);
	int status = system(commandTXL);
	if (status != 0) {
		printf("ERROR! AllLineMarkersExtractor.x\n");
	}
	free(commandTXL);
}

ListOfString * createTheListOfAllMarkers(ListOfString * SkeletonSourceCode,
		char * TXLTemporaryFolder) {

	//char * outputFileAbstractMarkers;
	//outputFileAbstractMarkers = (char *) malloc(500 * sizeof (char));
	//sprintf(outputFileAbstractMarkers, "%stemp_markers_for_abstract_vars.out", TXLTemporaryFolder);

	char * skeletonTemporaryFile;
	skeletonTemporaryFile = (char *) malloc(5000 * sizeof(char));
	sprintf(skeletonTemporaryFile,
			"%stemp_skeleton_for_mutation_candidates.out", TXLTemporaryFolder);
	printAListOfStrings(SkeletonSourceCode, skeletonTemporaryFile);
	char * tempListOfMarkers;
	tempListOfMarkers = (char *) malloc(5000 * sizeof(char));
	sprintf(tempListOfMarkers, "%stemp_file_of_line_markers.out",
			TXLTemporaryFolder);
	extractAllLineMarkersMarkers(skeletonTemporaryFile, tempListOfMarkers,
			TXLToolsPath);

	ListOfString * TempListOfStrings = returnListOfStringFromMarkersFile(
			tempListOfMarkers);

	//maybe here output also the list of abstract markers

	free(tempListOfMarkers);
	return TempListOfStrings;
}

ArrayOfStrings createArrayOfLineMutationCandidates(
		ListOfString * SkeletonSourceCode,
		ListOfString ** MarkersForLineWithAbstract, char * TXLTemporaryFolder) {
	char * outputFileAbstractMarkers;
	outputFileAbstractMarkers = (char *) malloc(500 * sizeof(char));
	sprintf(outputFileAbstractMarkers, "%stemp_markers_for_abstract_vars.out",
			TXLTemporaryFolder);

	char * skeletonTemporaryFile;
	skeletonTemporaryFile = (char *) malloc(500 * sizeof(char));
	sprintf(skeletonTemporaryFile,
			"%stemp_skeleton_for_mutation_candidates.out", TXLTemporaryFolder);
	printAListOfStrings(SkeletonSourceCode, skeletonTemporaryFile);
	char * tempListOfMarkers;
	tempListOfMarkers = (char *) malloc(500 * sizeof(char));
	sprintf(tempListOfMarkers, "%stemp_file_of_line_markers.out",
			TXLTemporaryFolder);
	extractStmOrCompoundMarkers(skeletonTemporaryFile, tempListOfMarkers,
			TXLToolsPath, outputFileAbstractMarkers);

	ListOfString * TempListOfStrings = returnListOfStringFromMarkersFile(
			tempListOfMarkers);

	*MarkersForLineWithAbstract = returnListOfStringFromMarkersFile(
			outputFileAbstractMarkers);

	free(tempListOfMarkers);
	int numberOfElements = countElementsInListOfString(TempListOfStrings);
	char ** arrayOfStrings;
	arrayOfStrings = (char **) malloc(numberOfElements * sizeof(char *));
	for (int i = 0; i < numberOfElements; i++) {
		arrayOfStrings[i] = (char *) malloc(100 * sizeof(char));
	}
	int count = 0;
	ArrayOfStrings arrayOfMarkers;
	arrayOfMarkers.count = numberOfElements;
	arrayOfMarkers.array = arrayOfStrings;
	ListOfString * auxListOfStr = TempListOfStrings;
	while (auxListOfStr) {
		strcpy(arrayOfMarkers.array[count], auxListOfStr->el);
		count++;
		auxListOfStr = auxListOfStr->next;
	}
	arrayOfMarkers.count = count;
	freeListOfStrings(&TempListOfStrings);
	return arrayOfMarkers;
	/*
	 for(int i = 0; i < 5000; i++){
	 free(arrayOfStrings [i]);
	 }
	 free(arrayOfStrings);
	 */
}

ArrayOfStrings copyArrayOfStrings(ArrayOfStrings initial) {
	ArrayOfStrings newArrayOfStrings;
	newArrayOfStrings.count = initial.count;
	char ** arrayOfStrings;
	arrayOfStrings = (char **) malloc(initial.count * sizeof(char *));
	for (int i = 0; i < initial.count; i++) {
		arrayOfStrings[i] = (char *) malloc(100 * sizeof(char));
		strcpy(arrayOfStrings[i], initial.array[i]);
	}
	newArrayOfStrings.array = arrayOfStrings;
	return newArrayOfStrings;
}
