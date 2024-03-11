/*-----------------------------------------------------------------------

   cc.y - parser definition file:

-------------------------------------------------------------------------*/
%{
#include <stdio.h>
#include <stdarg.h> 
#include "common.h"

extern int  yyerror (char *);
extern int  yylex (), yylineno;
extern FILE *yyin;

int yyparse(void);

#define ADD_SRC(n)	addSrc(n, getSrcCode())
#define CLR_SRC()	getSrcCode()

#define YYDEBUG 1

%}

%union {
	char   *s;  // string value
	int     i;  // integer value
	node   *n;
	attrib *a;
	int    *p;	
}

%token CHAR INT VOID FUNC_DEF FUNC_DEC FUNC_DATA
%token <i> CONSTANT 
%token <s> IDENTIFIER

%start program

%type <n> program external_definitions function_definition identifier
%type <a> type_specifier
%type <n> function_declarator func_data_declaration
%type <n> statement general_statement compound_statement primary_expr 
%type <n> assignment_statement statement_list declarator_list

%%

program
    : external_definitions		{ progUnit = $1; }
    ;
external_definitions
    : function_definition		{ $$ = makeList($1); CLR_SRC(); }
    | external_definitions 
	  function_definition		{ $$ = appendList($1, $2); CLR_SRC(); }
    ;
function_definition
    : type_specifier
	  function_declarator
      compound_statement        { $$ = oprNode(FUNC_DEF, 2, $2, $3); 
								  $$->opr.attr = $1;
								}
	;
identifier
	: IDENTIFIER				{ $$ = idNode($1); free($1); }
	;
primary_expr
    : identifier				{ $$ = $1; }
    | CONSTANT 					{ $$ = conNode($1); }
    | '(' primary_expr ')'		{ $$ = $2; }
    ;
assignment_statement
    : identifier
      '=' primary_expr ';'		{ $$ = oprNode('=', 2, $1, $3); ADD_SRC($$); }
    ;
func_data_declaration
    : type_specifier declarator_list ';' { $$ = oprNode(FUNC_DATA, 1, $2); 
								  $$->opr.attr = $1;
								}
    ;
declarator_list
    : identifier				{ $$ = makeList($1); }
    | declarator_list ',' identifier { $$ = appendList($1, $3); }
    ;
type_specifier
    : CHAR						{ $$ = newAttr(CHAR); }
    | INT						{ $$ = newAttr(INT);  }
	| VOID						{ $$ = newAttr(VOID); }
    ;
function_declarator
    : identifier '(' ')'		{ $$ = oprNode(FUNC_DEC, 1, $1); }
	;
statement
    : compound_statement		{ $$ = $1; }
    | assignment_statement		{ $$ = $1; }
    ;
compound_statement
    : '{' '}'					{ $$ = oprNode('{', 0); }
    | '{' statement_list '}'	{ $$ = oprNode('{', 1, $2); }
    ;
general_statement
	: statement					{ $$ = $1; CLR_SRC(); }
	| func_data_declaration		{ $$ = $1; CLR_SRC(); }
	;
statement_list
    : general_statement			{ $$ = makeList($1); }
	| statement_list general_statement { $$ = appendList($1, $2); }
    ;
%%

