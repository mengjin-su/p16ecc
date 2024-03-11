#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"

node *progUnit;

void *MALLOC(int size)
{
	char *s = malloc(size);
	if ( s == NULL ) exit(-1);	// out of memory space!
	memset(s, 0, size);
	return (void*)s;
}

char *dupStr(char *s)
{
	char *p = NULL;
	if ( s )
	{
		p = (char*)MALLOC(strlen(s)+1);
		strcpy(p, s);
	}
	return p;
}

// routines for 'attrib'
attrib *newAttr(int atype)
{
	attrib *a = MALLOC(sizeof(attrib));
	a->type = atype;
	return a;
}

static node *newNode(int type)
{
    node *np = (node*)MALLOC(sizeof(node));
	np->type = type;
    return np;
}

// routines for 'node'
node *idNode(char *s)
{
	node *np = newNode(NODE_ID);
	np->id.name = dupStr(s);
	return np;
}

node *conNode (int v)
{
	node *np = newNode(NODE_CON);
	np->con.value = v;
	return np;
}

node *listNode(int length)
{
	int size = sizeof(listNode_t);
    if ( length > 1 )
    	size += (length - 1) * sizeof(void*);

    node *np = (node*)MALLOC(size);
    np->type      = NODE_LIST;
    np->list.nops = length;
    return np;
}

node *strNode(char *s)
{
    node *np = newNode(NODE_STR);
    np->str.str = dupStr(s);
    return np;
}

node *oprNode(int type, int cnt, ...)
{
    int i, size = sizeof(oprNode_t);

    if ( cnt > 1 )
        size += sizeof(void*) * (cnt-1);

    node *np = (node *)MALLOC(size);
    np->type = NODE_OPR;

    np->opr.oper = type;
    np->opr.nops = cnt;

    va_list vp;
    va_start(vp, cnt);
	// put the nodes into operator list...
    for (i = 0; i < cnt; i++)
        np->opr.op[i] = va_arg(vp, node*);

	va_end(vp);
    return np;
}

node *mergeList(node *l1, node *l2)
{
	int  size;

	if ( l1 == NULL ) return l2;
	if ( l2 == NULL ) return l1;

	if ( l1->type != NODE_LIST || l2->type != NODE_LIST )
	{
		yyerror("'mergeList': illegal list node(s)!");
		exit (99);
	}

	// new length of the list
	node *np = listNode (l1->list.nops + l2->list.nops);

	for (size = 0; size < l1->list.nops; size++)
		np->list.ptr[size] = l1->list.ptr[size];

	for (size = 0; size < l2->list.nops; size++)
		np->list.ptr[l1->list.nops + size] = l2->list.ptr[size];

	// release old lists (without deleting their nodes)
	l1->list.nops = 0;	delNode(l1);
	l2->list.nops = 0;	delNode(l2);
	return np;
}

node *appendList(node *lp, node *np)
{
	int i;

	if ( lp == NULL ) return makeList(np);
	if ( np == NULL ) return lp;

	node *p = listNode(lp->list.nops + 1);

	for (i = 0; i < lp->list.nops; i++)
	{
		p->list.ptr[i]  = lp->list.ptr[i];
		lp->list.ptr[i] = NULL;
	}
	p->list.ptr[i] = np;
	lp->list.nops = 0;
	delNode(lp);
	return p;
}

node *makeList(node *np)
{
	node *p = listNode(1);
	p->list.ptr[0] = np;
	return p;
}

void delNode(node *np)
{
    int i;

    if ( np == NULL ) return;

    switch ( np->type )
    {
        case NODE_ID:
            free(np->id.name);
            break;

        case NODE_STR:
            free(np->str.str);
            break;

        case NODE_OPR:
            for (i = 0; i < np->opr.nops; i++)
                delNode(np->opr.op[i]);
            break;

        case NODE_LIST:
            for (i = 0; i < np->list.nops; i++)
                delNode(np->list.ptr[i]);
            break;

        case NODE_CON:
            break;
    }

    free (np);

}
