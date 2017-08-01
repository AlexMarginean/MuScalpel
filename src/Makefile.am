# Compiler and linker flags
AM_CFLAGS = -Wall -std=c99 -O0 -g
muScalpel_CFLAGS = -Wall -std=c99 -O0 -g


if Linux
AM_LDFLAGS = -pthread
else !Linux
AM_LDFLAGS = 
endif !Linux


# List of binaries to produce
bin_PROGRAMS = muScalpel
muScalpel_SOURCES =  \
	coreFunctionExtractor.c \
        coreFunctionExtractor.h \
        DeclarationSymbolTable.c \
        DeclarationSymbolTable.h \
        DeclarationWithHeaderDefinition.c \
        DeclarationWthHeaderDefinition.h \
        DependencyList.c \
        DependencyList.h \
        DependencyListWithID.c \
        DependencyListWithID.h \
        FeatureInterface.c \
        FeatureInterface.h \
        folderContent.c \
        folderContent.h \
        FunctionCallDependency.c \
        FunctionCallDependency.h \
        FunctionElement.c \
        FunctionElement.h \
        GenTrans.h \
        GIReduction.c \
        GIReduction.h \
        GlobalDeclarations.c \
        GlobalDeclarations.h \
        GP.c \
        GP.h \
        GPIndividual.c \
        GPIndividual.h \
        IndividualInstantiator.c \
        IndividualInstantiator.h \
        ListOfFunctionSourceFile.c \
        ListOfFunctionSourceFile.h \
        ListOfInt.c \
        ListOfInt.h \
        ListOfString.c \
        ListOfString.h \
        main.c \
        Mapping.h \
        MappingCandidate.c \
        MappingCandidate.h \
        Mappping.c \
        MarkFunctionCall.c \
        MarkFunctionCall.h \
        OptionParser.c \
        OptionParser.h \
        PrepareForGP.c \
        PrepareForGP.h \
        ReductionIndividual.c \
        ReductionIndividual.h \
        reverseCallGraph.c \
        reverseCallGraph.h \
        Skeleton.c \
        Skeleton.h \
        SkeletonMapping.c \
        SkeletonMapping.h \
        SymbolTable.c \
        SymbolTable.h \
        Utils.c \
        Utils.h 
