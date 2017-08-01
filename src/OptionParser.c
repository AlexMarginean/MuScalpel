#include "OptionParser.h"
#include "Utils.h"
#include "SkeletonMapping.h"
#include "ListOfString.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <config.h>
#if OS == Linux
#include <scsi/scsi.h>
#elif OS == MacOS
//#include "/usr/local/include/iscsi/iscsi.h"
#endif

char * SourceOutputCoreFunctionalities;
char * donorSourceFileFolder = NULL;
char transplantWorkFolderMainThread [250];
char * TXLToolsPath;
char transplantHostInputFile [255];
char txlFileHost [255];
char inputFileGraft [255];
char txlFileGraft [255];
char txlFileTransplant [255];
char transplantOutputFile [255];
char compileCommand [255];
char outputFileName [255];
char transplantCFile [255];
char * TXLTemporaryFolderMainThread;
char * SkeletonFile;
char * TransplantResultLocation;
char * AcceptedFunctionsForGP;
char * SkeletonCoreFeatureHeader;
char * InterfaceHeaderFileName;
char * JustCoreFeatureHeaderName;
char * GraftFunctionName;
char * InterfaceWithCoreFeatureFileName;
char * TemporaryInterface;
char * StatisticsFile;
char * TransplantLogFolder;
char * RandomNumberSeedsFile = NULL;
char * CFLAGS_FILE = NULL;
char * demoPath;
char * HostProject = NULL;

char * IfDefProgramPath;

char * functionsFile = NULL;
ListOfString * ListOfCoreFunctions;

void fixEnvironmentAndGlobalVariables() {
    //copy txl program names
    strcpy(txlFileHost, "HostSymbolTable.x");
    strcpy(txlFileGraft, "graftSymbolTable.x");
    strcpy(txlFileTransplant, "addGraftToHost.x");
    //cleanTransplantWorkFolder(transplantWorkFolder);
    //createADirIfItNotExists(transplantWorkFolder);
    //createADirIfItNotExists(TXLTemporaryFolder);
    //createADirIfItNotExists(TransplantResultLocation);

}


int exclude_functions = 0;
int transplant_log = 0;
int transplant_stats = 0;
int seeds_type = 0;
int host_compilation = 0;
int has_cflags = 0;
int is_demo = 0;

