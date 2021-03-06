include "CNoComments.grm"
tokens
	commentToken	"[ \t]*//[(\\\n)(\"#[\"\n]*\")(/\*#[(\*/)]*\*/)#n]*"
	inLineComment  	"/\*#(\*/)*\*/" 
	end_marker		"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
end tokens

%define inLineComment
%	[inLineCommentMarker] [NL]
%end define

define commentLine
	[commentToken] [NL]
end define



redefine declaration_or_statement
    [declaration] 
    |	[statement] 
#ifdef PREPROCESSOR
    |	[preprocessor]
#endif
#ifdef GNU
    |	[function_definition]  
#endif
#ifdef MACROS
    |	[macro_declaration_or_statement]
#endif
	|	[commentLine]
    |	[not inLineComment]  [unknown_declaration_or_statement]
end redefine



redefine declaration
	[opt inLineComment] [decl_specifiers] [list init_declarator] [semi]
    | [opt inLineComment] 	[decl_specifiers] [declarator] '= [compound_initializer] [NL]	% no semi
    | 	[opt inLineComment] [extern_langauge_declaration] 
    | 	[opt inLineComment] [null_declaration]
#ifdef GNU
    | [opt inLineComment]	[local_label] [list declarator+] 
    | [opt inLineComment]	[asm_spec] 
#endif
#ifdef MACROS
    | 	[opt inLineComment] [machinestart_declaration] 
#endif
#ifdef PREPROCESSOR
    | 	[opt inLineComment][preprocessor]
#endif
end redefine



