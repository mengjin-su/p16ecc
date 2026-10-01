#ifndef _COMMON_H
#define _COMMON_H

#define DEFAULT_OUTPUT	(char*)"_OUTPUT_"

enum {
	TYPE_VALUE=128,
	TYPE_STRING,
	TYPE_SYMBOL,
	TYPE_REG,
	TYPE_INDIR,
	TYPE_INDEX,
	TYPE_INDIR_INDEX
};


/* instruction descriptor*/
typedef struct {
    char *name;
    int  token;
} Inst_t;

/* /////////////////////////////////////////////////////////////////////////// */
typedef union data_ {
    int   val;
	char *str;
} data_t;

/* /////////////////////////////////////////////////////////////////////////// */

typedef struct item_ {
	int		      type;
	data_t 	  	  data;
	char		 bank0;
	struct item_ *left;
	struct item_ *right;
	struct item_ *next;
} item_t;

typedef struct seg_item_ {
	char		*name;
	item_t		*item;
	short  	  isABS: 1;
	short	  isREL: 1;
	short	  isOVR: 1;
	short	  isBEG: 1;
	short	  isEND: 1;
} area_t;

/* /////////////////////////////////////////////////////////////////////////// */
typedef struct line_ {
	char 		*label;		// label field
	int		  seg_type;
    int  global_lbl: 1;     //
	int   		  inst;		// instruction code field
	union {
		item_t	*items;		// operand field(s)
		area_t   *area;
	};
	char		  *src;		// source file code
	int		    lineno;		// source file line number
	char		*fname;		// source file name
	item_t		*mem_item;
	struct line_ *next;
} line_t;

/* /////////////////////////////////////////////////////////////////////////// */
typedef struct hex_ {
	int 		size;		// buffer size (int word)
	int			 max;		// max address that filled
	long 		*buf;		// buffer pointer
	long		fill;		// filler value
} hex_t;


/* /////////////////////////////////////////////////////////////////////////// */
typedef struct seg_ {
	char       	*filename;
	line_t	   	*start;
	line_t	   	*end;
	int		   	length;
	struct seg_	*next;
} seg_t;

/* /////////////////////////////////////////////////////////////////////////// */
extern int _yylineno;
extern char *cur_file;
extern int  scan_pass;

/* /////////////////////////////////////////////////////////////////////////// */
#define yy_error(s)	yyerror(s)
void yyerror (const char *s);
void my_yyerror (line_t *line, const char *s);

/* /////////////////////////////////////////////////////////////////////////// */
char *MALLOC (int length);
char *dupStr (char *str);

line_t *_main (char *filename);

line_t *newLine (char *label, int inst, item_t *item);
line_t *endLine (line_t *lp);
void addLabel(line_t *lp, char *label);
void appendLine (line_t **head, line_t *node);
void freeLine (line_t *lp);
void freeLines (line_t *lp);
line_t *newSegLine (char *seg_name, area_t *areap, item_t *item);

item_t *newItem (int  type);
item_t *valItem (long val);
item_t *strItem (char *str);
item_t *symItem (char *sym);
item_t *regItem (int reg_index);

void   freeItem (item_t *ip);
void   freeItems (item_t *ip);
item_t *appendItem (item_t *head, item_t *node);
item_t *cloneItem (item_t *iptr);
int    itemCount(line_t *lp);
item_t *itemPtr(line_t *lp, int index);
int    itemStrCheck (item_t *iptr, char *str);

#define itemMem 	itemPtr

void appendSeg (seg_t **head, seg_t *seg);
void deleteSeg (seg_t *sp);

#define LTYPE(l)	(l->inst)
#define ITYPE(i)	(i->type)

void delArea (area_t *ap);
area_t *newArea (item_t *ip, char assign);
area_t *mergeArea (area_t *a1, area_t *a2);

char charConst (char *str);
int  stringCount (char *in_str, char *out_str, char zero_end);
int  safeStr (char *str);

char STRCMP (char *s1, char *s2);

#endif
