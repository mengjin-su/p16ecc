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

#define YYDEBUG 1

%}

%union {
    char   *s;  // string value
    int     i;  // integer value
}

%token CHAR INT VOID 
%token <i> CONSTANT 
%token <s> IDENTIFIER

%start program

%%

program
    : external_definitions
    ;
external_definitions
    : function_definition
    | external_definitions function_definition
    ;
function_definition
    : type_specifier
	  function_declarator
      compound_statement        { printf ("#%d: function()\n", yylineno);   }
	;
identifier
	: IDENTIFIER				{ printf ("#%d: id = %s\n", yylineno, $1); }
	;
primary_expr
    : identifier				
    | CONSTANT 					{ printf ("#%d: value = %d\n", yylineno, $1); }
    | '(' primary_expr ')'
    ;
assignment_statement
    : identifier
      '=' primary_expr ';'		{ printf ("#%d: assignment statement\n", yylineno);  }
    ;
func_data_declaration
    : type_specifier declarator_list ';'
    ;
declarator_list
    : identifier
    | declarator_list ',' identifier
    ;
type_specifier
    : CHAR
    | INT
	| VOID
    ;
function_declarator
    : identifier '(' ')'
	;
statement
    : compound_statement
    | assignment_statement
    ;
compound_statement
    : '{' '}'
    | '{' statement_list '}'
    ;
general_statement
	: statement
	| func_data_declaration
	;
statement_list
    : general_statement
	| statement_list general_statement
    ;
%%

