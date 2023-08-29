%{
#include <stdio.h>
#include "common.h"

extern int yylineno;
extern int yylex();

/* reference to page 69 of "lex & yacc" */
void yyerror(char *s);

%}

%union {
    int	 	value;	// integer value
    char   	*name;	// any symbol/string
    line_t 	*line;	// an assembly line
    item_t 	*item;	// an operand/expr
    attr_t 	*attr;	// segment attribution
}

%token <name>   LABEL   SYMBOL  STRING	S_COMMENT
%token <value>  NUMBER  PIC_INST

%token ADDWF ANDWF CLRF COMF DECF DECFSZ INCF INCFSZ
%token IORWF MOVF MOVWF RLF RRF SUBWF NOP SWAPF XORWF
%token BCF BSF BTFSC BTFSS CLRWDT GOTO SLEEP
%token CALL RETFIE RETLW RETURN
%token ADDLW ANDLW IORLW MOVLW SUBLW XORLW CLRW 
%token RESET CALLW BRW MOVIW MOVWI MOVLP MOVLB
%token ADDFSR BRA LSLF LSRF ASRF SUBWFB ADDWFC
%token MOVIW_OP MOVWI_OP MOVIW_OFF MOVWI_OFF
%token FCALL	RS      EQU		DW		DBLANK	CBLANK
%token END     	SEGMENT INVOKE  DEVICE  PSEL 	BSEL
%token RSHIFT LSHIFT PLUS_PLUS	MINUS_MINUS
%token FSR_PRE_INC FSR_POST_INC FSR_PRE_DEC FSR_POST_DEC FSR_OFFSET


%left '-' '+'
%left '*' '/' '%' RSHIFT LSHIFT
%left '&' '|' '^' '~'
%nonassoc UMINUS
%nonassoc INVERSE

%type <line>  prog lines line source_line
%type <item>  opernd items exp primary_exp addition_exp 
%type <item>  multiply_exp shift_exp and_exp xor_exp or_exp param
%type <attr>  attrs attr
%type <item>  e_opernd

%%
prog
    :   lines                 	{   linePtr = $1; }
    ;

lines
    :   line                  	{   $$ = $1; }
    |   lines line            	{   $$ = $1;
									appendLine(&$1, $2); 
                                }
    ;

line
    :   source_line '\n'        {   $$ = $1; 
                                    $$->src = dupStr(__yyline);
                                    $$->lineno = yylineno;
                                }
    |   '\n'                    {   $$ = newLine(NULL, 0, NULL); 
                                    $$->src = dupStr(__yyline);
                                    $$->lineno = yylineno;
                                }
    ;
	
source_line
    :	LABEL					{	$$ = newLine($1, 0,  NULL); free($1); }
	|   LABEL PIC_INST        	{	$$ = newLine($1, $2, NULL); free($1); }
	|	LABEL PIC_INST opernd	{	$$ = newLine($1, $2, $3  ); free($1); }
	|  		  PIC_INST        	{	$$ = newLine(NULL, $1, NULL); }
	|		  PIC_INST opernd	{	$$ = newLine(NULL, $1, $2); }
	|	S_COMMENT				{	$$ = newLine(NULL, S_COMMENT, strItem($1));  free($1); }
    |   SEGMENT SYMBOL 			{   $$ = newSegLine($2, NULL, NULL); }
    |	SEGMENT SYMBOL 
		'(' attrs ')'  			{   $$ = newSegLine($2, $4, NULL); }
    |	SEGMENT SYMBOL 
		'(' attrs ')' param	    {   $$ = newSegLine($2, $4, $6); }
    ;
	
attrs
    :   attr	                {   $$ = $1; }
	|	attrs ',' attr			{	$$ = mergeAttr($1, $3); }
	;
    
attr
	:	exp						{	$$ = newAttr($1, 0); 
									if ( $$ == NULL )
										yyerror("illegal attr. specifier!");
								}
	| 	'=' NUMBER				{	$$ = newAttr(valItem($2), '='); }
	;
    
param
	:	SYMBOL					{	$$ = newItem(0);
									$$->left  = symItem($1); free($1);
								}
	|	SYMBOL ':' NUMBER		{	$$ = newItem(':');
                                    $$->left  = symItem($1); free($1);
									$$->right = valItem($3); 
								}
	;
	
opernd
    :   items                   {   $$ = $1; }
    |	e_opernd				{	$$ = $1; }
	;
    
items
    :   exp                   	{   $$ = $1; }
    |   items ',' exp           {   $$ = appendItem($1, $3); }
    ;

primary_exp
	:   '(' exp ')'             {   $$ = $2; }
    |   '+' primary_exp         {   $$ = $2; }	
    |   '-' primary_exp         {   $$ = newItem(UMINUS); $$->left = $2; }
    |   '~' primary_exp         {   $$ = newItem(INVERSE);$$->left = $2; }
    |   SYMBOL                  {   $$ = symItem($1); free($1); }
    |	STRING					{	$$ = strItem($1); free($1); }	
    |   NUMBER                  {   $$ = valItem($1);  }
    |   '.'                     {   $$ = newItem('.'); }
	;

multiply_exp
	:	primary_exp
	|	multiply_exp '*'
		primary_exp				{   $$ = newItem('*');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	|	multiply_exp '/'
		primary_exp				{   $$ = newItem('/');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	|	multiply_exp '%'
		primary_exp				{   $$ = newItem('%');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	;
	
addition_exp 
	:  	multiply_exp
	|	addition_exp '+'
		multiply_exp			{   $$ = newItem('+');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	|	addition_exp '-'
		multiply_exp			{   $$ = newItem('-');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	;
	
shift_exp
    : addition_exp
    | shift_exp LSHIFT 
      addition_exp	            {   $$ = newItem(LSHIFT);
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
    | shift_exp RSHIFT 
      addition_exp	            {   $$ = newItem(RSHIFT);
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
    ;

and_exp
	:	shift_exp

	|	and_exp '&'	shift_exp	{   $$ = newItem('&');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	;                                        
	
xor_exp
	:	and_exp
	|	xor_exp '^'	and_exp		{   $$ = newItem('^');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	;                                        

or_exp
	:	xor_exp
	|	or_exp '|' xor_exp		{   $$ = newItem('|');
                                    $$->left  = $1;
                                    $$->right = $3; 
                                }
	;                                        
	
exp
	:	or_exp					{	$$ = $1; }
    ;
    
e_opernd
	:	PLUS_PLUS SYMBOL		{	$$ = newItem(FSR_PRE_INC);	$$->left = symItem($2); free($2); }
	|	MINUS_MINUS SYMBOL		{	$$ = newItem(FSR_PRE_DEC); 	$$->left = symItem($2); free($2); }
	|	SYMBOL PLUS_PLUS		{	$$ = newItem(FSR_POST_INC);	$$->left = symItem($1); free($1); }
	|	SYMBOL MINUS_MINUS		{	$$ = newItem(FSR_POST_DEC);	$$->left = symItem($1); free($1); }
	|	exp '[' SYMBOL ']'		{	$$ = newItem(FSR_OFFSET); 
									$$->left = $1;
									$$->right= symItem($3); free($3);
								}
    
%%
