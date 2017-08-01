/* 
 * File:   main.c
 * Author: alex
 *
 * Created on 26 May 2014, 03:56
 */

//
//  main.c
//  ResearchProject
//
//  Created by Marginean Alexandru on 27/02/14.
//  Copyright (c) 2014 Marginean Alexandru. All rights reserved.
//

#include <config.h>

#include "PrepareForGP.h"
#include "GenTrans.h"
#include "FunctionCallDependency.h"
#include "FunctionElement.h"
#include "Skeleton.h"
#include "GlobalDeclarations.h"
#include "DependencyList.h"
#include "DependencyListWithID.h"

#include "GIReduction.h"

#include "IndividualInstantiator.h"
#include "DeclarationSymbolTable.h"
#include "GP.h"
#include "OptionParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <config.h>

char * hostWithInterfaceOutputFile;
extern int errno;

char * ConsoleNullOutput = " > /dev/null 2>&1";

//this function calls the TXL program for generating a transplant candidate

void generateNewProgram(char * inputFile, char * txlFile, char * parameters,
		char * outputFile, char * listOfNeededIncludeFiles) {
	char * command;
	command = malloc(400 * sizeof(char *));
	sprintf(command, "%s %s %s %s %s %s", TXLToolsPath, txlFile, inputFile,
			outputFile, parameters, ConsoleNullOutput);
	int status = system(command);
	if (status != 0) {
		printf("ERROR");
	}

	char *command2;
	command2 = (char *) malloc(500 * sizeof(char));
	sprintf(command2, "%s%s %s %s %s %s", TXLToolsPath,
			"multiplFiles.x -s 1000", inputFile, "graft.c",
			listOfNeededIncludeFiles, ConsoleNullOutput);
	status = system(command2);
	if (status != 0) {
		printf("ERROR");
	}
	//copyNeededFiles(listOfNeededIncludeFiles);
}

//this function compiles the resulted program and then executes it

void compileAndExecuteNewProgram(char * pathToFolderCompileCommand,
		char * outputFileName, char * transplantCFile) {
	char command[400];
	sprintf(command, "%s %s %s %s %s", TXLToolsPath, pathToFolderCompileCommand,
			outputFileName, transplantCFile, ConsoleNullOutput);
	int status = system(command);
	if (status != 0) {
		printf("ERROR");
	}
	char PathForCommand[250];
	strcpy(PathForCommand, "cd ");
	strcat(PathForCommand, TXLToolsPath);
	strcat(PathForCommand, "Transplant/ \n ");
	strcat(PathForCommand, "./transplantedResults");
	system(PathForCommand);
}

//this function generates the list of all defined symbols at the point of insertion
//this list is neded in the TLX program that generates the transplant candidate

char * generateAllDefinedSymbolsInHost(SymbolTable * hostSymbols) {
	char * parameters;
	parameters = (char*) malloc(300 * sizeof(char));
	while (hostSymbols) {
		while (hostSymbols->ListOfIdentifiers) {
			strcat(parameters, hostSymbols->ListOfIdentifiers->ID);
			strcat(parameters, " ");
			hostSymbols->ListOfIdentifiers =
					hostSymbols->ListOfIdentifiers->next;
		}
		hostSymbols = hostSymbols->next;
	}
	return parameters;
}

//this function just takes the first variable name from the host, which is the same type as
//the one in the graft, for mapping them
//for the moment it is checked just the first one
//here will be inserted the logic for generating multiple candidates

char* generateParametersForTransplant(SymbolTable * hostSymbols,
		SymbolTable * graftSymbols) {
	char *parameters;
	parameters = (char*) malloc(300 * sizeof(char));
	while (graftSymbols) {
		while (graftSymbols->ListOfIdentifiers) {
			strcat(parameters, graftSymbols->ListOfIdentifiers->ID);
			strcat(parameters, " ");
			char typeOfVarInGraft[250];
			strcpy(typeOfVarInGraft, graftSymbols->TypeOfVar);

			SymbolTable * newHostHead = hostSymbols;
			while (strcmp(newHostHead->TypeOfVar, typeOfVarInGraft)
					&& newHostHead->next) {
				newHostHead = newHostHead->next;
			}
			strcat(parameters, newHostHead->ListOfIdentifiers->ID);
			strcat(parameters, " ");
			graftSymbols->ListOfIdentifiers =
					graftSymbols->ListOfIdentifiers->next;
		}
		graftSymbols = graftSymbols->next;
	}
	strcat(parameters, "+ ");
	strcat(parameters, generateAllDefinedSymbolsInHost(hostSymbols));
	return parameters;
}

