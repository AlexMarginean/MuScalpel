/* 
 * File:   GenTrans.h
 * Author: alex
 *
 * Created on 25 June 2014, 19:44
 */

#ifndef GENTRANS_H
#define	GENTRANS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "SymbolTable.h"

#include <stdio.h>
#include "ListOfString.h"

#define RETURN_MARKER "/* RETURN_MARKER */"


#define LINE_END "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n" 
#define FUNCTION_CALL_MARKER "//call_the_function:"
#ifndef DEBUG
#define DEBUG
#endif

    extern char * ConsoleNullOutput;
    
    extern char * hostWithInterfaceOutputFile;

    SymbolTable * readSymbolTable(FILE *fin);

    extern char * donorSourceFileFolder;
    extern char transplantWorkFolderMainThread [250];
    extern char * TXLToolsPath;
    extern char * demoPath;
    extern char transplantHostInputFile [255];
    extern char txlFileHost [255];
    extern char inputFileGraft [255];
    extern char txlFileGraft [255];
    extern char txlFileTransplant [255];
    extern char transplantOutputFile [255];
    extern char compileCommand [255];
    extern char outputFileName [255];
    extern char transplantCFile [255];
    extern char * HostProject;

    extern char * TXLTemporaryFolderMainThread;
    extern char * SkeletonFile;
    extern char * TransplantResultLocation;
    extern char * AcceptedFunctionsForGP;

    extern char * SourceOutputCoreFunctionalities;

    extern char * SkeletonCoreFeatureHeader;

    extern char * JustCoreFeatureHeaderName;

    extern char * InterfaceHeaderFileName;

    extern char * GraftFunctionName;

    extern char * InterfaceWithCoreFeatureFileName;

    extern char * TemporaryInterface;

    extern char * IfDefProgramPath;

    char *replace_str(char *str, char *orig, char *rep);

    extern char * TransplantLogFolder;

    extern char * StatisticsFile;

    extern char * RandomNumberSeedsFile;

    extern char * CFLAGS_FILE;

    extern char * functionsFile;
    extern ListOfString * ListOfCoreFunctions;


    char * removeArraySizeInString(char * originalString);


    extern int exclude_functions;
    extern int transplant_log;
    extern int transplant_stats;
    extern int seeds_type;
    extern int host_compilation;
    extern int has_cflags;
    extern int is_demo;

#define AbstractMappingsMarker "Abstract_Mappings:\n"
#define DeclarationMappingsMarker "Declaration_Mappings:\n"
#define IndividualLinesMarker "Statement_Line_Number:\n"
#define CURRENT_DECLARATION_DONOR_MAPPING_SIGN "MAPPED_TO_ORIGINAL_VALUE_IN_DONOR"

//#define DETAILED_DEMO


#ifdef	__cplusplus
}
#endif

#endif	/* GENTRANS_H */

