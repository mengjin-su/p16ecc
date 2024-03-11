/*-----------------------------------------------------------------------

   cc.y - parser definition file:

-------------------------------------------------------------------------*/
%{
#include <stdio.h>
#include <stdarg.h> 
#include <string.h>
#include "common.h"

extern int  yyerror (char *);
extern int  yylex (), yylineno;
extern FILE *yyin;
extern int  ignoreTypedef;

int yyparse(void);

#define ADD_SRC(n)	addSrc(n, getSrcCode())
#define CLR_SRC()	getSrcCode()
static int parListCheck(node *np);

#define YYDEBUG 1
%}

%expect 1

%union {
	char   *s;  // string value
	int     i;  // integer value
	node   *n;
	attrib *a;
	int    *p;	
}

%token CHAR INT SHORT LONG UCHAR UINT USHORT ULONG VOID 
%token FUNC_HEAD FUNC_DECL DATA_DECL
%token _IF _ELSE _ENDIF _IFDEF _IFNDEF DEFINE UNDEF PRAGMA
%token SBIT BREAK CASE CONST CONTINUE DEFAULT DO ELSE ENUM EXTERN
%token FOR GOTO IF EEPROM REGISTER RETURN SIZEOF STATIC SWITCH 
%token VOLATILE WHILE INTERRUPT UNION STRUCT TYPEDEF ELIPSIS 
%token LINEAR
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token EQ_OP NE_OP LE_OP GE_OP LEFT_OP RIGHT_OP INC_OP DEC_OP
%token OR_OP AND_OP PTR_OP FPTR CAST CALL 
%token POST_INC POST_DEC PRE_INC PRE_DEC POS_OF ADDR_OF NEG_OF
%token EOL LABEL

%token <i> CONSTANT L_CONSTANT U_CONSTANT UL_CONSTANT C_CONSTANT MEM_BANK
%token <i> RELATIONAL_OP EQUALITY_OP SHIFT_OP ASSIGN_OP
%token <s> IDENTIFIER IDENTIFIER_ STRING AASM
%token <a> TYPEDEF_NAME


%type <n> program external_definitions external_definition
%type <n> identifier 
%type <n> declaration init_declarator_list init_declarator
%type <n> declarator direct_declarator
%type <a> type_specifier type_specifier2 declaration_specifiers
%type <i> storage_class_specifier type_qualifier pointer_acce
%type <p> pointer
%type <n> expr constant_expr conditional_expr logical_or_expr
%type <n> logical_and_expr inclusive_or_expr exclusive_or_expr and_expr
%type <n> equality_expr relational_expr shift_expr additive_expr
%type <n> multiplicative_expr cast_expr unary_expr primary_expr postfix_expr
%type <i> unary_operator assignment_operator
%type <n> argument_expr_list assignment_expr 
%type <n> parameter_list parameter_type_list parameter_declaration
%type <n> initializer initializer_list
%type <n> func_definition func_declarator func_declarator2 func_body
%type <n> compound_statement statement_list general_statement statement
%type <n> expression_statement labeled_statement selection_statement
%type <n> else_statement case_statement_list case_statement case_action
%type <n> jump_statement iteration_statement opt_expr 
%type <n> case_op_action while_expr
%type <n> if_condition opt_external_definitions op_else id_list 
%type <n> op_if_statement op_else_statement
%type <a> enum_specifier
%type <n> enumerator_list opt_enumerator_expr enumerator
%type <a> struct_or_union_specifier
%type <n> struct_declaration_list
%type <n> struct_declaration struct_declarator_list 
%type <i> struct_or_union
%type <s> opt_identifier 

%start program

%%

program
	: external_definitions		{ progUnit = $1; }
	;
external_definitions
	: external_definition		{ $$ = makeList($1); }
	| external_definitions
	  external_definition		{ $$ = appendList($1, $2); }
	;