void parse_args(int argc, char * argv[]) {

	ListOfCoreFunctions = NULL;

    fixEnvironmentAndGlobalVariables();

    int c;
    while (1) {
        static struct option long_options[] = {
            //flag options 
            {"exclude_functions", required_argument, &exclude_functions, 1},
            {"transplant_log", required_argument, &transplant_log, 1},
            {"transplant_statistics", required_argument, &transplant_stats, 1},
            {"urandom_seeds", no_argument, &seeds_type, 0},
            {"random_seeds", no_argument, &seeds_type, 1},
            {"seeds_file", required_argument, &seeds_type, 2},
            {"host_compilation", no_argument, &host_compilation, 1},
            {"compiler_options", required_argument, &has_cflags, 1},
            {"demo", required_argument, &is_demo,1},
            //non flag options
            {"host_target", required_argument, 0, 'h'},
            {"donor_target", required_argument, 0, 'd'},
            {"txl_tools_path", required_argument, 0, 'T'},
            {"donor_folder", required_argument, 0, 'D'},
            {"workspace", required_argument, 0, 'w'},
            {"core_function", required_argument, 0, 'f'},
			{"host_project", required_argument, 0, 'B'},
			{"functions", required_argument, 0, 'F'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        c = getopt_long(argc, argv, "h:d:T:D:w:f:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
            {
                //if (long_options[option_index].flag != 0){
                //} 0,1,2,5,7
                switch (option_index) {
                    case 0:
                    {
                        AcceptedFunctionsForGP = (char *) malloc((strlen(optarg) + 1) * sizeof (char));
                        strcpy(AcceptedFunctionsForGP, optarg);
                        break;
                    }
                    case 1:
                    {
                        TransplantLogFolder = (char *) malloc((strlen(optarg) + 1) * sizeof (char));
                        strcpy(TransplantLogFolder, optarg);
                        break;
                    }
                    case 2:
                    {
                        StatisticsFile = (char *) malloc((strlen(optarg) + 1) * sizeof (char));
                        strcpy(StatisticsFile, optarg);
                        break;
                    }
                    case 5:
                    {
                        RandomNumberSeedsFile = (char *) malloc((strlen(optarg) + 1) * sizeof (char));
                        strcpy(RandomNumberSeedsFile, optarg);
                        break;
                    }
                    case 7:
                    {
                        CFLAGS_FILE = (char *) malloc((strlen(optarg) + 1) * sizeof (char));
                        strcpy(CFLAGS_FILE, optarg);
                        break;
                    }
                    case 8:{
                        demoPath = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
                        strcpy(demoPath, optarg);
                        char * cmdCleanupDemoFolder = (char*) malloc(1000 * sizeof(char));
                        sprintf(cmdCleanupDemoFolder,"rm -rf %s && mkdir %s", demoPath, demoPath);
                        system(cmdCleanupDemoFolder);
                        break;
                    }
                    case 9:{
                    	strcpy(HostProject, optarg);
                    	break;
                    }
                    case 10:{
                        functionsFile = (char *) malloc (1000 * sizeof(char));
                    	strcpy(functionsFile,optarg);
                    	ListOfCoreFunctions = readListOfStringFromFileAsWords(functionsFile);
                    	break;
                    }
                    default:
                    {
                        //do nothing, just set the flag
                        break;
                    }
                }
                break;
            }
            case 'h':
            {
                strcat(transplantHostInputFile, optarg);
                break;
            }
            case 'd':
            {
                strcat(inputFileGraft, optarg);
                break;
            }
            case 'T':
            {
                TXLToolsPath = (char *) malloc((strlen(optarg) + 1) * sizeof (char));
                strcpy(TXLToolsPath, optarg);
                break;
            }
            case 'D':
            {
                donorSourceFileFolder = (char *) malloc (1000 * sizeof(char));
                strcpy(donorSourceFileFolder, optarg);
                break;
            }
            case 'w':
            {
                strcpy(transplantWorkFolderMainThread, optarg);
                break;
            }
            case 'f':
            {
                GraftFunctionName = (char *) malloc((strlen(optarg) + 1) * sizeof (char));
                strcpy(GraftFunctionName, optarg);
                break;
            }
            case 'B':
            {
                HostProject = (char *) malloc (1000 * sizeof(char));
            	strcpy(HostProject, optarg);
            	break;
            }
            case 'F':
            {
                functionsFile = (char *) malloc(1000 * sizeof(char));
            	strcpy(functionsFile,optarg);
            	ListOfCoreFunctions = readListOfStringFromFileAsWords(functionsFile);
            	break;
            }
        }
    }

    if(!RandomNumberSeedsFile){
        RandomNumberSeedsFile = (char *) malloc (1000 * sizeof(char));
        sprintf(RandomNumberSeedsFile,"%sseed-1.in",transplantWorkFolderMainThread);
    }

    if(!CFLAGS_FILE){
        CFLAGS_FILE = (char *) malloc (1000 * sizeof(char));
        sprintf(CFLAGS_FILE, "%sCFLAGS", transplantWorkFolderMainThread);
    }

    if(!donorSourceFileFolder){
        donorSourceFileFolder = (char *) malloc (1000 * sizeof(char));
        sprintf(donorSourceFileFolder, "%sDonor/", transplantWorkFolderMainThread);
    }

    if (!TXLToolsPath) {
        TXLToolsPath = (char *) malloc(5 * sizeof (char));
        strcpy(TXLToolsPath, "");
    }

    if(!HostProject){
        HostProject = (char *) malloc(1000 * sizeof(char));
        sprintf(HostProject, "%sHost", transplantWorkFolderMainThread);
    }

    if(!functionsFile){
        functionsFile = (char *) malloc (1000 * sizeof(char));
        sprintf(functionsFile,"%score_functions.in", transplantWorkFolderMainThread);
        ListOfCoreFunctions = readListOfStringFromFileAsWords(functionsFile);
    }

    TXLTemporaryFolderMainThread = (char *) malloc((strlen(transplantWorkFolderMainThread) + 8) * sizeof (char));
    sprintf(TXLTemporaryFolderMainThread, "%sTemp/", transplantWorkFolderMainThread);

    char * command = (char *) malloc(2000 * sizeof(char));
    sprintf(command, "rm -rf %s && mkdir %s", TXLTemporaryFolderMainThread, TXLTemporaryFolderMainThread);
    int status = system(command);
    if (status != 0) {
        printf("Error when creating the temporary working folder.");
        fflush(stdout);
    }


    TransplantResultLocation = (char *) malloc((strlen(transplantWorkFolderMainThread) + 20) * sizeof (char));
    
    sprintf(TransplantResultLocation, "%sTransplantCode/", transplantWorkFolderMainThread);

    SourceOutputCoreFunctionalities = (char *) malloc((strlen(TXLTemporaryFolderMainThread) + 20) * sizeof (char));
    sprintf(SourceOutputCoreFunctionalities, "%s%s", TXLTemporaryFolderMainThread, "NeededSourceCode.c");

    SkeletonFile = (char *) malloc((strlen(TXLTemporaryFolderMainThread) + 30) * sizeof (char));
    sprintf(SkeletonFile, "%sInstantiatedSkeleton.out", TXLTemporaryFolderMainThread);


    JustCoreFeatureHeaderName = (char *) malloc(500 * sizeof (char));
    sprintf(JustCoreFeatureHeaderName, "NeededFunctionsHeader.h");

    SkeletonCoreFeatureHeader = (char *) malloc((strlen(TXLTemporaryFolderMainThread)
            + strlen(JustCoreFeatureHeaderName) + 1) * sizeof (char));
    sprintf(SkeletonCoreFeatureHeader, "%s%s", TXLTemporaryFolderMainThread, JustCoreFeatureHeaderName);

    if (!exclude_functions) {

    	system("pwd");

        //write an empty file, needed by a TXL program
        AcceptedFunctionsForGP = (char *) malloc((strlen(TXLTemporaryFolderMainThread) + 25) * sizeof (char));
        sprintf(AcceptedFunctionsForGP, "%sfakeExcludeFile.in", TXLTemporaryFolderMainThread);
        FILE * fakeFile = fopen(AcceptedFunctionsForGP, "w");
        fclose(fakeFile);
    }

    if (strcmp(TXLToolsPath, "./")) {
        IfDefProgramPath = (char *) malloc((strlen(TXLToolsPath) + 10) * sizeof (char));
        sprintf(IfDefProgramPath, "%sifdef.x", TXLToolsPath);
    } else {
        IfDefProgramPath = (char *) malloc(10 * sizeof (char));
        sprintf(IfDefProgramPath, "./ifdef.x");
    }

}





