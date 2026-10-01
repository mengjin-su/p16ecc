%{

#include <string.h>
#include <ctype.h>
#include <string.h>
#include "define.h"
#include "common.h"
#include "asm.h"

int yyparse();
#ifdef __dj_include_ctype_h_
int yylineno;	// DGJPP GCC ...
#endif

#define YYLINE_BUF_SIZE   4096
char __yyline[YYLINE_BUF_SIZE];
char *__yytext;


/* --- data structures --- */
typedef struct _f {
    char     *name;     /* filename */
    void     *last;
    FILE     *fptr;
    int       lnum;
    struct _f *next;
} file;

/* --- local functions --- */
file *f_stack = NULL;
//file *current_file= NULL;
file *f_list = NULL;
char *current_file;

file *searchStrName(file *head, char *name);
file *addStrName(file *head, char *name);

void    append_str (void);
int     convert_num(char *str);
Inst_t *searchInst (char *str);

%}

sp                          [ \t\015]

symbol                      [_a-zA-Z][_a-zA-Z0-9$]*
hex                         (0[xX][a-fA-F0-9]+)
dec                         ([0-9]|([1-9][0-9]+))

s_char                      ([^"\r\n])
s_char_sequence             ({s_char}+)
string_constant             \L?\"{s_char_sequence}?\"

%x OPCODE
%x OPERAND 

%%

^{sp}*";".*                 {   append_str(); 
                                if ( memcmp(yytext, "; :: ", 5) == 0 )
                                {
                                    yylval.name = dupStr(yytext);
                                    return S_COMMENT;
                                }
                            }
^{sp}*{symbol}:(:)?         {   int i = 0;
								append_str();
								while ( yytext[i] == ' ' || yytext[i] == '\t' ) i++;
                                BEGIN(OPCODE);
                                yylval.name = dupStr(&yytext[i]);
                                return LABEL;
							}
^{sp}+                      {   append_str();
                                BEGIN(OPCODE); }
                                
\n                          {   append_str();
                                return yytext[0];
                            }
                            
<OPCODE>(".")?{symbol}      {   Inst_t *ip;

                                append_str ();
                                BEGIN (OPERAND);
                        
                                ip = searchInst((char *)yytext);

                                if ( ip != NULL )
                                {   
                                    yylval.value = ip->token;
                                    if ( ip->token == SEGMENT )
										return ip->token;

                                    return ST8_INST;
                                }
                                
                                printf("Line #%d: ", yylineno-1);
                                printf("illegal opcode - %s\n", yytext);
                            }

<OPCODE>\n                  {   append_str();
                                BEGIN(INITIAL);
                                return yytext[0]; 
                            }
                            
<OPCODE>";".*               |
<OPCODE>{sp}+               {   append_str(); }
                            
<OPERAND>"%A"				{   append_str(); yylval.value = REG_A;  return REG; }
<OPERAND>"%X"				{   append_str(); yylval.value = REG_X;  return REG; }
<OPERAND>"%Y"				{   append_str(); yylval.value = REG_Y;  return REG; }
<OPERAND>"%SP"				{   append_str(); yylval.value = REG_SP; return REG; }
<OPERAND>"%XH"				{   append_str(); yylval.value = REG_XH; return REG; }
<OPERAND>"%XL"				{   append_str(); yylval.value = REG_XL; return REG; }
<OPERAND>"%YH"				{   append_str(); yylval.value = REG_YH; return REG; }
<OPERAND>"%YL"				{   append_str(); yylval.value = REG_YL; return REG; }
<OPERAND>"%CC"				{   append_str(); yylval.value = REG_CC; return REG; }

<OPERAND>{symbol}           {   append_str();
                                yylval.name = dupStr(yytext);
                                return SYMBOL;  
                            }

<OPERAND>{string_constant}  {   append_str();
                                yytext[yyleng-1] = '\0';
                                yylval.name = dupStr(&yytext[1]);
                                return STRING; 
                            }
                            
<OPERAND>{hex}        		|
<OPERAND>{dec}              {   append_str();
                                yylval.value = convert_num((char *)yytext);
                                return NUMBER;
                            }
<OPERAND>">>"               {   append_str(); return RSHIFT; }
<OPERAND>"<<"               {   append_str(); return LSHIFT; }

<OPERAND>\n                 {   append_str();
                                BEGIN(INITIAL);
                                return yytext[0]; 
                            }

<OPERAND>[-+$%^&|=,()/~]    {   append_str();
                                return yytext[0]; 
                            }

<OPERAND>"*"                {   append_str();
                                return yytext[0]; 
                            }

<OPERAND>";".*              {   append_str(); }
<OPERAND>{sp}+              {   append_str(); }

<OPERAND>.                  {   append_str();
                                return yytext[0];
                            }

.                           {   append_str();
                                yyerror("unknown character!");
                            }

%%

/* ---------------------------------------------------------------------------- */
int STR_MATCHn(char *s1, char *s2, int len)
{
    while ( len-- )
    {
        if ( toupper(*s1) != toupper(*s2) )
            return 0;

        s1++; s2++;
    }
    return 1;
}

/* ---------------------------------------------------------------------------- */
int yywrap(void) 
{ 
    fclose(yyin);
    return -1;
}

static char last_ch = '\n';
/* ---------------------------------------------------------------------------- */
void append_str(void)
{
    int l = strlen(yytext);
    
//  printf("%s", yytext);
    
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
void clr_line_buf(void)
{
    __yyline[0] = '\0';
}

/* ---------------------------------------------------------------------------- */
int convert_num(char *str)
{
    int n = 0, i = 0, radix = 10;
    int l = strlen (str);

    if ( toupper(str[l-1]) == 'H'                   ||
         (str[i] == '0' && toupper(str[i+1]) == 'X') )
    {
        i += (toupper (str[l-1]) == 'H')? 0: 2;
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
const Inst_t InstTbl[] = {
    {"add",		ADD},
    {"addw",	ADDW},
    {"adc", 	ADC},
    {"and",		AND},
	{"bccm", 	BCCM},
	{"bcp",		BCP},
	{"bcpl",	BCPL},
	{"bres",	BRES},
	{"bset",	BSET},
	{"btjf",	BTJF},
	{"btjt",	BTJT},
	{"call",	CALL},
	{"callf",	CALLF},
	{"callr",	CALLR},
	{"ccf",		CCF},
	{"clr",		CLR},
	{"clrw",	CLRW},
	{"cp",		CP},
	{"cpw",		CPW},
	{"cpl",		CPL},
	{"cplw",	CPLW},
	{"dec",		DEC},
	{"decw",	DECW},
	{"div",		DIV},
	{"divw",	DIVW},
	{"exg",		EXG},
	{"exgw",	EXGW},
	{"halt",	HALT},
	{"inc",		INC},
	{"incw",	INCW},
	{"iret",	IRET},
	{"jp",		JP},
	{"jpf",		JPF},
	{"jra",		JRA},
	{"jrc",		JRC},
	{"jreq",	JREQ},
	{"jrf",		JRF},
	{"jrh",		JRH},
	{"jrih",	JRIH},
	{"jril",	JRIL},
	{"jrm",		JRM},
	{"jrmi",	JRMI},
	{"jrnc",	JRNC},
	{"jrne",	JRNE},
	{"jrnh",	JRNH},
	{"jrnm",	JRNM},
	{"jrnv",	JRNV},
	{"jrpl",	JRPL},
	{"jrsge",	JRSGE},
	{"jrsgt",	JRSGT},
	{"jrsle",	JRSLE},
	{"jrslt",	JRSLT},
	{"jrt",		JRT},
	{"jruge",	JRUGE},
	{"jrugt",	JRUGT},
	{"jrule",	JRULE},
	{"jrult",	JRULT},
	{"jrv",		JRV},
	{"ld",		LD},
	{"ldf",		LDF},
	{"ldw",		LDW},
	{"mov",		MOV},
	{"mul",		MUL},
	{"neg",		NEG},
	{"negw",	NEGW},
	{"nop",		NOP},
	{"or",		OR},
	{"pop",		POP},
	{"popw",	POPW},
	{"push",	PUSH},
	{"pushw",	PUSHW},
	{"rcf",		RCF},
	{"ret",		RET},
	{"retf",	RETF},
	{"rim",		RIM},
	{"rlc",		RLC},
	{"rlcw",	RLCW},
	{"rlwa",	RLWA},
	{"rrc",		RRC},
	{"rrcw",	RRCW},
	{"rrwa",	RRWA},
	{"rvf",		RVF},
	{"sbc",		SBC},
	{"scf",		SCF},
	{"sim",		SIM},
	{"sla",		SLL},	// SLA is equivalent to SLL
	{"slaw",	SLLW},
	{"sll",		SLL},
	{"sllw",	SLLW},
	{"sra",		SRA},
	{"sraw",	SRAW},
	{"srl",		SRL},
	{"srlw",	SRLW},
	{"sub",		SUB},
	{"subw",	SUBW},
	{"swap",	SWAP},
	{"swapw",	SWAPW},
	{"tnz",		TNZ},
	{"tnzw",	TNZW},
	{"trap",	TRAP},
	{"wfe",		WFE},
	{"wfi",		WFI},
	{"xor",		XOR},

    {".equ",     EQU},
    {".segment", SEGMENT},
    {".rs",      RS},
	{".db",		 DB},
    {".dw",		 DW},
	{".dd",		 DD},
	{".dt",		 DT},
    {".end",     END},
    {".stm8",    STM8},
    {".device",  DEVICE},
    {".invoke",  INVOKE},
	{".cblank",	 CBLANK},
	{".dblank",	 DBLANK},
	{".vect",	 VECT},
	
	{".jp",		 _JP},	// not determined instructions...
	{".jbz",	 _JBZ},
	{".jbnz",	 _JBNZ},
	
    {NULL,      0}
};

/* ---------------------------------------------------------------------------- */
Inst_t *searchInst(char *str)
{
    int i = 0;

    while ( InstTbl[i].name != NULL )
    {
        if ( strcasecmp(str, InstTbl[i].name) == 0 )
            return (Inst_t *)&InstTbl[i];

        i++;
    }
    return NULL;
}

/************************************************************/
line_t *_main(char *filename)
{
    int yyparse_ret;

    // if the file has been parsed, skip it
    if ( searchStrName(f_list, filename) != NULL )
        return NULL;

    f_list = addStrName(f_list, filename);


    yyin = fopen(filename, "r");

    if ( yyin == NULL )
    {
        printf("can't open file - %s!", filename);
        exit(99);
    }

    current_file = filename;

    yylineno = 0;
    line_ptr = NULL;
   
    yyparse_ret = yyparse();   	// parse the input text (file)
    
    if ( yyparse_ret != 0 )		// if fail to parse, stop!
    {
        printf("yyparse stopped at #Line %d\n", yylineno);
        exit(99);
    }

    return line_ptr;
}

///////////////////////////////////////////////////////////////
file *searchStrName(file *head, char *name)
{
    while ( head != NULL )
    {
        #ifdef LINUX
        if ( strcmp(head->name, name) == 0 )
        #else
        if ( strcasecmp(head->name, name) == 0 )
        #endif
        {
            return head;
        }

        head = head->next;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////
file *addStrName(file *head, char *name)
{
    file *p = searchStrName(head, name);

    if ( p == NULL )
    {
        p = (file *)MALLOC(sizeof(file));
        p->name = dupStr(name);
        p->next = head;
    }

    return p;
}
