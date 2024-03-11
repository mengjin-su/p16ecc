/********************************************************************
********************************************************************/
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define IS_SPACE(c)		((c) == ' ' || (c) == '\t')
#define IS_SLASH(c)		((c) == '/' || (c) == '\\')
#define IS_SEPERATOR(c)	((c) == '=' || (c) == ';' )
#define CC_FOLDER_NAME1	"\\p16ecc"
#define CC_FOLDER_NAME2	"/p16ecc"
#define CC_FOLDER_LEN	strlen(CC_FOLDER_NAME2)

typedef struct _File_t {
	char			*fname;
	FILE 			*yyin;
	int  			yylineno;
	YY_BUFFER_STATE yy_state;
	struct _File_t  *next;
} File_t;

FILE *fout = NULL;
char  includeType = 0;
char  currentFile[4096];
char  libFile[4096];
char *libPath = "../include/";
File_t *fileStack = NULL;

int  lineMark = 1;
char lastChar = 0;
int  define_state= 0;
int  error_count = 0;
        
int  cpp1(char *in_file);
void outputSrc(char *str);
char *skipSP(char *s);
int  search(char *file);

%}

escape_code             [ntbrfv\\'"a?]
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


%x COMMENT
%x INCLUDE
%x DEFINE
%x DEFINE_COMMENT

%%

"//".*                  	{ /* line-end comments, remove it. */ }
"/*"                    	{ BEGIN(COMMENT); }
<COMMENT>"*/"           	{ BEGIN(INITIAL); lineMark = 1; }
<COMMENT>{newline}      	{ yylineno++; }
<COMMENT>.              	{ /* C style comments, remove it. */ }

^{SP}*"#include"{SP}+		{ BEGIN(INCLUDE); }
<INCLUDE>{string_constant}.*{newline} 	|
<INCLUDE>{lib_inc_file}.*{newline} 		{
							  yylineno++;
							  BEGIN(INITIAL);
							  cpp1(yytext);
							}
							
^{SP}*"#define"{SP}+ | 
^{SP}*"#pragma"{SP}+	    { BEGIN(DEFINE);  outputSrc(yytext); define_state = -1;}

<DEFINE>"//".*{newline}		{ BEGIN(INITIAL); outputSrc("\n"); yylineno++; }
<DEFINE>{string_constant}	{ outputSrc(yytext); }
<DEFINE>[ \t]+				{ if ( define_state ) outputSrc(yytext); }
<DEFINE>"/*"				{ BEGIN(DEFINE_COMMENT); }
<DEFINE>.					{ if ( define_state == 0 )
							  {
								outputSrc(" "); 
								define_state = 1;	
							  }
							  outputSrc(yytext); 
							}
<DEFINE>{SP}*"\\".*{newline} { yylineno++; define_state = 0; }
<DEFINE>{newline}			{ BEGIN(INITIAL); outputSrc(yytext); yylineno++; 
							  if ( define_state >= 0 ) lineMark = 1;
							}

<DEFINE_COMMENT>"*/"		{ BEGIN(DEFINE); }
<DEFINE_COMMENT>.			;
<DEFINE_COMMENT>{newline}	{ yylineno++; }

{newline}               	{
                          	  outputSrc(yytext);
							  yylineno++;
                        	}
{string_constant}			{ outputSrc(yytext); }
.                      		{ outputSrc(yytext); }

<<EOF>>						{ if ( lastChar != '\n' ) 
								outputSrc("\n"); // add an extra CR
								
							  fclose(yyin);
							  
							  if ( fileStack == NULL )
								yyterminate();
							  else	
							  { // restore parent status...
								File_t *next = fileStack->next;
	
								yy_delete_buffer(YY_CURRENT_BUFFER);
								yy_switch_to_buffer(fileStack->yy_state);
								yyin = fileStack->yyin;
								yylineno = fileStack->yylineno;
	
								strcpy(currentFile, fileStack->fname);
								if ( next == NULL ) free(fileStack->fname);
								free(fileStack);
								fileStack = next;
	
								lineMark = 1;
								lastChar = 0;
							  }
							}
%%

/************************************************************/
int main(int argc, char *argv[], char *env[])
{
	if ( argc > 1 )
	{
		char output_file[4096];
		
		strcpy(currentFile, argv[1]);
		sprintf(output_file, "%s_", currentFile);
		fout = fopen(output_file, "w");
		
		for (; env && *env; env++) 
			if ( strncasecmp(*env, "Path=", 5) == 0 )
			{
				char *p = strstr(*env, CC_FOLDER_NAME1);
				if ( !p ) p = strstr(*env, CC_FOLDER_NAME2);

				if ( p && IS_SLASH(p[CC_FOLDER_LEN]) && !memcmp(p+CC_FOLDER_LEN+1, "bin", 3) )
				{
					int i = 0;
					while ( !IS_SEPERATOR(p[i-1]) ) i--;
					
					libPath = malloc(CC_FOLDER_LEN-i + 16);
					memcpy(libPath, &p[i], CC_FOLDER_LEN-i);
					strcpy(&libPath[CC_FOLDER_LEN-i], "/include/");
					break;
				}
			}
		
		if ( cpp1(currentFile) == 0 )
			yylex();

		fclose(fout);
		if ( libPath[0] != '.' ) free(libPath);
    }
    return error_count;
}

/************************************************************/
int cpp1(char *in_file)
{
	int  i;
	FILE *fin;
	char file_name[4096];
	char c = (*in_file == '"')? '"' :
			 (*in_file == '<')? '>' : 0;

	lineMark = 1;
	switch ( includeType = c )
	{
		case '"':	// include a user file...
			strcpy(file_name, skipSP(in_file+1));
			*strchr(file_name, c) = '\0';
			break;

		case '>':	// include a lib file...
			sprintf(file_name, "%s%s", libPath, skipSP(in_file+1));
			*strchr(file_name, c) = '\0';
			break;
		
		default:	// input file
			strcpy(file_name, in_file);
			break;
	}

	if ( c && search(file_name) )
		return 0;
	
	fin = fopen(file_name, "r");
    if ( fin == NULL )
	{
        printf ("can't open file - %s!\n", file_name);
		error_count++;
		return -1;
	}

	if ( c )	// nested include file ...
	{	// save current status...
		File_t *fp = malloc(sizeof(File_t));
		fp->fname  = malloc(strlen(currentFile)+1);
		strcpy(fp->fname, currentFile);
		strcpy(currentFile, file_name);
		
		fp->yyin     = yyin;	
		fp->yylineno = yylineno;
		fp->yy_state = YY_CURRENT_BUFFER;
		fp->next     = fileStack;
		fileStack    = fp;
		
		yy_switch_to_buffer(yy_create_buffer(fin, YY_BUF_SIZE));
	}

	yyin = fin;
	yylineno = 1;
	lastChar = 0;
	return 0;
}

/************************************************************/
int yywrap (void) 
{
	return -1;
}

/************************************************************/
void outputSrc(char *str)
{
	int i, len = strlen(str);
	
	for (i = 0; i < len; i++)
	{
		char c = *str++;
		if ( fout != NULL && c != '\r' )
		{
			if ( lineMark )
			{
				if ( lastChar && lastChar != '\n')
					fprintf(fout, "\n");
			
				if ( includeType == '>' )
					fprintf(fout, "#LINE %d %s\n", yylineno, currentFile);
				else
					fprintf(fout, "#line %d %s\n", yylineno, currentFile);
			}

			lineMark = 0;	
			fprintf (fout, "%c", lastChar = c);
		}
	}
}

/************************************************************/
int yyerror(char *s)
{
    fflush(stdout);
	error_count++;
    return 0;
}

/************************************************************/
char *skipSP(char *s)
{
	while ( IS_SPACE(*s) ) s++;
	return s;
}

/************************************************************/
int search(char *file)
{
	File_t *fp = fileStack;
	for (; fp; fp = fp->next)
		if ( strcmp(fp->fname, file) == 0 )
			return 1;
		
	return 0;
}
