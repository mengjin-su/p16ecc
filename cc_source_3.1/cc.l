/********************************************************************
********************************************************************/
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int fatalError=0;
char *currentFile;

%}

letter                  [A-Za-z]
digit                   [0-9]
nonzero_digit           [1-9]

underscore              "_"
following_character     ({letter}|{digit}|{underscore})
identifier              ({letter}|{underscore}){following_character}*

dec_constant        	({nonzero_digit}{digit}*)

SP                      [ \t\v\f\015]
newline                 [\n]

%%

^"#line"{SP}+{dec_constant}.*{newline} {
						  int i = 0;
						  while ( !isdigit(yytext[i]) ) i++;
                          yylineno = atoi(yytext+i);
                        }

{dec_constant}		    { printf("VALUE: %d\n", atoi(yytext)); }
"void" 					{ printf("KEY_WORD: %s\n", yytext); }
"int"					{ printf("KEY_WORD: %s\n", yytext); }
{identifier}            { printf("ID: %s\n", yytext); }
[;{}=()]				{ printf("%c\n", *yytext); }

{SP}+                   ;

{newline}               { yylineno++; }
                        
.                       { printf("illegal character! '%c'(0x%02X) in line #%d\n", 
								 *yytext, *yytext, yylineno); 
						}

%%

/************************************************************/
int _main(char *filename)
{
	currentFile = filename;
    yyin = fopen(filename, "r");

    if ( yyin == NULL )
    {
        printf("can't open file - %s!", filename);
        exit(-1);
    }
   
    yylineno = 1;
	yylex();
    return fatalError; 
}

/************************************************************/
int yywrap(void) 
{ 
    fclose(yyin);
    return -1;
}

/************************************************************/
int yyerror(char *s)
{
    fflush(stdout);
    fprintf(stdout, "\n%s:%d: %s: token -> '%s'\n", currentFile, yylineno, s, yytext);
    fatalError++;
    return 0;
}