void createADirIfItNotExists(char * name) {
	int e;
	struct stat sb;

	e = stat(name, &sb);
	printf("e=%d errno=%d\n", e, errno);
	if (e == 0) {
		if (sb.st_mode &
#if OS == Linux
				__S_IFDIR
#elif OS == MacOS
				S_IFDIR
#endif
				)
			printf("%s is a directory.\n", name);
		if (sb.st_mode &
#if OS == Linux
				__S_IFREG
#elif OS == MacOS
				S_IFREG
#endif
		)
			printf("%s is a regular file.\n", name);
		// etc.
	} else {
		printf("stat failed.\n");
		if (errno == ENOENT) {
			printf("The directory does not exist. Creating new directory...\n");
			// Add more flags to the mode if necessary.
			e = mkdir(name, S_IRWXU);
			if (e != 0) {
				printf("mkdir failed; errno=%d\n", errno);
			} else {
				printf("created the directory %s\n", name);
			}
		}
	}
}

void cleanTransplantWorkFolder(char * workFolder) {
}

void freeGlobalVariables() {
	free(TXLTemporaryFolderMainThread);
	free(SkeletonFile);
	free(TransplantResultLocation);
	free(ConsoleNullOutput);
	free(AcceptedFunctionsForGP);
	free(SourceOutputCoreFunctionalities);
	free(SkeletonCoreFeatureHeader);
}

