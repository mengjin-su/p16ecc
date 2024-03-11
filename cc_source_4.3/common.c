#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"
#include "cc.h"

node *progUnit;

void *MALLOC(int size)
{
	char *s = malloc(size);
	if ( s == NULL ) exit(-1);	// out of memory space!
	memset(s, 0, size);
	return (void*)s;
}

char *skipSP(char *s)
{
	while ( s && (*s == ' ' || *s == '\t') ) s++;
	return s;
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

void delAttr(attrib *ap)
{
	if ( ap != NULL )
	{
		delNode(ap->atAddr);
		delNode(ap->newData);
		delNode(ap->parList);
		if ( ap->typeName ) free(ap->typeName);
		if ( ap->dimVect )	free(ap->dimVect);
		if ( ap->ptrVect )	free(ap->ptrVect);
		free (ap);
	}
}

void mergeAttr(attrib *a1, attrib *a2)
{
	if ( a1 && a2 )
	{
		if ( a2->isStatic ) a1->isStatic = 1;
		if ( a2->isExtern ) a1->isExtern = 1;
		if ( a2->isTypedef ) a1->isTypedef = 1;
		if ( a2->isUnsigned ) a1->isUnsigned = 1;
		if ( a2->isVolatile ) a1->isVolatile = 1;
	}
}

int assignAcce(attrib *ap, int flag)
{
	if ( flag == LINEAR || flag == CONST || flag == EEPROM )
	{
		if ( ap->dataBank && ap->dataBank != flag ) return 1;
		ap->dataBank = flag;
	}
	else
		switch ( flag )
		{
			case EXTERN: 	ap->isExtern = 1;	break;
			case TYPEDEF:	ap->isTypedef = 1;	break;
			case VOLATILE: 	ap->isVolatile = 1;	break;
			case STATIC: 	ap->isStatic = 1;	break;
		}
	return 0;
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

node *conNode(int v, int type)
{
    node *np = newNode(NODE_CON);
    np->con.attr = newAttr(type);
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
	node *np = listNode(l1->list.nops + l2->list.nops);

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

	// free source message
	if ( np->id.src != NULL )
		delSrc(np);

	// free attrib
	if ( np->type != NODE_LIST )
       	delAttr(np->id.attr);
	
    free (np);
}

src_t *srcNode(char *s)
{
	if ( s != NULL )
	{
		src_t *sp = (src_t*)MALLOC(sizeof(src_t));
		sp->fileName = dupStr(currentFile);
		sp->lineNum  = yylineno;
		sp->srcCode  = dupStr(skipSP(s));
		return sp;
	}
	return NULL;
}

void addSrc(node *np, char *s)
{
	np->id.src = srcNode(s);
}

void delSrc(node *np)
{
	if ( np->id.src != NULL )
	{
		free(np->id.src->fileName);
		free(np->id.src->srcCode);
		free(np->id.src);
		np->id.src = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// routines for 'ptrVect'
int *newPtr(int type)
{
	int *p = MALLOC (sizeof(int)*2);
	p[0] = 1; p[1] = type;
	return p;
}

int reducePtr(attrib *attr)
{
	if ( attr && attr->ptrVect )
	{
		int t = attr->ptrVect[1];
		if ( attr->ptrVect[0] > 1 )
		{
			int *v = MALLOC(attr->ptrVect[0]*sizeof(int));
			v[0] = attr->ptrVect[0] - 1;
			memcpy(&v[1], &attr->ptrVect[2], v[0]*sizeof(int));
			free(attr->ptrVect);
			attr->ptrVect = v;
		}
		else
		{
			free(attr->ptrVect);
			attr->ptrVect = NULL;
		}
		return t;
	}
	return 0;
}

void appendPtr(attrib *attr, int *p)
{
	if ( attr && p )
	{
		if ( attr->ptrVect == NULL )
		{
			attr->ptrVect = p;
			return;
		}
		int l1 = attr->ptrVect[0];
		int l2 = p[0];
		int *ptr = MALLOC(sizeof(int)*(l1 + l2 + 1));
		ptr[0] = l1 + l2;
		memcpy(&ptr[1],    &attr->ptrVect[1], sizeof(int)*l1);
		memcpy(&ptr[1+l1], &p[1], 			  sizeof(int)*l2);
		free(attr->ptrVect);
		free(p);
		attr->ptrVect = ptr;
	}
}

int *includePtr(int t, int *p)
{
	int *ptr = MALLOC(sizeof(int)*(p[0] + 2));
	ptr[0] = p[0] + 1;
	ptr[1] = t;
	memcpy(ptr+2, p+1, sizeof(int)*p[0]);
	free(p);
	return ptr;
}

void insertPtr(attrib *aptr, int type)
{
	if ( aptr )
	{
		if ( aptr->ptrVect == NULL )
		{
			aptr->ptrVect = newPtr(type);
			return;
		}
		int l1 = aptr->ptrVect[0];
		int *ptr = MALLOC(sizeof(int)*(l1 + 1 + 1));
		ptr[0] = l1 + 1;
		ptr[1] = type;
		memcpy(&ptr[1+1], &aptr->ptrVect[1], sizeof(int)*l1);
		free(aptr->ptrVect);
		aptr->ptrVect = ptr;
	}
}

void updatePtr(attrib *aptr, int *p)
{
	if ( aptr )
	{
		if ( aptr->ptrVect ) free(aptr->ptrVect);
		aptr->ptrVect = p;
	}
}

void deletePtr(attrib *attr)
{
	if ( attr && attr->ptrVect )
	{
		free(attr->ptrVect);
		attr->ptrVect = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////
NameList *searchName(NameList *list, char *name)
{
	for(; list; list = list->next)
		if ( strcmp(list->name, name) == 0 )
			return list;

	return NULL;
}

void delName(NameList **list)
{
	NameList *lst = *list;
	*list = NULL;
	while ( lst )
	{
		NameList *next = lst->next;
		if ( lst->name ) free(lst->name);
		free(lst); lst = next;
	}
}

int addName(NameList **list, char *name)
{
	if ( name && searchName(*list, name) == NULL )
	{
		NameList *dp = MALLOC(sizeof(NameList));
		dp->next = *list;
		dp->name = dupStr(name);
		*list = dp;
		return 0;
	}
	return -1;
}