external_definition
	: declaration				{ $$ = $1; CLR_SRC(); }
	| func_definition			{ $$ = $1; CLR_SRC(); }
	| declaration_specifiers 
	  init_declarator_list 
	  '@' conditional_expr ';'	{ $$ = oprNode(DATA_DECL, 1, $2); 
	  							  $$->opr.attr = $1;
								  $1->atAddr = $4;	
								  ADD_SRC($$);
	  							}
    | SBIT identifier 
	  '@' conditional_expr ';'  { $$ = oprNode(SBIT, 2, $2, $4); 
								  ADD_SRC($$);
								}
    | UNDEF identifier EOL      { $$ = oprNode(UNDEF, 1, $2); 
								  ADD_SRC($$);}
    | DEFINE identifier EOL     { $$ = oprNode(DEFINE, 1, $2); 
								  ADD_SRC($$); }
    | DEFINE identifier 
      expr EOL  				{ $$ = oprNode(DEFINE, 2, $2, $3);
								  ADD_SRC($$); }
	| DEFINE IDENTIFIER_
      id_list ')' expr EOL 		{ $$ = oprNode(DEFINE, 3, idNode($2), $3, $5);
      							  ADD_SRC($$); free($2); 
                                }								  
    | _IFDEF identifier
      opt_external_definitions  
      op_else
      _ENDIF                    { $$ = oprNode(_IFDEF, 3, $2, $3, $4); 
								  ADD_SRC($$); }
    | _IFNDEF identifier
      opt_external_definitions
      op_else
      _ENDIF                    { $$ = oprNode(_IFNDEF, 3, $2, $3, $4); 
								  ADD_SRC($$); }
    | if_condition
      opt_external_definitions
      op_else
      _ENDIF                    { $$ = oprNode(_IF, 3, $1, $2, $3); 
								  ADD_SRC($$); }
    | PRAGMA identifier EOL     { $$ = oprNode(PRAGMA, 1, $2); 
								  ADD_SRC($$);  }
    | PRAGMA identifier 
      conditional_expr  EOL     { $$ = oprNode(PRAGMA, 2, $2, $3); 
								  ADD_SRC($$); }	
	;
declaration
	: declaration_specifiers ';' {if ( $1->type == ENUM ) {
									$$ = $1->newData;
									$1->newData = NULL;
									delAttr($1);
									ADD_SRC($$);
								  }
								  else 
								  {
								    $$ = oprNode(DATA_DECL, 0); 
	  							    $$->opr.attr = $1;
									ADD_SRC($$);
								  }
  								  ignoreTypedef &= ~1;
	  							}
	| declaration_specifiers 
	  init_declarator_list ';'	{ $$ = oprNode(DATA_DECL, 1, $2); 
								  $$->opr.attr = $1;
								  ADD_SRC($$);
   								  ignoreTypedef &= ~1;
	  							}
    | AASM                      { $$ = oprNode(AASM, 1, strNode($1)); 
								  ADD_SRC($$); }
	| TYPEDEF					{ ignoreTypedef &= ~1; }
	  type_specifier 			{ ignoreTypedef |=  1; }
	  init_declarator_list ';'	{ $3->isTypedef = 1;
								  if ( $5 && $5->list.nops == 1 ) 
								  {
									char *id = $5->list.ptr[0]->id.name;
									$$ = oprNode(TYPEDEF, 1, idNode(id));
									$$->opr.attr = $3;
									addName(&newNameList, id);
									ADD_SRC($$);
								  }  
								  else
								  {
									$$ = NULL;
									delAttr($3);
									yyerror("incorrect 'typedef' format!");
									CLR_SRC();
								  }
								  ignoreTypedef &= ~1;
								  delNode($5);
								} 								  
	;
opt_external_definitions
    : external_definitions      { $$ = $1; }
    |                           { $$ = NULL; }
	;
if_condition
	: _IF conditional_expr EOL	{ $$ = $2; ADD_SRC($$); }
	;
op_else
    :  _ELSE
      opt_external_definitions  { $$ = $2; }
    |                           { $$ = NULL; }
	;	
id_list
    :                           { $$ = NULL; }
    | identifier                { $$ = makeList ($1); }
    | id_list ',' identifier    { $$ = appendList($1, $3); }	  							
	;
declaration_specifiers
	: storage_class_specifier	{ $$ = newAttr(0); 
								  if ( assignAcce($$, $1) ) 
									yyerror("storage type redefined!");
								}
	| storage_class_specifier
	  declaration_specifiers	{ $$ = $2; 
								  if ( assignAcce($$, $1) )
									yyerror("storage type redefined!");
								}
	| type_specifier
	  declaration_specifiers	{ $$ = $1; mergeAttr($$, $2); delAttr($2); }
	| type_specifier			{ $$ = $1; }
	| type_qualifier
	  declaration_specifiers	{ $$ = $2; assignAcce($$, $1); }
	;