void generateMakeFile(char * checkWithMainMethodFile,
		char * InterfaceWithCoreFeature, char * interfaceHeader,
		char * programOutput) {
	ListOfString * makefileLines = NULL;

	char * lineCFLAGS;
	char * testFgets = NULL;

	if (has_cflags) {
		FILE * finCFLAGS = fopen(CFLAGS_FILE, "r");

		lineCFLAGS = (char *) malloc(1000 * sizeof(char));

		testFgets = (char *) malloc(1000 * sizeof(char));
		testFgets = fgets(lineCFLAGS, 999, finCFLAGS);

		fclose(finCFLAGS);
	}

	if (!testFgets) {

		addNewString(&makefileLines,
				"CHECKPACKAGE = `pkg-config --cflags --libs check`\n\n");

		char * checkWithMainMethodObjectFile = NULL;
		char * interfaceWithCoreFeatureObjectFile = NULL;

		checkWithMainMethodObjectFile = (char *) malloc(500 * sizeof(char));
		interfaceWithCoreFeatureObjectFile = (char *) malloc(
				500 * sizeof(char));

		strcpy(checkWithMainMethodObjectFile, checkWithMainMethodFile);
		strtok(checkWithMainMethodObjectFile, ".");
		strcat(checkWithMainMethodObjectFile, ".o");

		strcpy(interfaceWithCoreFeatureObjectFile, InterfaceWithCoreFeature);
		strtok(interfaceWithCoreFeatureObjectFile, ".");
		strcat(interfaceWithCoreFeatureObjectFile, ".o");

		char * line;
		line = (char *) malloc(1000 * sizeof(char));

		sprintf(line, "%s:\t%s %s\n", programOutput,
				checkWithMainMethodObjectFile,
				interfaceWithCoreFeatureObjectFile);
		addNewString(&makefileLines, line);

		sprintf(line, "\tgcc -std=gnu99 -o %s %s %s -lm $(CHECKPACKAGE)\n\n",
				programOutput, checkWithMainMethodObjectFile,
				interfaceWithCoreFeatureObjectFile);

		addNewString(&makefileLines, line);

		sprintf(line, "%s:\t%s\n", checkWithMainMethodObjectFile,
				checkWithMainMethodFile);
		addNewString(&makefileLines, line);
		sprintf(line, "\tgcc -std=gnu99 -c -o %s %s\n\n",
				checkWithMainMethodObjectFile, checkWithMainMethodFile);
		addNewString(&makefileLines, line);

		sprintf(line, "%s:\t%s %s\n", interfaceWithCoreFeatureObjectFile,
				interfaceHeader, InterfaceWithCoreFeature);
		addNewString(&makefileLines, line);
		sprintf(line, "\tgcc -std=gnu99 -c -o %s %s\n\n",
				interfaceWithCoreFeatureObjectFile, InterfaceWithCoreFeature);
		addNewString(&makefileLines, line);

		char * makefileOutputFile;
		makefileOutputFile = (char *) malloc(500 * sizeof(char));
		sprintf(makefileOutputFile, "%sMakefile", TransplantResultLocation);

		printAListOfStrings(makefileLines, makefileOutputFile);
	} else {

		char * cflagsInMakeFile = (char *) malloc(2000 * sizeof(char));
		sprintf(cflagsInMakeFile, "%s\n", lineCFLAGS);

		addNewString(&makefileLines, cflagsInMakeFile);

		addNewString(&makefileLines,
				"CHECKPACKAGE = `pkg-config --cflags --libs check`\n\n");

		char * checkWithMainMethodObjectFile = NULL;
		char * interfaceWithCoreFeatureObjectFile = NULL;

		checkWithMainMethodObjectFile = (char *) malloc(500 * sizeof(char));
		interfaceWithCoreFeatureObjectFile = (char *) malloc(
				500 * sizeof(char));

		strcpy(checkWithMainMethodObjectFile, checkWithMainMethodFile);
		strtok(checkWithMainMethodObjectFile, ".");
		strcat(checkWithMainMethodObjectFile, ".o");

		strcpy(interfaceWithCoreFeatureObjectFile, InterfaceWithCoreFeature);
		strtok(interfaceWithCoreFeatureObjectFile, ".");
		strcat(interfaceWithCoreFeatureObjectFile, ".o");

		char * line;
		line = (char *) malloc(1000 * sizeof(char));

		sprintf(line, "%s:\t%s %s\n", programOutput,
				checkWithMainMethodObjectFile,
				interfaceWithCoreFeatureObjectFile);
		addNewString(&makefileLines, line);

		sprintf(line,
				"\tgcc -std=gnu99 -o %s %s %s -lm $(CHECKPACKAGE) $(CFLAGS)\n\n",
				programOutput, checkWithMainMethodObjectFile,
				interfaceWithCoreFeatureObjectFile);

		addNewString(&makefileLines, line);

		sprintf(line, "%s:\t%s\n", checkWithMainMethodObjectFile,
				checkWithMainMethodFile);
		addNewString(&makefileLines, line);
		sprintf(line, "\tgcc -std=gnu99 -c -o %s %s $(CFLAGS)\n\n",
				checkWithMainMethodObjectFile, checkWithMainMethodFile);
		addNewString(&makefileLines, line);

		sprintf(line, "%s:\t%s %s\n", interfaceWithCoreFeatureObjectFile,
				interfaceHeader, InterfaceWithCoreFeature);
		addNewString(&makefileLines, line);
		sprintf(line, "\tgcc -std=gnu99 -c -o %s %s $(CFLAGS)\n\n",
				interfaceWithCoreFeatureObjectFile, InterfaceWithCoreFeature);
		addNewString(&makefileLines, line);

		char * makefileOutputFile;
		makefileOutputFile = (char *) malloc(500 * sizeof(char));
		sprintf(makefileOutputFile, "%sMakefile", TransplantResultLocation);

		printAListOfStrings(makefileLines, makefileOutputFile);
	}
}

void listOfMarkersThatContainsAbstractValues(ListOfString * skeletonSourceCode) {
	while (skeletonSourceCode) {
		skeletonSourceCode = skeletonSourceCode->next;
	}
}


