/********************************************************************
********************************************************************/
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "common.h"
#include "ascii.h"
#include "cc.h"

extern int yyparse();
int fatalError=0;
char *currentFile = NULL;

#define SRC_BUF_SIZE    4096
char srcBuf[SRC_BUF_SIZE], emptySrc = 1;

/* --- constant convertion types --- */
enum {C_OCT, C_HEX, C_CHAR, C_BIN};
int  aConstant(char *s, int type);
int  constantType(char *s);
void addSrcCode(void);
void updateSourceInfo(char *);

%}

letter                  [A-Za-z]
digit                   [0-9]
underscore              "_"
following_character     ({letter}|{digit}|{underscore})
identifier              ({letter}|{underscore}){following_character}*

nonzero_digit           [1-9]
oct_digit               [0-7]
hex_digit               [0-9a-fA-F]
integer_suffix          [uU]|[lL]|([uU][lL])|([lL][uU])
dec_constant        	({nonzero_digit}{digit}*)
oct_constant          	(0{oct_digit}*)
hex_constant    		(0[xX]{hex_digit}+)
bin_constant         	(0[bB][01]+)

char_escape_code        [ntbrfv\\'"a?]
oct_escape_code       	({oct_digit}{1,3})
hex_escape_code         (x{hex_digit}+)
escape_code             ({char_escape_code}|{oct_escape_code}|{hex_escape_code})
escape_character        (\\{escape_code})

c_char                  ([^'\\\n]|{escape_character})
c_char_sequence         ({c_char}+)
character_constant      (\L?\'{c_char_sequence}\')

s_char                  ([^"\\\n]|{escape_character})
s_char_sequence         ({s_char}+)
string_constant         \L?\"{s_char_sequence}?\"

SP                      [ \t\v\f\015]
newline                 [\n]

%%

^"#line"{SP}+{dec_constant}.*{newline} {
						  yytext[yyleng-1] = '\0';
						  updateSourceInfo(yytext);
                        }
{dec_constant}{integer_suffix}? { 
						  addSrcCode(); yylval.i = atoi(yytext);
                          return constantType(yytext);
                        }
{hex_constant}{integer_suffix}? { 
						  addSrcCode(); yylval.i = aConstant(yytext, C_HEX);
                          return constantType(yytext);
                        }
{oct_constant}{integer_suffix}? { 
						  addSrcCode(); yylval.i = aConstant(yytext, C_OCT);
                          return constantType(yytext);
                        }
{bin_constant}{integer_suffix}? { 
						  addSrcCode(); yylval.i = aConstant(yytext, C_BIN);
                          return constantType(yytext);
                        }
{character_constant}    { addSrcCode(); yylval.i = aConstant(yytext, C_CHAR);  
                          return C_CONSTANT;
                        }
{string_constant}       { addSrcCode();
						  yytext[yyleng - 1] = 0;
                          yylval.s = parseRawStr(yytext+1);
                          return STRING;
                        }
"unsigned"{SP}+"char"   { addSrcCode(); return UCHAR; }
"unsigned"{SP}+"int"    { addSrcCode(); return UINT; }
"unsigned"{SP}+"short"  { addSrcCode(); return USHORT; }
"unsigned"{SP}+"long"   { addSrcCode(); return ULONG; }
("signed"{SP}+)?"char"  { addSrcCode(); return CHAR; }
("signed"{SP}+)?"short" { addSrcCode(); return SHORT; }
("signed"{SP}+)?"int"   { addSrcCode(); return INT; }
("signed"{SP}+)?"long"  { addSrcCode(); return LONG; }							
"void" 					{ addSrcCode(); return VOID; }						
"sbit"                  { addSrcCode(); return SBIT; }
"break"                 { addSrcCode(); return BREAK; }
"case"                  { addSrcCode(); return CASE; }
"const"                 { addSrcCode(); return CONST; }
"continue"              { addSrcCode(); return CONTINUE; }
"default"               { addSrcCode(); return DEFAULT; }
"do"                    { addSrcCode(); return DO; }
"else"                  { addSrcCode(); return ELSE; }
"enum"                  { addSrcCode(); return ENUM; }
"extern"                { addSrcCode(); return EXTERN; }
"for"                   { addSrcCode(); return FOR; }
"goto"                  { addSrcCode(); return GOTO; }
"if"                    { addSrcCode(); return IF; }
"eeprom"                { addSrcCode(); return EEPROM; }
"register"              { addSrcCode(); return REGISTER; }
"return"                { addSrcCode(); return RETURN; }
"sizeof"                { addSrcCode(); return SIZEOF; }
"static"                { addSrcCode(); return STATIC; }
"switch"                { addSrcCode(); return SWITCH; }
"volatile"              { addSrcCode(); return VOLATILE; }
"while"                 { addSrcCode(); return WHILE; }
"interrupt"             { addSrcCode(); return INTERRUPT; }
"union"                 { addSrcCode(); return UNION; }
"struct"                { addSrcCode(); return STRUCT; }
"typedef"               { addSrcCode(); return TYPEDEF; }
"..."                   { addSrcCode(); return ELIPSIS; }
"_linear_"              { addSrcCode(); yylval.i = LINEAR;	return MEM_BANK; }

{identifier}            { addSrcCode(); yylval.s = dupStr(yytext); return IDENTIFIER; }

"+="                    { addSrcCode(); yylval.i = ADD_ASSIGN; return ASSIGN_OP; }
"-="                    { addSrcCode(); yylval.i = SUB_ASSIGN; return ASSIGN_OP; }
"*="                    { addSrcCode(); yylval.i = MUL_ASSIGN; return ASSIGN_OP; }
"/="                    { addSrcCode(); yylval.i = DIV_ASSIGN; return ASSIGN_OP; }
"%="                    { addSrcCode(); yylval.i = MOD_ASSIGN; return ASSIGN_OP; }
"<<="                   { addSrcCode(); yylval.i = LEFT_ASSIGN; return ASSIGN_OP; }
">>="                   { addSrcCode(); yylval.i = RIGHT_ASSIGN; return ASSIGN_OP; }
"&="                    { addSrcCode(); yylval.i = AND_ASSIGN; return ASSIGN_OP; }
"^="                    { addSrcCode(); yylval.i = XOR_ASSIGN; return ASSIGN_OP; }
"|="                    { addSrcCode(); yylval.i = OR_ASSIGN;  return ASSIGN_OP; }
"=="                    { addSrcCode(); yylval.i = EQ_OP; return EQUALITY_OP; }
"!="                    { addSrcCode(); yylval.i = NE_OP; return EQUALITY_OP; }
"<="                    { addSrcCode(); yylval.i = LE_OP; return RELATIONAL_OP; }
">="                    { addSrcCode(); yylval.i = GE_OP; return RELATIONAL_OP; }
[<>]                    { addSrcCode(); yylval.i = *yytext; return RELATIONAL_OP; }
"<<"                    { addSrcCode(); yylval.i = LEFT_OP; return SHIFT_OP; }
">>"                    { addSrcCode(); yylval.i = RIGHT_OP; return SHIFT_OP; }
"++"                    { addSrcCode(); return INC_OP; }
"--"                    { addSrcCode(); return DEC_OP; }
"||"                    { addSrcCode(); return OR_OP; }
"&&"                    { addSrcCode(); return AND_OP; }
"->"                    { addSrcCode(); return PTR_OP; }

[;{}=(),:&|!~*/%^@+-]	{ if ( *yytext != '{' && *yytext != '}' ) addSrcCode(); 
						  return *yytext; }
"." 					{ addSrcCode(); return '.'; }
"?"						{ addSrcCode(); return '?'; }
"["						{ addSrcCode(); return '['; }
"]"						{ addSrcCode(); return ']'; }

{SP}+                   { addSrcCode(); }

{newline}               { yylineno++; }
                        
.                       { printf("illegal character! ... 0x%02X '%s' in line #%d\n", 
								 *yytext, yytext, yylineno); 
						}

%%

/************************************************************/
int _main(char *filename)
{
	currentFile = dupStr(filename);
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

int aConstant(char *s, int type)
{
    int n = 0;
    switch ( type )
    {
        case C_HEX:	sscanf (s, "%x", &n); 
					break;
					
        case C_OCT: for (; *s >= '0' && *s < '8'; s++)
						n = (n*8) + *s - '0';
					break;
					
        case C_BIN: s += 2;
					while ( *s == '0' || *s == '1' )
						n = (n << 1) | (*s++ - '0');
					break;
					
        case C_CHAR:n = (s[1] == '\\')? parseRawChr(&s[2]): s[1];
					break;
	}
    return n;
}

int constantType(char *s)
{
	int len = strlen(s);
	int type = 0;
	
	while ( len-- )
		switch ( s[len] )
		{
			case 'U': case 'u':	type |= 1;	break;
			case 'L': case 'l': type |= 2;	break;
		}
		
	return (type == 1)? U_CONSTANT:
		   (type == 2)? L_CONSTANT:	
		   (type == 3)? UL_CONSTANT: CONSTANT;
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