storage_class_specifier
	: REGISTER					{ $$ = REGISTER; }
	| STATIC					{ $$ = STATIC; }
	| EEPROM					{ $$ = EEPROM; }
	| EXTERN					{ $$ = EXTERN; }
	| MEM_BANK					{ $$ = $1; }
	;
type_qualifier
	: CONST						{ $$ = CONST; }
	| VOLATILE					{ $$ = VOLATILE; }
	;
type_specifier
	: VOID						{ $$ = newAttr(VOID); }
	| CHAR						{ $$ = newAttr(CHAR); }
	| SHORT						{ $$ = newAttr(SHORT); }
	| INT						{ $$ = newAttr(INT); }
	| LONG						{ $$ = newAttr(LONG); }
	| UCHAR						{ $$ = newAttr(CHAR);  $$->isUnsigned = 1; }
	| USHORT					{ $$ = newAttr(SHORT); $$->isUnsigned = 1; }
	| UINT						{ $$ = newAttr(INT);   $$->isUnsigned = 1; }
	| ULONG						{ $$ = newAttr(LONG);  $$->isUnsigned = 1; }
	| enum_specifier			{ $$ = $1; }
	| struct_or_union_specifier { $$ = $1; }
	| TYPEDEF_NAME				{ $$ = $1; }	
	;
type_specifier2
	: declaration_specifiers	{ $$ = $1; }
	| declaration_specifiers 
	  pointer					{ $$ = $1; appendPtr($$, $2); 
								  if ( $1->type == 0 )
									yyerror ("pointer type not defined!");
								}
	;
init_declarator_list
	: init_declarator			{ $$ = makeList($1); }
	| init_declarator_list ','
	  init_declarator			{ $$ = appendList($1, $3); }
	;
init_declarator
	: declarator				{ $$ = $1; }
	| declarator '='
	  initializer				{ $$ = $1; $$->id.init = $3; }
	;
declarator
	: direct_declarator			{ $$ = $1; }
	| pointer direct_declarator { $$ = $2; 
								  if ( $2->id.attr == NULL ) 
									$2->id.attr = newAttr(0);
								  $2->id.attr->ptrVect = $1; 
								}
	;
direct_declarator
	: identifier				{ $$ = $1; }
	| direct_declarator '[' ']' { $$ = $1; 
								  $$->id.dim = appendList($$->id.dim, conNode(0, INT)); 
								}
	| direct_declarator 
	  '[' constant_expr ']'		{ $$ = $1; 
								  $$->id.dim = appendList($$->id.dim, $3); 
								}
 	| '(' '*' identifier ')' '(' 
 	  parameter_type_list ')' 	{ int n = parListCheck($6);
								  $$ = $3;
								  $$->id.fp_decl = 1;
								  if ( n > 0 )
									$$->id.parp = $6;
								  else
								  {
									delNode($6);
									if ( n < 0 )
									  yyerror("parameter list error!");
								  }
          						}
 	| '(' '*' identifier
 	  '[' constant_expr ']' ')' '(' 
 	  parameter_type_list ')' 	{ int n = parListCheck($9);
								  $$ = $3;
								  $$->id.dim = makeList($5);
								  $$->id.fp_decl = 1;
								  if ( n > 0 )
									$$->id.parp = $9;
								  else
								  {
									delNode($9);
									if ( n < 0 )
									  yyerror("parameter list error!");
								  }
          						}          						
	|  '(' '*' ')' '(' 
      parameter_type_list ')' 	{ int n = parListCheck($5);
								  $$ = idNode("");
           						  $$->id.fp_decl = 1;
								  if ( n > 0 )
									$$->id.parp = $5;
								  else
								  {
								    delNode($5);
									if ( n < 0 )
									  yyerror("parameter list error!");	
								  }
          						}								
	;
identifier
	: IDENTIFIER				{ $$ = idNode($1); free($1); }
	;
pointer_acce
	: CONST						{ $$ = CONST; }
	| MEM_BANK					{ $$ = $1; }
	;
pointer
	: '*'						{ $$ = newPtr(0); }
	| '*' pointer				{ $$ = includePtr(0, $2); }
	| '*' pointer_acce			{ $$ = newPtr($2); }
	| '*' pointer_acce pointer	{ $$ = includePtr($2, $3); }
	;