redefine compound_statement
    { [IN] [NL]
	[compound_statement_body] [EX]
    [opt inLineComment] } [opt ';] [NL]
#ifdef PREPROCESSOR
    [repeat preprocessor]
#endif
end redefine

redefine statement
#ifdef PREPROCESSOR
	 [repeat preprocessor]
#endif
	 [opt inLineComment] [repeat label] [unlabeled_statement]
    |  [opt inLineComment] [label]	% e.g. at end of switch block
#ifdef PREPROCESSOR
    |  [opt inLineComment]  [preprocessor]
#endif
	| [inLineComment]
end redefine


redefine do_statement
    'do [sub_statement] [opt inLineComment] 'while ( [expression_list] ) [semi] 
#ifdef MACROS
|   'do [sub_statement] [opt inLineComment] 'while [macro_call] [semi] 
|   'do [sub_statement] [whileeachid] ( [expression_list] ) [semi] 
#endif
end redefine









define dependency_output
	'FUNCTION_NAME: [NL] [inLineComment] [NL] 'DECLARATION_LIST: [NL] [repeat inline_comment_with_line_end]  [end_marker] [NL]
end define


define inline_comment_with_line_end
	[inLineComment] [NL]
end define





define struct_declarations_output
	[repeat struct_needed_declarations] [end_marker] [NL]
end define


define struct_needed_declarations
	[repeat declared_types] [repeat declared_globals]
end define




define declared_types
	DECLARED_TYPES [NL] [inLineComment] [NL] [repeat id_or_struct_decl_with_line_break]
end define

define declared_globals
	DECLARED_GLOBALS [NL] [inLineComment] [NL] [repeat id_with_line_break]
end define


  
define dependency_list 
	FUNCTION_NAME: [NL] [inLineComment] [NL] DECLARATION_LIST: [NL] [repeat inLineComment] [NL] [end_marker] [NL]
end define


define declared_globals_marker
	DECLARED_GLOBALS
end define

define id_or_struct_decl_with_line_break
	[struct_decl_line_break]
	|	[id_with_line_break]
end define

define id_with_line_break
	[not declared_globals_marker] [id] [NL]
end define

define struct_decl_line_break
	'struct [id] [NL]
	|	'union [id] [NL]
	|	'enum [id] [NL]
end define





function main
    match [program]
        P [program]
    import TXLargs [repeat stringlit]
    deconstruct  TXLargs
    	globalDeclarationFile [stringlit] 
    	globalDefinedTypesVarStructsTypedef [stringlit] outputFile [stringlit]
	deconstruct P 
		interfaceCompUnit [compilation_unit]
   	%construct the list of global declarations, as a program
   	construct globalDeclP [program]
   		_ [read globalDeclarationFile]
   	construct globalVariableDeclarationList [repeat declaration]	
   		_ [^ globalDeclP]
   	deconstruct interfaceCompUnit
   		interfaceFunctionsDefs [repeat function_definition_or_declaration]		
   	deconstruct * [function_definition] interfaceCompUnit
   		TheInterfaceFunction [function_definition]
   	%first construct the list of global declarations, for types and variables
   	%and parse the list of declared globals to the function for generating the list
   	%of global dependencies	
   	construct dependencyAndDeclarationsList [repeat struct_declarations_output]
   		_ [read globalDefinedTypesVarStructsTypedef]
   	construct listOfDeclaredTypes [repeat declared_types]
   		_ [^ dependencyAndDeclarationsList]
   	construct listOfDeclaredGlobals [repeat declared_globals]
   		_ [^ dependencyAndDeclarationsList]	
   		
   	construct mmisamidsai [repeat declared_globals]
   		listOfDeclaredGlobals
   		
   		
   	construct ListOfAllDependencies [repeat dependency_output] 
   		_ [extractAllGlobalTypeAndVariableDependenciesForAFunction TheInterfaceFunction globalVariableDeclarationList 
   			listOfDeclaredGlobals listOfDeclaredTypes]

	construct msfdfdss [repeat dependency_output]
   		ListOfAllDependencies  [write outputFile]

end function 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Main function for dependency extraction

function extractAllGlobalTypeAndVariableDependenciesForAFunction	CurrentFunction [function_definition]
																	GlobalVariableDeclarations [repeat declaration]
																	GlobalStructVariableDeclarations [repeat declared_globals]
																	GlobalStructTypedefDeclaredTypes [repeat declared_types]
	replace [repeat dependency_output]
		_ [repeat dependency_output]
		
	%construct the list of global dependencies (variable declarations + struct var declarations)
   	export ListOfAllDependencies [repeat dependency_output]
   		_
    construct createTheListOfStatements [function_definition_or_declaration]
   		CurrentFunction [fixEveryBlock  GlobalVariableDeclarations GlobalStructVariableDeclarations]		
   	import ListOfAllDependencies [repeat dependency_output]
   	
   	%construct the list of global type definition dependencies
   	
   	construct justDeclarationFromCurrentFunction [repeat declaration]
   		_ [^ CurrentFunction]

   	construct theListOfGlobalTypeDependency [repeat dependency_output]
   		_ [constructListOfNeededDeclForTypeDefs justDeclarationFromCurrentFunction GlobalStructTypedefDeclaredTypes]
   	
   	construct ListOfGlobalAndTypeDefsDependencies [repeat dependency_output]
   		ListOfAllDependencies [. theListOfGlobalTypeDependency]


	by
		ListOfGlobalAndTypeDefsDependencies
end function




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%GLOBAL TYPEDEF EXTRACTOR
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%the list of dependency list for a repeat typedef
function constructListOfNeededDeclForTypeDefs	TypeDefs [repeat declaration]
												DeclaredTypes [repeat declared_types]
	replace [repeat dependency_output]
		current [repeat dependency_output]
		
		
		

		
	deconstruct TypeDefs 
		first [declaration] rest [repeat declaration]
	deconstruct * [inLineComment] first
		commentMarker [inLineComment]
	%all the needed structs / typedefs for structs
	construct neededDeclForCurrentTypeDefs [repeat inline_comment_with_line_end]
		_ [extractNeededDeclForCurrentTypeDef first DeclaredTypes]

	construct newNeededDependenciesForTypeDefs [repeat dependency_output]
		_ [constructGlobalDependencyListWhenNonEmpty commentMarker neededDeclForCurrentTypeDefs]
	
	by
		current [. newNeededDependenciesForTypeDefs]
		[constructListOfNeededDeclForTypeDefs rest DeclaredTypes]
end function


function constructGlobalDependencyListWhenNonEmpty	CommentMarker [inLineComment]
													IdentifiedDecl [repeat inline_comment_with_line_end]
	replace [repeat dependency_output]
		current [repeat dependency_output]
	construct endMarker [end_marker]
		 '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
	construct newNeededDependenciesForTypeDefs [dependency_output]
		'FUNCTION_NAME: CommentMarker 'DECLARATION_LIST: IdentifiedDecl endMarker
	construct lengthIdentified [number]	
		_ [length IdentifiedDecl]
	where
		lengthIdentified [> 0]
	by
		current [. newNeededDependenciesForTypeDefs]
end function



function extractNeededDeclForCurrentTypeDef	CurrentDeclaration [declaration]
											DeclaredTypes [repeat declared_types]
	replace [repeat inline_comment_with_line_end]
		current [repeat inline_comment_with_line_end]
		
		

		
	export CurrentMarkers [repeat inline_comment_with_line_end]
		_ 
	construct exportTheMarkers [repeat declared_types]
		DeclaredTypes [exportNeededMarkersTypeDef CurrentDeclaration]
	import CurrentMarkers
	
	
	by
		CurrentMarkers
end function

rule exportNeededMarkersTypeDef	CurrentDeclaration [declaration]
	replace $ [declared_types]
		current [declared_types]
	deconstruct * [inLineComment] CurrentDeclaration
		justTheMarker [inLineComment]
	deconstruct * [repeat id_or_struct_decl_with_line_break] current
		justTheTypeNames [repeat id_or_struct_decl_with_line_break]
	deconstruct * [inLineComment] current
		currentDependencyMarker [inLineComment]
	where
		justTheTypeNames [typeIdNameMatchedByDeclarationTypeDef CurrentDeclaration justTheMarker currentDependencyMarker]
		[typeIdNameMatchedByDeclarationStruct CurrentDeclaration justTheMarker currentDependencyMarker]
		
		
		import CurrentMarkers [repeat inline_comment_with_line_end]
		
		construct currentMarkerNewLine [inline_comment_with_line_end]
			currentDependencyMarker
		
		export CurrentMarkers 
			CurrentMarkers [. currentMarkerNewLine]
	by
		current
end rule


rule typeIdNameMatchedByDeclarationTypeDef	CurrentDeclaration [declaration]
											CurrentDeclarationMarker [inLineComment]
											CurrentDependencyMarker [inLineComment]
	match [id_with_line_break]
		current [id_with_line_break]
	where 
		CurrentDeclaration [aDeclarationMatchID current]
		[aMemberDeclarationMatchUpperMacroName current]
end rule


rule aMemberDeclarationMatchUpperMacroName	CurrentID [id_with_line_break]
	match [upper_macro_name]
		current [upper_macro_name]
		
	construct currentStringlit [stringlit]
		_ [quote current]
		
	deconstruct * [id] CurrentID
		justID [id]
		
	construct justIDStringlit [stringlit]
		_ [quote justID]
		
	where
		currentStringlit [= justIDStringlit]
	
end rule



rule typeIdNameMatchedByDeclarationStruct	CurrentDeclaration [declaration]
											CurrentDeclarationMarker [inLineComment]
											CurrentDependencyMarker [inLineComment]
	match [struct_decl_line_break]
		current [struct_decl_line_break]
	
	construct structTogheterString [stringlit]
		_ [quote current]
	
	construct structTogheterID [id]
		_ [unquote structTogheterString]
		
	construct structIdWithLineBreak [id_with_line_break]
		structTogheterID
	
	where 
		CurrentDeclaration [aDeclarationMatchID structIdWithLineBreak]
		[aMemberDeclarationMatchUpperMacroName structIdWithLineBreak]
end rule


function aDeclarationMatchID	CurrentID [id_with_line_break]
	match [declaration]
		CurrentDecl [declaration]
	
	deconstruct * [type_specifier] CurrentDecl
		TypeSpec [type_specifier]
	construct TypeSpecString [stringlit]
		_ [quote TypeSpec]
	construct currentIDString [stringlit]
		_ [quote CurrentID]
	where
		TypeSpecString [= currentIDString]
end function

%end global typedef extractor
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



















rule fixEveryBlock	GlobalDeclarations [repeat declaration]
					GlobalStructsDeclarations [repeat declared_globals]
	replace $ [compound_statement]
		currentBlock [compound_statement]
	deconstruct * [compound_statement_body] currentBlock
		currentBlockBody [compound_statement_body]
	construct addStatements [compound_statement_body]
		currentBlockBody [matchAllStatements currentBlock  GlobalDeclarations GlobalStructsDeclarations]
	construct addStatementsFromDecl [compound_statement_body]
		currentBlockBody [matchStatementsFromDeclaration currentBlock GlobalDeclarations GlobalStructsDeclarations]
	by
		currentBlock
end rule


rule matchStatementsFromDeclaration	CurrentBlock [compound_statement]
									GlobalDeclarations [repeat declaration]
									GlobalStructsDeclarations [repeat declared_globals]
	skipping [compound_statement] 
	replace $ [declaration]
		current [declaration]
	%this will match all the declarations, and statements that are identified as declarations
	%by the current grammar	
	construct currentDeclOrStm [declaration_or_statement]
		current
	construct allIdsFromStatementNoUpperMacro [repeat id]
		_ [^ currentDeclOrStm] 
		
	construct allUpperMacroNameFromStatement [repeat upper_macro_name]
		_ [^ currentDeclOrStm]	
		
	construct allUpperMacroAsID [repeat id]
		_ [upperMacroNameListToIdList allUpperMacroNameFromStatement]
		
	construct allIdsFromStatement [repeat id]
		allIdsFromStatementNoUpperMacro [. allUpperMacroAsID]	
		
		
	%the statement line marker
	deconstruct * [inLineComment] currentDeclOrStm
		StatementNumberMarker [inLineComment]		
		
	export repeatOfNeededDecl [repeat inline_comment_with_line_end]
		_
	%construct the needed decl list	
	construct addTheNeededDeclarations [repeat declaration]
		GlobalDeclarations [matchAllDeclarationOfNeededIDs allIdsFromStatement]
	construct addTheNeededStructDeclarations [repeat declared_globals]
		GlobalStructsDeclarations [matchAllStructDeclarationsOfNeededIDs allIdsFromStatement]
	
	import repeatOfNeededDecl [repeat inline_comment_with_line_end]	
	construct dependOutputEl [repeat dependency_output]
		_ [constructGlobalDependencyListWhenNonEmpty StatementNumberMarker repeatOfNeededDecl]

	import ListOfAllDependencies [repeat dependency_output]
	export ListOfAllDependencies
		ListOfAllDependencies [. dependOutputEl]
	by
		current
end rule

function upperMacroNameListToIdList	UpperMacroList [repeat upper_macro_name]
	replace [repeat id]
		current [repeat id]
		
	deconstruct UpperMacroList 
		first [upper_macro_name] rest [repeat upper_macro_name]
		
	construct firstString [stringlit]
		_ [quote first]
	construct firstID [id]
		_ [unquote firstString]

	by
		current [. firstID]
		[upperMacroNameListToIdList rest] 
end function






rule matchAllStatements	CurrentBlock [compound_statement]
						GlobalDeclarations [repeat declaration]
						GlobalStructsDeclarations [repeat declared_globals]
	skipping [compound_statement] 
	replace $ [statement]
		current [statement]
	construct currentNoBody [statement]
		current [removeCompoundBody]
	construct currentDeclOrStm [declaration_or_statement]
		currentNoBody

	construct allIdsFromStatementNoUpperMacro [repeat id]
		_ [^ currentNoBody] 
		
		
	construct allUpperMacroNameFromStatement [repeat upper_macro_name]
		_ [^ currentNoBody]
		
	construct allUpperMacroAsID [repeat id]
		_ [upperMacroNameListToIdList allUpperMacroNameFromStatement]
		
	construct allIdsFromStatement [repeat id]
		allIdsFromStatementNoUpperMacro [. allUpperMacroAsID]
		
		
		
		
		
	%the statement line marker
	deconstruct * [inLineComment] currentNoBody
		StatementNumberMarker [inLineComment]	
	export repeatOfNeededDecl [repeat inline_comment_with_line_end]
		_
	%construct the needed decl list	
	construct addTheNeededDeclarations [repeat declaration]
		GlobalDeclarations [matchAllDeclarationOfNeededIDs allIdsFromStatement]
	construct addTheNeededStructDeclarations [repeat declared_globals]
		GlobalStructsDeclarations [matchAllStructDeclarationsOfNeededIDs allIdsFromStatement]
	
	import repeatOfNeededDecl [repeat inline_comment_with_line_end]
	
	construct dependOutputEl [repeat dependency_output]
		_ [constructGlobalDependencyListWhenNonEmpty StatementNumberMarker repeatOfNeededDecl]
	import ListOfAllDependencies [repeat dependency_output]
	
	export ListOfAllDependencies 
		ListOfAllDependencies [. dependOutputEl]
	by
		current
		
end rule




rule matchAllStructDeclarationsOfNeededIDs	ListOfIDs [repeat id]
	replace $ [declared_globals]
		currentDecl [declared_globals]
	construct addedStructDefs [repeat inline_comment_with_line_end]
		_ [addStructDefinitionIfCase currentDecl ListOfIDs]
	import repeatOfNeededDecl [repeat inline_comment_with_line_end]
	export repeatOfNeededDecl 
		repeatOfNeededDecl [. addedStructDefs]
	by
		currentDecl
end rule


function addStructDefinitionIfCase	CurrentStructDef [declared_globals]
									ListOfNeededIDs [repeat id]
	replace [repeat inline_comment_with_line_end]
		_ [repeat inline_comment_with_line_end]
	
	deconstruct CurrentStructDef
		DECLARED_GLOBALS currentMarker [inLineComment] declarationList [repeat id_with_line_break]
	
	construct listOfIDs [repeat id]
		_ [^ declarationList]

	construct currentDeclMarkerNL [inline_comment_with_line_end]
		currentMarker
	where
		listOfIDs [matchAnIdFromTheList ListOfNeededIDs]	
	by
		_ [. currentDeclMarkerNL]
end function















rule matchAllDeclarationOfNeededIDs	ListOfIDs [repeat id]
	replace $ [declaration]
		currentDecl [declaration]
	construct addedDeclFromBaseDecl [repeat inline_comment_with_line_end]
		_ [addDeclIfCaseFromBaseDecl currentDecl ListOfIDs]
	construct addedDeclFromTypeSpec [repeat inline_comment_with_line_end]
		addedDeclFromBaseDecl [addDeclIfCaseFromTypeSpecifier currentDecl ListOfIDs]
	import repeatOfNeededDecl [repeat inline_comment_with_line_end]
	export repeatOfNeededDecl 
		repeatOfNeededDecl [. addedDeclFromTypeSpec]
	by
		currentDecl
end rule

function addDeclIfCaseFromBaseDecl	CurrentDeclaration [declaration]
									ListOfNeededIDs [repeat id]
	replace [repeat inline_comment_with_line_end]
		_ [repeat inline_comment_with_line_end]
	construct listOfBaseDecl [repeat base_declarator]
		_ [^ CurrentDeclaration]
	construct listOfIDs [repeat id]
		_ [^ listOfBaseDecl]
	construct listOfUpperMacroName [repeat upper_macro_name]
		_ [^ listOfBaseDecl]
		
		
		
	deconstruct * [inLineComment] CurrentDeclaration
		currentDeclMarker [inLineComment]
	construct currentDeclMarkerNL [inline_comment_with_line_end]
		currentDeclMarker

	where
		ListOfNeededIDs [matchAnIdFromTheList listOfIDs]
		[idMatchAnUpperMacroFromList listOfUpperMacroName]
	by
		_ [. currentDeclMarkerNL]
end function


function idMatchAnUpperMacroFromList	ListOfUpperMacroNames [repeat upper_macro_name]
	match * [id]
		current [id]
	where
		ListOfUpperMacroNames [matchUppperMacroInID current]	
end function

function matchUppperMacroInID	CurrentID[id]
	match * [upper_macro_name]
		current [upper_macro_name]
		
	construct currentStringlit [stringlit]
		_ [quote current]
		
	construct idStringlit [stringlit]
		_ [quote CurrentID]
	
	where
		idStringlit [= currentStringlit]
end function














%when just one variable is declared, we must do different since it will not be matched as base declarator
%because of a bug in grammar
function addDeclIfCaseFromTypeSpecifier	CurrentDeclaration [declaration]
										ListOfNeededIDs [repeat id]
	replace [repeat inline_comment_with_line_end]
		_ [repeat inline_comment_with_line_end]	
	deconstruct CurrentDeclaration
		inlineComm [opt inLineComment] declSpec[decl_specifiers] semiMark [semi]
	construct idsFromDeclSpec [repeat id]
		_ [^ declSpec]		
	construct lastID [repeat id]
		_ [returnLastID idsFromDeclSpec]
	construct listOfUpperMacroName [repeat upper_macro_name]
		_ [^ declSpec]
		
		
	deconstruct * [inLineComment] CurrentDeclaration
		currentDeclMarker [inLineComment]
	construct currentDeclMarkerNL [inline_comment_with_line_end]
		currentDeclMarker
	where
		ListOfNeededIDs [matchAnIdFromTheList lastID]
		[idMatchAnUpperMacroFromList listOfUpperMacroName]
	by
		_ [.currentDeclMarkerNL]
end function


function returnLastID	IDList [repeat id]
	replace [repeat id]
		_ [repeat id]
	deconstruct IDList 
		first [id] rest [repeat id]
	by
		_ [. first]
		[returnLastID rest]
end function



function matchAnIdFromTheList	ListOfNeededIDs [repeat id]
	match * [id]
		current [id]
	where
		ListOfNeededIDs [matchInList current]
end function

function matchInList	CurrentID[id]
	match * [id]
		current [id]
	where
		current [= CurrentID]
end function


rule removeCompoundBody 
	replace $ [compound_statement_body]
		currentBody [compound_statement_body]
		
	by
		_	
end rule







