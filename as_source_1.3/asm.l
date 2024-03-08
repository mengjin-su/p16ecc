%{

#include <string.h>
#include <ctype.h>
#include "common.h"
#include "asm.h"

int yyparse();

#define YYLINE_BUF_SIZE   4096
char __yyline[YYLINE_BUF_SIZE];
char *__yytext;

/* --- instruction descriptor --- */
typedef struct {
    char *name;
    int  token;
} inst_t;

/* --- data structures --- */
typedef struct _f {
    char     *name;     /* filename */
    void     *last;
    FILE     *fptr;
    int       lnum;
    struct _f *next;
} file_t;

/* --- local functions --- */
file_t *fileList = NULL;
char   *currentFile;

file_t *searchStrName(file_t *head, char *name);
file_t *addStrName(file_t *head, char *name);

void    appendStr(void);
int     convertNum(char *str);
inst_t *searchInst(char *str);

%}

sp                          [ \t\015]
symbol                      [_a-zA-Z][_a-zA-Z0-9$]*
hex                         [a-fA-F0-9]
dec                         [0-9]

s_char                  	([^"\r\n])
s_char_sequence         	({s_char}+)
string			         	\"{s_char_sequence}?\"

%x OPCODE
%x OPERAND 

%%

^{sp}*";".* 				{   appendStr(); 
								if ( memcmp(yytext, "; :: ", 5) == 0 )
								{
									yylval.name = dupStr(yytext);
									return S_COMMENT;
								}
							}
^{sp}+{symbol}:(:)?         {   int i = 0;
								appendStr();
								while ( yytext[i] == ' ' || yytext[i] == '\t' ) i++;
                                BEGIN(OPCODE);
                                yylval.name = dupStr(&yytext[i]);
                                return LABEL;
							}
^{symbol}((:?):)?           {   appendStr();
                                BEGIN(OPCODE);
                                yylval.name = dupStr(yytext);
                                return LABEL;
                            }
^{sp}+                      {   appendStr();
                                BEGIN(OPCODE); 
							}
\n                          {   appendStr();
                                return yytext[0]; 
                            }							
<OPCODE>(".")?{symbol}      {   inst_t *ip;
                                appendStr();
                                BEGIN(OPERAND);
                        
                                ip = searchInst(yytext);
                                if ( ip != NULL )
                                {   
                                    yylval.value = ip->token;
                                    return (ip->token == SEGMENT)? SEGMENT: PIC_INST;
                                }
                                
                                printf("Line #%d: ", yylineno-1);
                                printf("illegal opcode - %s\n", yytext);
                            }
<OPCODE>{sp}*(";".*)?       {   appendStr(); }
<OPCODE>\n                  {   appendStr();
                                BEGIN(INITIAL);
                                return yytext[0]; 
                            }
<OPERAND>{string}           {   appendStr();
								yytext[yyleng-1] = 0;
                                yylval.name = dupStr(yytext+1);
                                return STRING;  
                            }                 
<OPERAND>{symbol}           {   appendStr();
                                yylval.name = dupStr(yytext);
                                return SYMBOL;  
                            }
<OPERAND>0[xX]{hex}+        |
<OPERAND>{dec}+             {   appendStr();
                                yylval.value = convertNum((char *)yytext);
                                return NUMBER;
                            }
<OPERAND>">>"               {   appendStr(); return RSHIFT; }
<OPERAND>"<<"               {   appendStr(); return LSHIFT; }

<OPERAND>[-+%^:&|=,()*/~] 	{   appendStr();
                                return yytext[0]; 
                            }
<OPERAND>{sp}*";".*         {   appendStr(); }
<OPERAND>{sp}+              {   appendStr(); }
<OPERAND>"++"               {   appendStr();
                                return PLUS_PLUS;
                            }
<OPERAND>"--"               {   appendStr();
                                return MINUS_MINUS;
                            }
<OPERAND>"["				{	appendStr();	return '['; }
<OPERAND>"]"				{	appendStr();	return ']'; }
<OPERAND>"."                {   appendStr();	return '.'; }
<OPERAND>\n                 {   appendStr();
                                BEGIN(INITIAL);
                                return yytext[0]; 
                            }
.                           {   appendStr ();
                                yyerror ("unknown character!");
                            }

%%

/* ---------------------------------------------------------------------------- */
int yywrap(void) 
{ 
    fclose (yyin);
    return -1;
}

static char last_ch = '\n';
/* ---------------------------------------------------------------------------- */
void appendStr(void)
{
    int l = strlen(yytext);
    
    if ( last_ch == '\n' )
    {
        yylineno++;
        __yyline[0] = '\0';
    }

    last_ch = yytext[0];

    if ( (strlen (__yyline) + l) < YYLINE_BUF_SIZE )
        strcat (__yyline, yytext);
}

/* ---------------------------------------------------------------------------- */
int convertNum(char *str)
{
    int n = 0, i = 0, radix = 10;

    if ( (str[i] == '0' && toupper(str[i+1]) == 'X') )
    {
        i += 2;
        radix = 16;
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
const inst_t instCodeTable[] = {
    {"addwf",	ADDWF},
    {"andwf",   ANDWF},
    {"clrf",    CLRF},
    {"comf",    COMF},
    {"decf",    DECF}, 
    {"decfsz",  DECFSZ},
    {"incf",    INCF},
    {"incfsz",  INCFSZ},
    {"iorwf",   IORWF},
    {"movf",    MOVF},
    {"movwf",   MOVWF},
    {"subwf",   SUBWF},
    {"swapf",   SWAPF},
    {"xorwf",   XORWF},
    {"bcf",     BCF},
    {"bsf",     BSF},
    {"btfsc",   BTFSC},
    {"btfss",   BTFSS},
    {"call",    CALL},
    {"clrwdt",  CLRWDT},
    {"goto",    GOTO},
    {"nop",     NOP},
    {"retfie",  RETFIE},
    {"retlw",   RETLW},
    {"return",  RETURN},
    {"sleep",   SLEEP},
    {"addlw",   ADDLW},
    {"andlw",   ANDLW},
    {"iorlw",   IORLW},
    {"movlw",   MOVLW},
    {"sublw",   SUBLW},
    {"xorlw",   XORLW},
    {"rlf",     RLF},
    {"rrf",     RRF},
    {"clrw",    CLRW},
    {"addwfc",  ADDWFC},
    {"subwfb",  SUBWFB},
    {"addfsr",  ADDFSR},
    {"movwi",   MOVWI},
    {"moviw",   MOVIW},
    {"reset",   RESET},
    {"movlb",   MOVLB},
    {"movlp",   MOVLP},
    {"callw",   CALLW},
    {"brw",     BRW},
    {"bra",     BRA},
    {"lslf",    LSLF},
    {"lsrf",    LSRF},
    {"asrf",    ASRF},
    {".equ",    EQU},
    {".invoke", INVOKE},
    {".segment",SEGMENT},
    {".rs",     RS},
    {".dw",		DW},
    {".end",    END},
    {".device", DEVICE},
    {".psel",	PSEL},		// page select (PCLATH)
    {".bsel",	BSEL},		// bank select (BSR)
    {".fcall",  FCALL},
    {".dblank",	DBLANK},	
    {".cblank",	CBLANK},
    {NULL,      0}
};

/* ---------------------------------------------------------------------------- */
inst_t *searchInst(char *str)
{
    int i;
	for (i = 0; instCodeTable[i].name != NULL; i++)
    {
        if ( strcasecmp(str, instCodeTable[i].name) == 0 )
            return (inst_t *)&instCodeTable[i];
    }
    return NULL;
}

/************************************************************/
line_t *_main(char *filename)
{
    int yyparse_ret;

    // if the file has been parsed, skip it
    if ( searchStrName(fileList, filename) != NULL )
        return NULL;

    fileList = addStrName(fileList, filename);
    yyin = fopen(filename, "r");

    if ( yyin == NULL )
    {
        printf ("can't open file - %s!", filename);
        exit (99);
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

///////////////////////////////////////////////////////////////
file_t *searchStrName(file_t *head, char *name)
{
    while ( head != NULL )
    {
      	if ( !strcasecmp(head->name, name) )
            return head;

        head = head->next;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////
file_t *addStrName(file_t *head, char *name)
{
    file_t *p = searchStrName(head, name);

    if ( p == NULL )
    {
        p = (file_t*)MALLOC(sizeof(file_t));
        p->name = dupStr(name);
        p->next = head;
    }

    return p;
}

/************************************************************************/
void yyerror(char *s)
{
    printf ("%s\n", s);
    errorCnt++;
}

/************************************************************************/
void my_yyerror(line_t *line, char *s)
{
    if ( line )
        printf("[%s] #%d: ", line->fname, line->lineno);

    yyerror(s);
}