constant_expr
	: conditional_expr			{ $$ = $1; }
	;
conditional_expr
	: logical_or_expr			{ $$ = $1; }
	| logical_or_expr '?' expr 
	  ':' conditional_expr		{ $$ = oprNode('?', 3, $1, $3, $5); }
	;
logical_or_expr
	: logical_and_expr			{ $$ = $1; }
	| logical_or_expr OR_OP 
	  logical_and_expr			{ $$ = oprNode(OR_OP, 2, $1, $3); }
	;
logical_and_expr
	: inclusive_or_expr			{ $$ = $1; }
	| logical_and_expr AND_OP 
	  inclusive_or_expr			{ $$ = oprNode(AND_OP, 2, $1, $3); }
	;
inclusive_or_expr
	: exclusive_or_expr			{ $$ = $1; }
	| inclusive_or_expr '|' 
	  exclusive_or_expr			{ $$ = oprNode('|', 2, $1, $3); }
	; 
exclusive_or_expr
	: and_expr					{ $$ = $1; }
	| exclusive_or_expr '^' 
	  and_expr					{ $$ = oprNode('^', 2, $1, $3); }
	;
and_expr
	: equality_expr				{ $$ = $1; }
	| and_expr '&' 
	  equality_expr				{ $$ = oprNode('&', 2, $1, $3); }
	;
equality_expr
	: relational_expr			{ $$ = $1; }
	| equality_expr EQUALITY_OP 
	  relational_expr			{ $$ = oprNode($2, 2, $1, $3); }
	;
relational_expr 
	: shift_expr				{ $$ = $1; }
	| relational_expr RELATIONAL_OP 
	  shift_expr				{ $$ = oprNode($2, 2, $1, $3); }
	;
shift_expr
	: additive_expr
	| shift_expr SHIFT_OP 
	  additive_expr				{ $$ = oprNode($2, 2, $1, $3); }
	;
additive_expr
	: multiplicative_expr		{ $$ = $1; }
	| additive_expr '+' 	
	  multiplicative_expr		{ $$ = oprNode('+', 2, $1, $3); }
	| additive_expr '-' 
	  multiplicative_expr		{ $$ = oprNode('-', 2, $1, $3); }
	;
multiplicative_expr       
	: cast_expr					{ $$ = $1; }
	| multiplicative_expr '*' 
	  cast_expr					{ $$ = oprNode('*', 2, $1, $3); }
	| multiplicative_expr '/' 
	  cast_expr					{ $$ = oprNode('/', 2, $1, $3); }
	| multiplicative_expr '%' 
	  cast_expr					{ $$ = oprNode('%', 2, $1, $3); }
	;
cast_expr
	: unary_expr				{ $$ = $1; }
	| '(' type_specifier2 ')' 
	  cast_expr					{ if ( $2->type == VOID &&
									   $2->ptrVect == NULL )
									yyerror("invalid cast type - 'void'!");
								  $$ = oprNode(CAST, 1, $4); 
								  $$->opr.attr = $2; }
	;
unary_expr
	: postfix_expr				{ $$ = $1; }
	| INC_OP unary_expr			{ $$ = oprNode(PRE_INC, 1, $2); }
	| DEC_OP unary_expr			{ $$ = oprNode(PRE_DEC, 1, $2); }
	| unary_operator cast_expr	{ $$ = $1? oprNode($1, 1, $2): $2; }
	| SIZEOF '(' unary_expr	')'	{ $$ = oprNode(SIZEOF, 1, $3); }
	| SIZEOF 
	  '(' type_specifier2 ')'	{ $$ = oprNode(SIZEOF, 0); $$->opr.attr = $3; }
	;
unary_operator
	: '&'						{ $$ = ADDR_OF; }
	| '*'						{ $$ = POS_OF; }
	| '+'						{ $$ = 0; }
	| '-'						{ $$ = NEG_OF; }
	| '~'						{ $$ = '~'; }
	| '!'						{ $$ = '!'; }
	;
