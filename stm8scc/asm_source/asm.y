%{

#include "define.h"
#include "common.h"

extern int yylineno;
extern int yylex();

/* reference to page 69 of "lex & yacc" */
//void yyerror (char *s);

%}

%union {
    int         value;
    char        *name;
    line_t      *line;
    item_t      *item;
    area_t      *area;
}

%token <name>   LABEL   SYMBOL  STRING	S_COMMENT
%token <value>  NUMBER  ST8_INST REG

%token ADD ADDW ADC AND BCCM BCP BCPL BRES BSET BTJF BTJT CALL CALLF CALLR
%token CCF CLR CLRW CP CPW CPL CPLW DEC DECW DIV DIVW EXG EXGW HALT INC INCW
%token IRET JP JPF JRA JRC JREQ JRF JRH JRIH JRIL JRM JRMI JRNC JRNE JRNH
%token JRNM JRNV JRPL JRSGE JRSGT JRSLE JRSLT JRT JRUGE JRUGT JRULE JRULT JRV
%token LD  LDF  LDW MOV MUL NEG NEGW NOP OR POP POPW PUSH PUSHW RCF RET RETF
%token RIM RLC RLCW RLWA RRC RRCW RRWA RVF SBC SCF SIM SLL SLLW SLA SLAW
%token SRA SRAW SRL SRLW SUB SUBW SWAP SWAPW TNZ TNZW TRAP WFE WFI XOR
%token _JP _JBZ _JBNZ

%token <i> REG_A REG_X REG_Y REG_SP REG_XH REG_XL REG_YH REG_YL REG_CC

%token  RS      EQU		DB		DW		DT		DD		VECT
%token  END     SEGMENT
%token  INCLUDE CBLANK	DBLANK
%token  DEVICE  INVOKE  STM8

%token  RSHIFT
%token  LSHIFT

%left '-' '+'
%left '*' '/' '%' RSHIFT LSHIFT
%left '&' '|' '^' '~'
%nonassoc UMINUS
%nonassoc INVERSE

%type <line>  prog lines line line_eol source_line
%type <item>  exp primary_exp addition_exp multiply_exp
%type <item>  shift_exp and_exp xor_exp or_exp param operands operand
%type <area>  a_items a_item

%%
prog
    :   lines                       {   line_ptr = $1; }
    ;

lines
    :   line                        {   $$ = $1; }
    |   lines line                  {   $$ = $1;
                                        appendLine(&$1, $2); 
                                    }
    ;

line
	:	line_eol					{	$$ = $1;
                                        $$->src = dupStr(__yyline);
                                        $$->lineno = yylineno;	
									}
	;	
line_eol
    :   '\n'                        {   $$ = newLine(NULL, 0, NULL); 		 }
	|	LABEL '\n'                  {	$$ = newLine($1, 0, NULL); free($1); }                  
    |   source_line '\n'            {   $$ = $1; 							 }
	|	LABEL source_line '\n'		{   $$ = $2; addLabel($$, $1); free($1); }
    ;
	
source_line
    :   ST8_INST                    {   $$ = newLine(NULL, $1, NULL); }
    |   ST8_INST operands           {   $$ = newLine(NULL, $1,   $2); }
    |   SEGMENT SYMBOL 				{   $$ = newSegLine($2, NULL, NULL); }
    |	SEGMENT SYMBOL 
		'(' a_items ')'  			{   $$ = newSegLine($2, $4, NULL); }
    |	SEGMENT SYMBOL 
    	'(' a_items ')' param	   	{   $$ = newSegLine($2, $4, $6); }
	|	S_COMMENT					{	$$ = newLine(NULL, S_COMMENT, strItem($1)); 
										free($1);
									}
    ;

param
	:	SYMBOL						{	$$ = newItem(0);
										$$->left  = symItem($1); free($1);
									}
	|	SYMBOL ':' NUMBER			{	$$ = newItem(':');
                                	    $$->left  = symItem($1); free($1);
										$$->right = valItem($3); 
									}
    ;

