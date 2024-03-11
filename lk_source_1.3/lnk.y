%{
#include <stdio.h>
#include "common.h"
extern int yylineno;
%}

%union {
    int      value; 
    char     *syml;
    line_t   *line;
    item_t   *item;
}

%token <value> TYPE RSHIFT LSHIFT UMINUS INVERSE NUMBER
%token <syml> SYMBOL COMMENT STRING

%left '-' '+'
%left '*' '/' '%' RSHIFT LSHIFT
%left '&' '|' '^' '~'
%nonassoc UMINUS
%nonassoc INVERSE


%type <line>  prog lines line source_line
%type <item>  items item
%type <item>  multiplicative_expr additive_expr	shift_expr and_expr 
%type <item>  exclusive_or_expr inclusive_or_expr primary_expr

%%
prog
    :   lines                           {   linePtr = $1; }
    ;

lines
    :   line                            {   $$ = $1; }
    |   lines line                      {   $$ = $1;
                                            appendLine(&$1, $2); 
                                        }
    ;

line
    :   '\n'                            {   $$ = newLine(0, NULL); 
                                            $$->src = dupStr(__yyline);
                                            $$->lineno = yylineno;
                                        }                           
    |   source_line '\n'                {   $$ = $1; 
                                            $$->src = dupStr(__yyline);
                                            $$->lineno = yylineno;
                                        }
	|	COMMENT	'\n'					{	$$ = newLine(';', strItem($1));
											free($1);
											$$->src = dupStr(__yyline);
											$$->lineno = yylineno;
										}
    ;

source_line
    :   TYPE items                      {   $$ = newLine($1, $2); }
    |   TYPE                            {   $$ = newLine($1, NULL); }
    ;
    
items
    :   item                            {   $$ = $1; }
    |   items item                      {   $$ = appendItem($1, $2); }
    ;

item
	: 	inclusive_or_expr				{	$$ = $1; }
	|	NUMBER ':' inclusive_or_expr	{	$$ = newItem(':');
											$$->left = valItem($1);
											$$->right = $3;
										}
    ;
    
primary_expr
    : '(' inclusive_or_expr ')' 		{   $$ = $2; }
    | '~' primary_expr					{	$$ = newItem('~');
                                            $$->left = $2;
										}
    |   SYMBOL                          {   $$ = symItem($1); free($1); }
	|	STRING							{   $$ = strItem($1); free($1); }
    |   NUMBER                          {   $$ = valItem($1);  }
    |   '.'                             {   $$ = newItem('.'); }
    ;
         
multiplicative_expr
    : primary_expr                 		{   $$ = $1; }
    | multiplicative_expr '*' 
      primary_expr                 		{   $$ = newItem('*');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    | multiplicative_expr '/' 
	  primary_expr                 		{   $$ =  newItem('/');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
                                        
    | multiplicative_expr '%' 
      primary_expr                 		{   $$ =  newItem('%');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    ;

additive_expr
    : multiplicative_expr       		{   $$ = $1; }
    | additive_expr '+' 
      multiplicative_expr       		{   $$ = newItem('+');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    | additive_expr '-' 
      multiplicative_expr       		{   $$ = newItem('-');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    ;

shift_expr
    : additive_expr             		{   $$ = $1; }
    | shift_expr LSHIFT 
      additive_expr             		{   $$ = newItem(LSHIFT);
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    | shift_expr RSHIFT 
      additive_expr             		{   $$ = newItem(RSHIFT);
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    ;

and_expr
    : shift_expr             			{   $$ = $1; }
    | and_expr '&' shift_expr  			{   $$ = newItem('&');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    ;

exclusive_or_expr
    : and_expr                  		{   $$ = $1; }
    | exclusive_or_expr '^' and_expr	{   $$ = newItem('^');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    ;

inclusive_or_expr
    : exclusive_or_expr
    | inclusive_or_expr '|' 
      exclusive_or_expr         		{   $$ = newItem('|');
                                            $$->left  = $1;
                                            $$->right = $3; 
                                        }
    ;
    
%%

/************************************************************************/
void yyerror(char *s)
{
    printf("%s\n", s);
    errorCnt++;
}

/************************************************************************/
void my_yyerror (line_t *line, char *s)
{
    if ( line != NULL )
        printf("[%s] #%d: ", line->fname, line->lineno);

    yyerror(s);
}
