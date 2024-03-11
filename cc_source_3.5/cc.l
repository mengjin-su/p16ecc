/********************************************************************
********************************************************************/
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "common.h"
#include "cc.h"

extern int yyparse ();

int fatalError=0;
char *currentFile = NULL;

#define SRC_BUF_SIZE    4096
char srcBuf[SRC_BUF_SIZE], emptySrc = 1;

void addSrcCode(void);
void updateSourceInfo(char *file);

%}

letter                  [A-Za-z]
digit                   [0-9]
underscore              "_"
following_character     ({letter}|{digit}|{underscore})
identifier              ({letter}|{underscore}){following_character}*

nonzero_digit           [1-9]
dec_constant        	({nonzero_digit}{digit}*)

character_escape_code   [ntbrfv\\'"a?]
escape_code             ({character_escape_code}|{octal_escape_code}|{hex_escape_code})
escape_character        (\\{escape_code})

c_char                  ([^'\\\n]|{escape_character})
c_char_sequence         ({c_char}+)
character_constant      (\L?\'{c_char_sequence}\')

s_char                  ([^"\\\n]|{escape_character})
s_char_sequence         ({s_char}+)
string_constant         \L?\"{s_char_sequence}?\"
lib_inc_file            <{s_char_sequence}>

SP                      [ \t\v\f\015]
newline                 [\n]

%%

^"#line"{SP}+{dec_constant}.*{newline} {
						  yytext[yyleng-1] = '\0';
						  updateSourceInfo(yytext);
                        }

{dec_constant}		    { addSrcCode(); yylval.i = atoi(yytext);   return CONSTANT; }
"void" 					{ addSrcCode(); return VOID; }
"int"					{ addSrcCode(); return INT; }
"char"					{ addSrcCode(); return CHAR; }
{identifier}            { addSrcCode(); yylval.s = dupStr(yytext); return IDENTIFIER; }
[;{}=()+-]				{ addSrcCode(); return *yytext; }

{SP}+                   { addSrcCode(); }

{newline}               { yylineno++; }
                        
.                       { printf("illegal character! ... 0x%02X '%s' in line #%d\n", 
								 *yytext, yytext, yylineno); 
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
        exit (99);
    }
   
    yylineno  = 1;
    if ( yyparse() )     // if fail to parse, stop!
    {
        printf("yyparse stopped at #Line %d\n", yylineno);
        exit (99);
    }
	
	free(currentFile);
    return fatalError; 
}

void addSrcCode(void)
{
	if ( emptySrc )
		srcBuf[0] = '\0';
	
	if ( (strlen(srcBuf) + yyleng) < SRC_BUF_SIZE )
	{
		strcat(srcBuf, yytext);	
		emptySrc = 0;
	}
}

char *getSrcCode(void)
{
	if ( emptySrc )
		return NULL;

	emptySrc = 1;
	return skipSP(srcBuf);
}

void updateSourceInfo(char *file)
{
	int i = 0;
	
	while ( !isdigit(file[i]) ) i++;
	yylineno = atoi(&file[i]);
	while (  isdigit(file[i]) ) i++;
	
	free(currentFile);
	currentFile = dupStr(skipSP(&file[i]));
	emptySrc = 1;
}

/************************************************************/
int yywrap (void) 
{ 
    fclose (yyin);
    return -1;
}

/************************************************************/
int yyerror (char *s)
{
    fflush (stdout);
    fprintf (stdout, "\n%s:%d: %s: token -> '%s'\n", currentFile, yylineno, s, yytext);
    fatalError++;
    return 0;
}