a_items
    :   a_item                      {   $$ = $1; }
	|	a_items ',' a_item			{	$$ = mergeArea($1, $3); }
	;
    
a_item
	:	exp							{	$$ = newArea($1, 0); 
										if ( $$ == NULL )
											yy_error("illegal area specifier!");
									}
	| 	'=' exp						{	$$ = newArea($2, '='); }
	;
    
operands
	:	operand						{   $$ = $1; }
	|	operands ',' operand		{   $$ = appendItem($1, $3); }
	;
	
operand
    :   exp                         {   $$ = $1; }
   	|	'#' exp						{	$$ = newItem('#'); $$->left = $2; }
    |	'[' exp ']'					{   $$ = newItem(TYPE_INDIR);
    									$$->left = $2;
    								}
    |	'(' exp ',' REG ')'			{	$$ = newItem(TYPE_INDEX);
    									$$->left = $2; 
    									$$->right = regItem($4); 
    								}
    |	'(' '[' exp ']' ',' REG ')'	{	$$ = newItem(TYPE_INDIR_INDEX);
    									$$->left = $3;
    									$$->right = regItem($6); 
    								}
    |	'(' REG ')'					{	$$ = newItem(TYPE_INDEX); 	
    									$$->right = regItem($2);
    								}
    |	REG							{	$$ = regItem($1); }
    ;

primary_exp
	:   '(' exp ')'                 {   $$ = $2; }
    |   '+' primary_exp            	{   $$ = $2; }	
    |   '-' primary_exp            	{   $$ = newItem(UMINUS);
                                        $$->left = $2;
                                    }
    |   '~' primary_exp           	{   $$ = newItem(INVERSE);
                                        $$->left = $2;
                                    }
    |   STRING                      {   $$ = strItem($1); free($1); }
    |   SYMBOL                      {   $$ = symItem($1); free($1); }
	|   NUMBER                      {   $$ = valItem($1);  }
    |   '.'                         {   $$ = newItem('.'); }
	;

multiply_exp
	:	primary_exp					{	$$ = $1; }
	|	multiply_exp '*' 
		primary_exp					{   $$ = newItem('*');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	|	multiply_exp '/'
		primary_exp					{   $$ = newItem('/');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	|	multiply_exp '%'
		primary_exp					{   $$ = newItem('%');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	;
	
addition_exp 
	:  	multiply_exp				{	$$ = $1; }
	|	addition_exp '+'
		multiply_exp				{   $$ = newItem('+');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	|	addition_exp '-'
		multiply_exp				{   $$ = newItem('-');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	;
	
shift_exp
    : addition_exp	  	       		{   $$ = $1; }
    | shift_exp LSHIFT 
      addition_exp	          		{   $$ = newItem(LSHIFT);
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
    | shift_exp RSHIFT 
      addition_exp	          		{   $$ = newItem(RSHIFT);
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
    ;

and_exp
	:	shift_exp					{	$$ = $1; }
	|	and_exp '&'	shift_exp		{   $$ = newItem('&');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	;                                        
	
xor_exp
	:	and_exp
	|	xor_exp '^'	and_exp			{   $$ = newItem('^');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	;                                        

or_exp
	:	xor_exp	
	|	or_exp '|' xor_exp			{   $$ = newItem('|');
                                        $$->left  = $1;
                                        $$->right = $3; 
                                    }
	;                                        
	
exp
	:	or_exp						{	$$ = $1; }
	|	'*' or_exp					{	$$ = $2; $$->bank0 = 1; }
    ;
    
%%

/************************************************************************/
void yyerror(const char *s)
{
    printf("%s\n", s);
    error_cnt++;
}

/************************************************************************/
void my_yyerror(line_t *line, const char *s)
{
    if ( line != NULL )
        printf("[%s] #%d: ", line->fname, line->lineno);

    yyerror((char*)s);
}