postfix_expr
	: primary_expr				{ $$ = $1; }
	| postfix_expr 
	  '[' expr ']'				{ $$ = oprNode('[', 2, $1, $3); }
	| postfix_expr '(' 
	  argument_expr_list ')'	{ $$ = oprNode(CALL, 2, $1, $3); }
	| postfix_expr '(' ')'		{ $$ = oprNode(CALL, 2, $1, NULL); }
	| postfix_expr '.' 
	  identifier				{ $$ = oprNode('.', 2, $1, $3); }
	| postfix_expr PTR_OP 
	  identifier				{ $$ = oprNode(PTR_OP, 2, $1, $3); }
	| postfix_expr INC_OP		{ $$ = oprNode(POST_INC, 1, $1); }
	| postfix_expr DEC_OP		{ $$ = oprNode(POST_DEC, 1, $1); }
	;
primary_expr
	: identifier				{ $$ = $1; }
	| CONSTANT					{ $$ = conNode($1, INT); }
	| C_CONSTANT				{ $$ = conNode($1, CHAR); }
	| U_CONSTANT				{ $$ = conNode($1, INT); $$->con.attr->isUnsigned = 1; }
	| L_CONSTANT				{ $$ = conNode($1, LONG); }
	| UL_CONSTANT				{ $$ = conNode($1, LONG); $$->con.attr->isUnsigned = 1; }
	| STRING					{ $$ = strNode($1); free($1); }
	| '(' expr ')'				{ $$ = $2; }
	;
expr
	: expr ',' assignment_expr	{ $$ = oprNode(',', 2, $1, $3); }
	| assignment_expr			{ $$ = $1; }
	;
assignment_operator
	: '='						{ $$ = '='; }
	| ASSIGN_OP					{ $$ = $1; }
	;
assignment_expr
	: conditional_expr			{ $$ = $1; }
	| unary_expr assignment_operator 
	  assignment_expr			{ $$ = oprNode($2, 2, $1, $3); }
	;
argument_expr_list
	: assignment_expr			{ $$ = makeList($1); }
	| argument_expr_list ',' 
	  assignment_expr			{ $$ = appendList($1, $3); }
	;
parameter_type_list
	:							{ $$ = NULL; }
	| parameter_list			{ $$ = $1; }
	| parameter_list ',' ELIPSIS{ $$ = $1; $$->list.elipsis = 1; }
	;
parameter_list
	: parameter_declaration		{ $$ = makeList($1); }
	| parameter_list ','
	  parameter_declaration		{ $$ = appendList($1, $3); }
	;
parameter_declaration
	: type_specifier 
	  direct_declarator 		{ $1->isFptr  = $2->id.fp_decl;
								  $1->parList = $2->id.parp;
								  $2->id.parp = NULL;	  
								  $$ = $2;
								  $$->id.attr = $1;
								}
	| type_specifier 
	  pointer direct_declarator { $1->isFptr  = $3->id.fp_decl;
								  $1->parList = $3->id.parp;
								  $3->id.parp = NULL;
							  	  $1->ptrVect = $2;
								  $$ = $3;
								  $$->id.attr = $1;
								}								
	| type_specifier 			{ $$ = idNode("");
								  $$->id.attr = $1;
								}
	| type_specifier pointer	{ $$ = idNode("");
								  $1->ptrVect = $2;	
								  $$->id.attr = $1;
								}
	;
initializer
	: assignment_expr			{ $$ = $1; }
	| '{' initializer_list '}'	{ $$ = $2; }
	| '{' 
	   initializer_list ',' '}' { $$ = $2; }
	;
initializer_list
	: initializer				{ $$ = makeList($1); }
	| initializer_list
	  ',' initializer			{ $$ = appendList($1, $3); }
	;
func_definition
	: func_declarator
	  func_body					{ $$ = oprNode(FUNC_DECL, 2, $1, $2); 
								  $$->opr.attr = newAttr(INT);
								}
	| declaration_specifiers
	  func_declarator2			{ if ( $1->type == 0 )
								  {
									if ( $2->id.attr && $2->id.attr->ptrVect )
										yyerror("function pointer type not defined!");
									else
										$1->type = INT;
								  }
								}
	  func_body					{ $$ = oprNode(FUNC_DECL, 2, $2, $4); 
								  $$->opr.attr = $1;

								  if ( $2->id.attr )
								  {
									$$->opr.attr->ptrVect = $2->id.attr->ptrVect;
									$2->id.attr->ptrVect = NULL;
									delAttr($2->id.attr);
									$2->id.attr = NULL;
								  }
								}
	| INTERRUPT 
	  identifier '(' ')' 
	  compound_statement		{ node *np = oprNode(FUNC_HEAD, 1, $2);
	  							  $$ = oprNode(FUNC_DECL, 2, np, $5); 
  								  $$->opr.attr = newAttr(INTERRUPT);
  								}
	;