int main(int argc, char * argv[]) {

	//char cwd[1024];
	//if (getcwd(cwd, sizeof(cwd)) != NULL)
	//       fprintf(stdout, "Current working dir: %s\n", cwd);
	//   else
	//       perror("getcwd() error");

	//fixEnvironmentAndGlobalVariables(argv);
	parse_args(argc, argv);

	char * skeletonInterfaceSourceCodeOutput;
	char * skeletonInterfaceHeaderFileOutput;

	Mapping * ListOfMappingsFromCurrentDonor = NULL;
	DependencyList * ListOfDependenciesForStatements = NULL;
	MappingCandidate * MappingCandidatesForGP = NULL;
	CallDependency * ListOfNeededFunctionsForAllLOCs = NULL;
	DeclarationSymbolTable * FinalDeclSymbolTable = NULL;
	ListOfString * ListOfMarkersForAbstractVariables = NULL;
	SymbolTable * HostSymbolTable = NULL;

	//TEMP CHECK GI FOR ORGAN

	//char * inputFile = "/home/alex/workspace/ResultsGENTRANS/TransplantsWorkingFolder/TransplantCode/InterfaceWithCoreFeature.c";
	//reduceCandidate(inputFile);
	//return 0;
	//END TEMP

	//FIRST OF ALL SOLVE UNIQUE NAMES + STATIC DECLS + FIX REPLACED NAMES!!!

	solveDonorStaticFunctionDeclarationsUniqueNamesAndMarkersReplacement(
			donorSourceFileFolder, TXLTemporaryFolderMainThread);

	fflush(stdout);

	InterfaceHeaderFileName = (char *) malloc(500 * sizeof(char));
	sprintf(InterfaceHeaderFileName, "InterfaceHeader.h");

	//transplant result, the graft's interface header
	skeletonInterfaceHeaderFileOutput = (char *) malloc(500 * sizeof(char));
	sprintf(skeletonInterfaceHeaderFileOutput, "%s%s", TransplantResultLocation,
			InterfaceHeaderFileName);

	//temporary interface header, needed for holding the interface before an individual instantiation,
	//the interface just with global declarations and graft function definition

	TemporaryInterface = (char *) malloc(500 * sizeof(char));
	sprintf(TemporaryInterface,
			"%stemp_interface_for_individual_instantiation.out",
			TXLTemporaryFolderMainThread);

	//transplant result, the new Host's transplant target file, with transplant added
	hostWithInterfaceOutputFile = (char *) malloc(500 * sizeof(char));
	sprintf(hostWithInterfaceOutputFile, "%sHostWithInterface.c",
			TransplantResultLocation);

	InterfaceWithCoreFeatureFileName = (char *) malloc(500 * sizeof(char));
	sprintf(InterfaceWithCoreFeatureFileName, "InterfaceWithCoreFeature.c");
	//transplant result, the new interface with core feature added
	skeletonInterfaceSourceCodeOutput = (char *) malloc(500 * sizeof(char));
	sprintf(skeletonInterfaceSourceCodeOutput, "%s%s", TransplantResultLocation,
			InterfaceWithCoreFeatureFileName);

	char * interfaceOutput;
	interfaceOutput = (char *) malloc(500 * sizeof(char));
	sprintf(interfaceOutput, "%stemp_InterfaceSourceFile.out",
			TXLTemporaryFolderMainThread);

	//clean and create the source code file of functionalities and header file

	FILE * coreSourceCode = fopen(SourceOutputCoreFunctionalities, "w");
	fclose(coreSourceCode);

	FILE * coreHeaderCode = fopen(SkeletonCoreFeatureHeader, "w");
	fclose(coreHeaderCode);

	//used for fitness computation
	int TotalIDsInHostSymbolTable;

	fflush(stdout);

	char * AllDefinedFunctionsInDonor;
	AllDefinedFunctionsInDonor = (char *) malloc(1000 * sizeof(char));
	sprintf(AllDefinedFunctionsInDonor,
			"%stemp_all_defined_functions_int_donor.out",
			TXLTemporaryFolderMainThread);

	char * AllDefinedLabelsFile = (char *) malloc(2000 * sizeof(char));
	sprintf(AllDefinedLabelsFile, "%stemp_output_labels.out",
			TXLTemporaryFolderMainThread);

	ListOfString * FinalSkeletonSourceCode = prepareForGP(
			&ListOfMappingsFromCurrentDonor, &ListOfDependenciesForStatements,
			&MappingCandidatesForGP, &ListOfNeededFunctionsForAllLOCs,
			&FinalDeclSymbolTable, skeletonInterfaceSourceCodeOutput,
			TemporaryInterface, InterfaceHeaderFileName,
			SkeletonCoreFeatureHeader, hostWithInterfaceOutputFile,
			interfaceOutput, SourceOutputCoreFunctionalities, inputFileGraft,
			GraftFunctionName, &TotalIDsInHostSymbolTable, &HostSymbolTable,
			AllDefinedFunctionsInDonor, AllDefinedLabelsFile,
			TXLTemporaryFolderMainThread);

	//temporary from here; write the mapped variables into a file (the ones from the donor)

	/*
	 FILE * fff = fopen("/home/alex/workspace/test_current_mapps.out", "w");
	 Mapping * auxMappedVars = ListOfMappingsFromCurrentDonor;
	 while(auxMappedVars){


	 fprintf(fff, "%s ",auxMappedVars->destination);
	 auxMappedVars = auxMappedVars->next;
	 }
	 fclose(fff);
	 */
	//GENERATE GLOBAL DEPENDENCY LIST FOR THE SKELETON!!!
	fflush(stdout);

	char * definedGlobalTypesAndVariablesFile;
	definedGlobalTypesAndVariablesFile = (char *) malloc(500 * sizeof(char));
	sprintf(definedGlobalTypesAndVariablesFile,
			"%stemp_global_type_var_declarations.out",
			TXLTemporaryFolderMainThread);

	DependencyList * GlobalDependencyList;
	//ListOfDefinedTypesAndVars * GlobalListOfDefinedTypeVar;
	DependencyList * SkeletonDependenciesOnGlobals;

	//GlobalDeclarations ListOfFinalGlobalDeclarations = constructListOfGlobalDeclarations(&GlobalDependencyList,
	//&GlobalListOfDefinedTypeVar, definedGlobalTypesAndVariablesFile);

	//DependencyList * SkeletonDependenciesOnGlobals = generateGlobalDependencyListForSkeleton(FinalSkeletonSourceCode,
	// definedGlobalTypesAndVariablesFile, ListOfFinalGlobalDeclarations);

	//needed for keeping the global declarations file, to be called for core functions global
	//dependency extractor
	char * tempGlobalDeclarations;
	tempGlobalDeclarations = (char *) malloc(500 * sizeof(char));
	sprintf(tempGlobalDeclarations, "%stemp_global_variable_declarations",
			TXLTemporaryFolderMainThread);

	char * tempDefinesFile;
	tempDefinesFile = (char *) malloc(500 * sizeof(char));
	sprintf(tempDefinesFile, "%stemp_define_file.out",
			TXLTemporaryFolderMainThread);

	char * tempIncludeLibFile;
	tempIncludeLibFile = (char *) malloc(500 * sizeof(char));
	sprintf(tempIncludeLibFile, "%stemp_include_lib_file.out",
			TXLTemporaryFolderMainThread);

	//the output file of needed functions to be extracted for globals!
	char * GlobalFctDependOutput;
	GlobalFctDependOutput = (char *) malloc(1000 * sizeof(char));

	sprintf(GlobalFctDependOutput,
			"%stemp_needed_functions_for_global_declarations.out",
			TXLTemporaryFolderMainThread);

	fflush(stdout);
	GlobalDeclarations ListOfFinalGlobalDeclarations =
			fixGlobalDeclarationsForSkeleton(FinalSkeletonSourceCode,
					&GlobalDependencyList, &SkeletonDependenciesOnGlobals,
					tempGlobalDeclarations, definedGlobalTypesAndVariablesFile,
					tempDefinesFile, tempIncludeLibFile,
					AllDefinedFunctionsInDonor, GlobalFctDependOutput,
					TXLTemporaryFolderMainThread);

	ArrayOfStrings ArrayOfLineMarkers = createArrayOfLineMutationCandidates(
			FinalSkeletonSourceCode, &ListOfMarkersForAbstractVariables,
			TXLTemporaryFolderMainThread);

	//generate the dependency list for abstract values, which should be added iff
	//the line is selected, and the mapping is done to an ID in the donor
	DependencyListWithID * dependencyListForAbstract = NULL;
	fflush(stdout);
	dependencyListForAbstract = dependencyListWithIDForDeclWithAbstractValues(
			ListOfMappingsFromCurrentDonor, FinalSkeletonSourceCode,
			ListOfMarkersForAbstractVariables, TXLTemporaryFolderMainThread);

	fflush(stdout);

#if 0

	FILE * foutArrayLineMarkers = fopen(
			"/home/alex/Development/lineMarkersArray.out", "w");

	for (int kk = 0; kk < ArrayOfLineMarkers.count; kk++) {

		fprintf(foutArrayLineMarkers, "%d :   %s\n", kk,
				ArrayOfLineMarkers.array[kk]);

	}
	fclose(foutArrayLineMarkers);

#endif

	generateMakeFile("main.c", InterfaceWithCoreFeatureFileName, InterfaceHeaderFileName, "Individual.x");

	//ListOfString * interfaceHeaderJustGraftFunctionDecl = readListOfStringFromFile(TemporaryInterface);

	//ListOfString * initialInterfaceForFinalInstantiation = NULL;
	//ListOfString * initialInterfaceHeaderForFinalInstantiation = NULL;

#if 0
	CallDependency *aux = ListOfNeededFunctionsForAllLOCs;

	FILE * testCallDepended = fopen("TESTCALLDEPEND.out", "w");

	while (aux) {

		fprintf(testCallDepended, "FUNCTION: %s\n", aux->LOCMarker);

		ListOfFunctionSourceFile * auxFct = aux->calledFunctions;

		while (auxFct) {
			fprintf(testCallDepended, "%s\n ", auxFct->functionName);
			auxFct = auxFct->next;
		}
		fprintf(testCallDepended, ">>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");

		aux = aux->next;
	}
	fclose(testCallDepended);
#endif

	GPAlgorithm(FinalSkeletonSourceCode, interfaceOutput,
			SourceOutputCoreFunctionalities, skeletonInterfaceSourceCodeOutput,
			ArrayOfLineMarkers, ListOfDependenciesForStatements,
			ListOfNeededFunctionsForAllLOCs, ListOfFinalGlobalDeclarations,
			GlobalDependencyList, SkeletonDependenciesOnGlobals,
			tempGlobalDeclarations, definedGlobalTypesAndVariablesFile,
			GraftFunctionName, TemporaryInterface, TotalIDsInHostSymbolTable,
			MappingCandidatesForGP, FinalDeclSymbolTable,
			ListOfMappingsFromCurrentDonor, dependencyListForAbstract,
			tempDefinesFile, tempIncludeLibFile,
			skeletonInterfaceHeaderFileOutput, HostSymbolTable,
			GlobalFctDependOutput);

	/*       instantiateAnIndividual(FinalSkeletonSourceCode, interfaceOutput, SourceOutputCoreFunctionalities,
	 skeletonInterfaceSourceCodeOutput, ArrayOfLineMarkers, ListOfDependenciesForStatements,
	 ListOfNeededFunctionsForAllLOCs,  ListOfFinalGlobalDeclarations, GlobalDependencyList,
	 SkeletonDependenciesOnGlobals, tempGlobalDeclarations, definedGlobalTypesAndVariablesFile,
	 GraftFunctionName, skeletonInterfaceHeaderFileOutput, TotalIDsInHostSymbolTable);
	 */

	free(skeletonInterfaceHeaderFileOutput);
	free(skeletonInterfaceSourceCodeOutput);
	free(hostWithInterfaceOutputFile);
	//freeListOfSymbolTable(&HostSymbolTable);
	//free(finalHostInterfaceOutput);

	printf("Hello, World!\n");
	return EXIT_SUCCESS;
}
