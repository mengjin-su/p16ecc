#ifndef _COMMON_H
#define _COMMON_H

extern int useBsr6;
extern int libFile;

enum {
	TYPE_VALUE=1,
	TYPE_STRING,
	TYPE_SYMBOL
};

/* /////////////////////////////////////////////////////////////////////////// */
typedef union data_ {
    int   val;
	char *str;
} data_t;

/* /////////////////////////////////////////////////////////////////////////// */
typedef struct item_ {
	int		      type;
	data_t 	  	  data;
	struct item_ *left;
	struct item_ *right;
	struct item_ *next;
} item_t;

typedef struct {
	char		*name;
	item_t		*addr;
	int			isABS: 1;
	int			isREL: 1;
	int			isBEG: 1;
	int		 	isEND: 1;
} attr_t;

/* /////////////////////////////////////////////////////////////////////////// */
typedef struct line_ {
	char 		*label;		// label field
	int		   segType;		// segment type
    int   	 globalLbl;     //
	int   		  inst;		// instruction code field
	attr_t   	 *attr;		// segment descriptor
	item_t		*oprds;		// operand field(s)
	int		   desRegF;		// destination F
	char		  *src;		// source file code
	int		    lineno;		// source file line number
	char		*fname;		// source file name
	struct line_ *next;
} line_t;

/* /////////////////////////////////////////////////////////////////////////// */
extern int _yylineno;
extern char *curFile;
extern line_t *linePtr;
extern int  errorCnt;
extern char __yyline[];

#define LTYPE(l)	(l->inst)	// line type
#define ITYPE(i)	(i->type)	// item type

/* /////////////////////////////////////////////////////////////////////////// */
void yyerror(char *s);
void my_yyerror(line_t *line, char *s);

/* /////////////////////////////////////////////////////////////////////////// */
char *MALLOC(int length);
char *dupStr(char *str);

line_t *_main(char *filename);

line_t *newLine(char *label, int inst, item_t *item);
line_t *newSegLine(char *name, attr_t *attr, item_t *item);
void   appendLine(line_t **head, line_t *node);
void   freeLine(line_t *lp);

void    delAttr(attr_t *ap);
attr_t *newAttr(item_t *ip, char assign);
attr_t *mergeAttr(attr_t *a1, attr_t *a2);

item_t *newItem(int  type);
item_t *valItem(long val);
item_t *symItem(char *sym);
item_t *strItem(char *str);
void   freeItem(item_t *ip);
void   freeItems(item_t *ip);
item_t *appendItem(item_t *head, item_t *node);
int    itemCount(line_t *lp);
item_t *cloneItem(item_t *ip);

int  memcasecmp(char *s1, char *s2, int size);

#endif