func_declarator
	: identifier '('
	  parameter_type_list ')'	{ int n = parListCheck($3);
								  if ( n > 0 )
								  {
									$$ = oprNode(FUNC_HEAD, 2, $1, $3);
									ADD_SRC($$); 
								  }	
								  else
								  {
								    delNode($3);
								    if ( n == 0 )
									{
									  $$ = oprNode(FUNC_HEAD, 1, $1); 
									  ADD_SRC($$); 
									}  
									else  
									{
									  yyerror("parameter error!");
									  CLR_SRC();
									}  
								  }
								}
	;
func_declarator2	/* may include 'pointer' */
	: func_declarator 			{ $$ = $1; }
	| pointer func_declarator   { $$ = $2; 
								  $2->id.attr = newAttr(0);
								  $2->id.attr->ptrVect = $1;
								  ADD_SRC($$); 
								}
	;
func_body
	: ';'						{ $$ = NULL; }
	| compound_statement		{ $$ = $1;   }
	;
compound_statement
	: '{' '}'					{ $$ = oprNode('{', 0);     CLR_SRC(); }
	| '{' statement_list '}'	{ $$ = oprNode('{', 1, $2); CLR_SRC(); }
	;
statement_list
	: general_statement			{ $$ = makeList($1); 
								  $$ = appendList($$, oprNode(';', 0)); }
	| statement_list
	  general_statement			{ $$ = appendList($1, $2); 
								  $$ = appendList($$, oprNode(';', 0)); }
	;	
general_statement
	: statement					{ $$ = $1; CLR_SRC(); }
	| declaration				{ $$ = $1; CLR_SRC(); }
    ;
statement
	: compound_statement		{ $$ = $1; }
	| expression_statement		{ $$ = $1; }
	| labeled_statement			{ $$ = $1; }
	| selection_statement		{ $$ = $1; }
	| jump_statement			{ $$ = $1; }
	| iteration_statement		{ $$ = $1; }
    | _IFDEF identifier
      op_if_statement
	  op_else_statement
      _ENDIF                    { $$ = oprNode(_IFDEF, 3, $2, $3, $4); }
    | _IFNDEF identifier
      op_if_statement
	  op_else_statement
      _ENDIF                    { $$ = oprNode(_IFNDEF, 3, $2, $3, $4); }
    | if_condition
      op_if_statement
	  op_else_statement
      _ENDIF                    { $$ = oprNode(_IF, 3, $1, $2, $3); }
    | DEFINE identifier EOL     { $$ = oprNode(DEFINE, 1, $2); }
    | DEFINE identifier
      conditional_expr EOL   	{ $$ = oprNode(DEFINE, 2, $2, $3); }
	| DEFINE IDENTIFIER_
      id_list ')' expr EOL 		{ $$ = oprNode(DEFINE, 3, idNode($2), $3, $5);
      							  free($2); ADD_SRC($$); 
                                }
	;
op_if_statement
	:							{ $$ = NULL; }
	| statement_list			{ $$ = $1;   }
	;
op_else_statement
	:							{ $$ = NULL; }
	| _ELSE statement_list		{ $$ = $2;   }
	;
expression_statement
	: ';'						{ $$ = oprNode(';', 0); ADD_SRC($$); }
	| expr ';'					{ $$ = $1; ADD_SRC($$); }
	;
labeled_statement
	: identifier ':'			{ $$ = oprNode(LABEL, 1, $1); ADD_SRC($$); }
	;
selection_statement
	: IF '(' expr ')'			{ ADD_SRC($3); }
      statement
      else_statement            { $$ = oprNode(IF, 3, $3, $6, $7); }
    | SWITCH '(' expr ')' '{'   { ADD_SRC($3); }
      case_statement_list '}'	{ $$ = oprNode (SWITCH, 2, $3, $7); }
	;
else_statement
	:							{ $$ = NULL; }
	| ELSE statement			{ $$ = $2; }
	;
case_statement_list			
	: case_statement			{ $$ = makeList($1); }
	| case_statement_list 
	  case_statement			{ $$ = appendList($1, $2); }
	;
