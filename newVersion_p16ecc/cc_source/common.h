#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

int yyerror(char*);

typedef struct {
	char *fileName;
	int  lineNum;
	char *srcCode;
} src_t;

// attribution of a node
typedef struct {
    int	 	type;			/* basic type */
    int 	isExtern:   1;	/* extern indication flag */
    int 	isTypedef:  1;	/* typedef indication flag */
	int 	isStatic:   1;	/* static indication flag */
	int 	isUnsigned: 1;	/* unsigned indication flag */
    int 	isVolatile: 1;  /* volatile indication flag */
	int 	isFptr:	    1;	/* function pointer indication flag */
	int		isNeg:		1;	/* negative (constant) indication flag */
    int     dataBank;		/* data storage type,　BANK0, BANK1, ...　, CONST */
    int	    *ptrVect;		/* pointer descrition,　depth and types　*/
    int	    *dimVect;		/* array description, dimensions and sizes　*/
	void	*atAddr;		/* storage address indication (a node pointer) */
	void	*newData;		/* user defined data description (a node pointer) */
	char	*typeName;		/* user defined data name */
	void	*parList;		/* parameter lise for functions (a node pointer) */
	int     bitField;		/* bit-field attributes */
} attrib;

// node types
enum {
    NODE_CON=1,	NODE_STR, NODE_ID, NODE_OPR, NODE_LIST
};

/* constants */
typedef struct {
    int		   	 type;    	/* type: NODE_CON */
	src_t	   	 *src;
    attrib		 *attr;
    long       	 value;
} conNode_t;

/* string */
typedef struct {
    int		     type;    	/* type: NODE_STR */
	src_t		 *src;
    attrib		 *attr;
    char       	 *str;   	/* string value */
} strNode_t;

/* identifiers */
typedef struct {
    int		     type;    	/* type: NODE_ID */
	src_t		 *src;
    attrib		 *attr;
    union node_t *dim;
    union node_t *init;
    char         *name;
    char		 fp_decl:1;	/* function pointer */
    union node_t *parp;		/* parameter list  	   */
} idNode_t;

/* operators */
typedef struct {
    int		     type;		/* type: NODE_OPR */
	src_t		 *src;
	attrib		 *attr;
    int          oper;    	/* operator */
	int 		 nops;		/* counts of operands */
    union node_t *op[1]; 	/* operands (expandable) */
} oprNode_t;

/* list */
typedef struct {
    int		      type;    	/* type: NODE_LIST */
	src_t		  *src;
    int           elipsis; 	/* elipsis option */
	int 		  nops;		/* length of list */
    union node_t *ptr[1];	/* parameters list (expandable) */
} listNode_t;

/* /////////////////////////////////////////////////////////////////////////// */
typedef union node_t {
    int		      type;   	/* type of node */
    conNode_t     con;
    idNode_t      id;
    oprNode_t     opr;
    listNode_t    list;
    strNode_t     str;
} node;

#define LIST_NODE(p,i)	(((node*)p)->list.ptr[i])
#define LIST_LENGTH(p)	((p)? ((node*)p)->list.nops: 0)
#define OPR_NODE(p,i)	(((node*)p)->opr.op[i])
#define OPR_LENGTH(p)	(((node*)p)->opr.nops)
#define OPR_TYPE(p)		(((node*)p)->opr.oper)

/* /////////////////////////////////////////////////////////////////////////// */
extern node *progUnit;
extern char *currentFile;
extern int   yylineno;

char *getSrcCode(void);		/* located in cc.l */

int  _main(char *filename);
void *MALLOC(int size);
char *skipSP(char *s);
char *dupStr(char *s);

// routines for 'attr'
void 	mergeAttr (attrib *a1, attrib *a2);
attrib *newAttr (int atype);
//attrib *newAttrAcce (int atype, acces *acce);
int     assignAcce(attrib *ap, int flag);
void    delAttr (attrib *ap);
attrib *cloneAttr (attrib *ap);
int     nullAttr (attrib *ap);
int		cmpAttr (attrib *ap1, attrib *ap2);
void	copyAttrSpecs (attrib *des, attrib *src);
void    castAttr (attrib *target_ap, attrib *source_ap);

// routines for 'ptrVect'
int   *newPtr (int type);
int   reducePtr (attrib *attr);
void  appendPtr (attrib *attr, int *p);
int  *includePtr(int t, int *p);
void  insertPtr (attrib *aptr, int type);
void  updatePtr (attrib *aptr, int *p);
void  deletePtr(attrib *attr);
int   ptrWeight  (attrib *aptr);
int   equPtr (attrib *ap1, attrib *ap2);
int  *clonePtr(int *p);

// routines for 'node'
node *idNode  (char *s);
node *conNode (int v, int type);
node *listNode(int length);
node *strNode (char *s);
node *lblNode (char *s);
node *oprNode (int type, int cnt, ...);

node *mergeList (node *l1, node *l2);
node *appendList(node *lp, node *np);
node *makeList (node *np);

node *cloneNode(node *np);		// clone a node
void delNode   (node *np);
void moveNode  (node **des, node **src);

int  equListLength(node *np1, node *np2);

void addSrc (node *np, char *s);
void delSrc (node *np);

int  *makeDim(node *np);
int  *cloneDim(int *dim);
void decDim(attrib *attr);
int  dimScale(attrib *attr);
int  dimDepth(attrib *attr);
void clearDim(attrib *attr);

/* --- --- */
typedef struct _NameList {
	char *name;
	struct _NameList *next;
} NameList;

extern NameList *newNameList;
extern NameList *sysIncludeList;

int  addName(NameList **list, char *name);
void delName(NameList **list);
NameList *searchName(NameList *list, char *name);

#define MIN(x,y)	(((x) <= (y))? (x):(y))
#define MAX(x,y)	(((x) >= (y))? (x):(y))

/* --- bit-field services --- */
#define IS_BF_VAR(ap)	(ap && ap->bitField)
#define BF_SIZE(ap)		(ap->bitField & 0xff)
#define BF_OFFSET(ap)	(ap->bitField >> 8)

#ifdef __cplusplus
 }
#endif

#endif