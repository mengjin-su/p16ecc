#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

// attribution of a node
typedef struct {
    int	 	type;			/* type specifier */
} attrib;

// node types
enum {
    NODE_CON=1,	NODE_STR, NODE_ID, NODE_OPR, NODE_LIST
};

/* constants */
typedef struct {
    int		    type;    	/* type: NODE_CON */
    long        value;
} conNode_t;

/* string */
typedef struct {
    int		     type;    	/* type: NODE_STR */
    char         *str;   	/* string value */
} strNode_t;

/* identifiers */
typedef struct {
    int		     type;    	/* type: NODE_ID */
    attrib		 *attr;
    char         *name;
    union node_t *parp;		/* parameter list  	   */
} idNode_t;

/* operators */
typedef struct {
    int		     type;		/* type: NODE_OPR */
	attrib		 *attr;
    int          oper;    	/* operator */
	int 		 nops;		/* counts of operands */
    union node_t *op[1]; 	/* operands (expandable) */
} oprNode_t;

/* list */
typedef struct {
    int		      type;    	/* type: NODE_LIST */
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

extern node *progUnit;

int  _main(char *filename);
void *MALLOC(int size);
char *dupStr(char *s);

attrib *newAttr(int atype);

// routines for 'node'
node *idNode  (char *s);
node *conNode (int v);
node *listNode(int length);
node *strNode (char *s);
node *oprNode (int type, int cnt, ...);

node *mergeList (node *l1, node *l2);
node *appendList(node *lp, node *np);
node *makeList (node *np);

node *cloneNode(node *np);		// clone a node
void delNode   (node *np);

#ifdef __cplusplus
 }
#endif

#endif 