case_statement
	: CASE constant_expr ':' 	{ ADD_SRC($2); }
	  case_op_action			{ $$ = oprNode(CASE, 2, $2, $5); }  
	| DEFAULT ':' 				{ CLR_SRC(); }
	  case_action				{ $$ = oprNode(DEFAULT, 1, $4); }
	;
case_action 
	: statement					{ $$ = makeList($1); }
	| case_action
	  statement					{ $$ = appendList($1, $2); }
	;
case_op_action
	:							{ $$ = NULL; }
	| case_action 				{ $$ = $1; }
	;
jump_statement
    : GOTO identifier ';'       { $$ = oprNode(GOTO, 1, $2); ADD_SRC($$); }
    | CONTINUE ';'              { $$ = oprNode(CONTINUE, 0); ADD_SRC($$); }
    | BREAK ';'                 { $$ = oprNode(BREAK, 0);    ADD_SRC($$); }
    | RETURN ';'                { $$ = oprNode(RETURN, 0);   ADD_SRC($$); }
    | RETURN expr ';'           { $$ = oprNode(RETURN, 1, $2); ADD_SRC($$); }
    ;
while_expr
	: WHILE '(' expr ')'		{ $$ = $3; ADD_SRC($$); }
	;
iteration_statement
	: while_expr statement		{ $$ = oprNode(WHILE, 2, $1, $2); }
	| DO 						{ CLR_SRC(); }
	  statement while_expr ';'	{ $$ = oprNode(DO, 2, $3, $4); }
    | FOR '(' opt_expr ';'
			  opt_expr ';'
			  opt_expr ')'      { CLR_SRC(); }
      statement                 { $$ = oprNode(FOR, 4, $3, $5, $7, $10); }
  	;
opt_expr
	: 							{ $$ = NULL; }
	| expr						{ $$ = $1; ADD_SRC($$); }
	;
enum_specifier
    : ENUM 
      '{' enumerator_list '}'   { $$ = newAttr(ENUM);
                                  $$->newData = oprNode(ENUM, 1, $3);
                                }
    | ENUM 
      '{' 
      enumerator_list ',' '}'	{ $$ = newAttr(ENUM);
                                  $$->newData = oprNode(ENUM, 1, $3);
                                }                               
	;	
enumerator_list
    : enumerator                { $$ = makeList($1); }
    | enumerator_list ',' 
      enumerator                { $$ = appendList($1, $3); }
    ;
enumerator
    : identifier 
      opt_enumerator_expr       { $$ = $1;
                                  $$->id.init = $2;
                                }
    ;
opt_enumerator_expr
	: '=' constant_expr			{ $$ = $2; }
	|							{ $$ = NULL; }
	;	
struct_or_union_specifier	
    : struct_or_union 
      opt_identifier			{ ignoreTypedef <<= 1; }
      '{' 
      struct_declaration_list
      '}'                       { $$ = newAttr($1);
								  $$->typeName = $2;
								  $$->newData  = $5;
								  { ignoreTypedef >>= 1; }
                                }
    | struct_or_union
      IDENTIFIER				{ $$ = newAttr($1);
								  $$->typeName = $2;
								}
	;
struct_or_union
	: STRUCT					{ $$ = STRUCT; }
	| UNION						{ $$ = UNION; }
	;
opt_identifier
	:							{ $$ = NULL; }
	| IDENTIFIER				{ $$ = $1; }
	;
struct_declaration_list
    : struct_declaration        { $$ = makeList($1); }
    | struct_declaration_list 
      struct_declaration        { $$ = appendList($1, $2); }
    ;
struct_declaration
	: type_specifier
	  struct_declarator_list ';' { $$ = oprNode(DATA_DECL, 1, $2);
								   $$->opr.attr = $1;	
								}
	;
struct_declarator_list
	: declarator				{ $$ = makeList($1); }
	| struct_declarator_list ','
	  declarator				{ $$ = appendList($1, $3); }
	;	
%%

static int parListCheck(node *np)
{
	int i, n = LIST_LENGTH(np);
	for (i = 0; i < n; i++)
	{
		node *nnp = LIST_NODE(np, i);
		attrib *attr = nnp->id.attr;
		
        if ( attr->type == VOID && !attr->ptrVect && !attr->isFptr )
			return (n == 1 && !strlen(nnp->id.name))? 0: -1;
	}
	return n;
}
