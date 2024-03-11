#ifndef _COMMON_H
#define _COMMON_H

extern int useBSR6;

// item types ...
enum {
	TYPE_VALUE=1,
	TYPE_SYMBOL,
	TYPE_STRING
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

/* /////////////////////////////////////////////////////////////////////////// */
typedef struct line_ {
	int  		  type;		// line type
	item_t		*items;
	char		  *src;		// source file code
	int		    lineno;		// source file line number
	char		*fname;		// source file name
    char        insert;
    char		retry;		// new in v2.1
	char		pclath;
	struct line_ *next;
} line_t;

/* /////////////////////////////////////////////////////////////////////////// */
extern int _yylineno;
extern char *curFile;
extern int  errorCnt;
extern line_t *linePtr;

#define YYLINE_BUF_SIZE   4096
extern char __yyline[YYLINE_BUF_SIZE];

/* /////////////////////////////////////////////////////////////////////////// */
#define yy_error(s)		yyerror(s)
void yyerror(char *s);
void my_yyerror(line_t *line, char *s);

/* /////////////////////////////////////////////////////////////////////////// */
char *MALLOC(int length);
char *dupStr(char *str);

line_t *_main(char *filename);

line_t *newLine(int type, item_t *items);
line_t *endLine(line_t *lp);
void   appendLine(line_t **head, line_t *node);
void   freeLine(line_t *lp);
void   freeLines(line_t *lp);

item_t *newItem(int  type);
item_t *valItem(long val);
item_t *strItem(char *str);
item_t *symItem(char *sym);
void   freeItem(item_t *ip);
void   freeItems(item_t *ip);
item_t *appendItem(item_t *head, item_t *node);
item_t *cloneItem(item_t *iptr);
int    itemCount(item_t *item);
item_t *itemPtr(item_t *item, int index);
int    itemStrCheck(item_t *iptr, char *str);

char *skipSP(char *s);

#endif
