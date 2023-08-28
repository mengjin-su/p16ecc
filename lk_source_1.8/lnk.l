%{

#include <string.h>
#include <ctype.h>
#include "common.h"
#include "lnk.h"

int yyparse ();

char __yyline[YYLINE_BUF_SIZE];
char *__yytext;
#ifdef __dj_include_ctype_h_
int yylineno;	// DGJPP GCC ...
#endif

/* --- local functions --- */
char   *currentFile;
line_t *linePtr;

int  isAnOpcode(char c);
void appendStr(void);
int  convertNum(char *str);

%}

sp                          [ \t\015]
symbol                      [_a-zA-Z][_a-zA-Z0-9$]*
hex                         [a-fA-F0-9]
dec                         [0-9]

s_char                  	([^"\r\n])
s_char_sequence         	({s_char}+)
string			         	\"{s_char_sequence}?\"

%x OPERAND
%x ESCAPE

%%

^";"{sp}+.*					{	char *p = yytext;
								appendStr(); 
                                for(p++; *p == ' '; p++);
								p[strlen(p)-1] = 0;
								yylval.syml = dupStr(p+1);
                                return COMMENT;
							}
^.{sp}+                     {   appendStr(); 
                                if ( isAnOpcode(yytext[0]) )
                                {
                                    BEGIN(OPERAND);
                                    yylval.value = yytext[0];
                                    return TYPE;
                                }
                                BEGIN(ESCAPE);
                            }
<ESCAPE>.*                  {   /*ignore the line */ }		
<ESCAPE>\n                  {   appendStr();
                                BEGIN(INITIAL); 
                            }
<OPERAND>{symbol}           {   appendStr();
                                yylval.syml = dupStr(yytext);
                                return SYMBOL;
                            }
<OPERAND>{string}			{	int len = strlen(yytext);
								appendStr();
								yytext[len-1] = 0;
								yylval.syml = dupStr(yytext+1);
								return STRING;
							}
<OPERAND>0[xX]{hex}+    |
<OPERAND>{dec}+             {   appendStr();
                                yylval.value = convertNum((char *)yytext);
                                return NUMBER;
                            }
<OPERAND>">>"               {   appendStr(); return RSHIFT; }
<OPERAND>"<<"               {   appendStr(); return LSHIFT; }
<OPERAND>[-+%^:&|()*/~.]    {   appendStr();
                                return yytext[0]; 
                            }
<OPERAND>{sp}+              {   appendStr(); }
<OPERAND>\n                 {   appendStr();
                                BEGIN(INITIAL);
                                return yytext[0]; 
                            }
{sp}+						{   appendStr(); }
\n                          {   appendStr();
                                BEGIN INITIAL;
                                return yytext[0]; 
                            }
.                           {   appendStr();
                                yyerror("unknown character!");
                            }
%%

/* ---------------------------------------------------------------------------- */
int yywrap(void) 
{ 
    fclose(yyin);
    return -1;
}

static char last_ch = '\n';
/* ---------------------------------------------------------------------------- */
void appendStr (void)
{
    int l = strlen (yytext);
    
    if ( last_ch == '\n' )
    {
        yylineno++;
        __yyline[0] = '\0';
    }

    last_ch = yytext[0];

    if ( (strlen(__yyline) + l) < YYLINE_BUF_SIZE )
        strcat(__yyline, yytext);
}

/* ---------------------------------------------------------------------------- */
int convertNum(char *str)
{
    int n = 0, i = 0, radix = 10;

    if ( str[i] == '0' && toupper(str[i+1]) == 'X' )
	{
        radix = 16;
		i += 2;
	}

    while ( (isxdigit(str[i]) && radix == 16) || isdigit(str[i]) )
    {
        char c = toupper(str[i]);
        if ( c > '9' ) c -= 7;
        n = (n * radix) + c - '0';
        i++;
    }

    return n;
}   

/* ---------------------------------------------------------------------------- */
int isAnOpcode(char c)
{
    return (c == 'P' ||     // device
            c == 'S' ||     // segment start
            c == 'U' ||		// segment data specifier
            c == 'G' ||     // global label
            c == 'F' ||		// function call
            c == 'L' ||     // local label
            c == 'R' ||     // reserve area
            c == 'W' ||     // data (word)
            c == 'E' ||     // equalization
            c == 'I' ||     // invoke
            c == 'N' ||     // hole in code space
            c == 'M' ||     // hole in data space
			c == 'K' ||		// .bsel
            c == 'J' )? 1:0;// .psel
}

/************************************************************/
line_t *_main(char *filename)
{
    int yyparse_ret;

    yyin = fopen(filename, "r");

    if ( yyin == NULL )
    {
        printf("can't open file - %s!", filename);
        exit(99);
    }

    currentFile = filename;

    yylineno = 0;
    linePtr = NULL;
   
    yyparse_ret = yyparse();   // parse the input text (file)
    
    if ( yyparse_ret != 0 )     // if fail to parse, stop!
    {
        printf("yyparse stopped at #Line %d\n", yylineno);
        exit(99);
    }

    return linePtr;
}
