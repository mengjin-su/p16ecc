#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "asm.h"

char   *curFile = NULL;
line_t *linePtr = NULL;
int    errorCnt = 0;

////////////////////////////////////////////////////////////////////////////////
char *MALLOC(int length)
{
    char *p = (char *)malloc (length);

    if ( p == NULL )
    {
        printf("run out memory!\n");
        exit(99);
    }

    memset(p, 0, length);
    return p;
}


////////////////////////////////////////////////////////////////////////////////
char *dupStr(char *str)
{
    char *p = MALLOC (strlen (str) + 1);
    strcpy (p, str);
    return p;
}

////////////////////////////////////////////////////////////////////////////////
line_t *newLine(char *label, int inst, item_t *items)
{
    line_t *p = (line_t *)MALLOC(sizeof(line_t));

    if ( label != NULL )
    {
		int len = strlen(label);
		int col_count = 0, i = len;
		while ( label[i-1] == ':' ) { i--; col_count++; }
		p->globalLbl = (col_count >= 2)? 1: 0;

		label[i] = '\0';
        p->label = dupStr(label);
	}

    p->inst  = inst;
    p->oprds = items;
    p->fname = dupStr(curFile);// (cur_file->name);
    return p;
}

////////////////////////////////////////////////////////////////////////////////
line_t *endLine(line_t *lp)
{
	while ( lp->next != NULL )
		lp = lp->next;

	return lp;
}

////////////////////////////////////////////////////////////////////////////////
void appendLine(line_t **head, line_t *node)
{
    line_t *tmp = *head;

    if ( tmp == NULL )
    {
        *head = node;
        return;
	}

    while ( tmp->next != NULL )
        tmp = tmp->next;

    tmp->next = node;
}

////////////////////////////////////////////////////////////////////////////////
void freeLine(line_t *lp)
{
	if ( lp == NULL )
		return;

    free(lp->label);
    free(lp->src);
    free(lp->fname);

    if ( lp->inst == SEGMENT )
		delAttr(lp->attr);

	freeItems(lp->oprds);
    free (lp);
}

////////////////////////////////////////////////////////////////////////////////
void freeLines(line_t *lp)
{
	while ( lp != NULL )
	{
		line_t *tmp = lp->next;

		freeLine (lp);
		lp = tmp;
	}
}

////////////////////////////////////////////////////////////////////////////////
line_t *newSegLine(char *name, attr_t *ap, item_t *item)
{
	line_t *p = (line_t *)MALLOC(sizeof (line_t));

	p->inst = SEGMENT;
	p->attr = ap;
	p->oprds= item;
	p->attr->name = name;
	p->fname = dupStr(curFile);
	return p;
}

////////////////////////////////////////////////////////////////////////////////
void freeItems(item_t *ip)
{
    while ( ip != NULL )
    {
        item_t *tmp = ip;
        ip = ip->next;
        freeItem (tmp);
    }
}


////////////////////////////////////////////////////////////////////////////////
void freeItem(item_t *ip)
{
    if ( ip->type == TYPE_SYMBOL || ip->type == TYPE_STRING )
        free(ip->data.str);

    free(ip);
}

////////////////////////////////////////////////////////////////////////////////
item_t *newItem(int type)
{
    item_t *p = (item_t *)MALLOC(sizeof (item_t));
    p->type   = type;
    return p;
}

////////////////////////////////////////////////////////////////////////////////
item_t *valItem(long val)
{
    item_t *p   = newItem(TYPE_VALUE);
    p->data.val = val;
    return p;
}

////////////////////////////////////////////////////////////////////////////////
item_t *strItem(char *str)
{
    item_t *p   = newItem(TYPE_STRING);
    p->data.str = dupStr(str);
    return p;
}

////////////////////////////////////////////////////////////////////////////////
item_t *symItem(char *sym)
{
    item_t *p   = newItem(TYPE_SYMBOL);
    p->data.str = dupStr(sym);
    return p;
}

////////////////////////////////////////////////////////////////////////////////
item_t *appendItem(item_t *head, item_t *node)
{
    item_t *tmp = head;

    if ( tmp == NULL )
        return node;

    while ( tmp->next != NULL )
        tmp = tmp->next;

    tmp->next = node;
    return head;
}

////////////////////////////////////////////////////////////////////////////////
item_t *cloneItem(item_t *iptr)
{
	item_t *ip = NULL;

	if ( iptr == NULL )
		return NULL;

	ip = newItem(iptr->type);
	ip->left  = cloneItem(iptr->left);
	ip->right = cloneItem(iptr->right);

	switch ( iptr->type )
	{
		case TYPE_VALUE:
			ip->data.val = iptr->data.val;
			break;

		case TYPE_STRING:
		case TYPE_SYMBOL:
			ip->data.str = dupStr(iptr->data.str);
			break;
	}
	return ip;
}

////////////////////////////////////////////////////////////////////////////////
int itemCount(line_t *lp)
{
    int n = 0;
	item_t *ip;
    for (ip = lp->oprds; ip != NULL; ip = ip->next) n++;
    return n;
}

////////////////////////////////////////////////////////////////////////////////
attr_t *newAttr(item_t *ip, char assign)
{
	if ( assign == '=' || ip->type == TYPE_SYMBOL )
	{
		attr_t *ap = (attr_t *)MALLOC(sizeof (attr_t));

		if ( assign == '=' )
			ap->addr = ip;		// ABS segment address
		else
		{
            if ( strcmp(ip->data.str, "ABS") == 0 )
				ap->isABS = 1;
            else if ( strcmp(ip->data.str, "OVR") == 0 )
				ap->isOVR = 1;
            else if ( strcmp(ip->data.str, "REL") == 0 )
				ap->isREL = 1;
            else if ( strcmp(ip->data.str, "BEG") == 0 )
				ap->isBEG = 1;
            else if ( strcmp(ip->data.str, "END") == 0 )
				ap->isEND = 1;

			freeItem(ip);
		}

		return ap;
	}

	freeItem(ip);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void delAttr(attr_t *ap)
{
	if ( ap != NULL )
	{
		free(ap->name);
		freeItems(ap->addr);
		free(ap);
	}
}

////////////////////////////////////////////////////////////////////////////////
attr_t *mergeAttr(attr_t *a1, attr_t *a2)
{
	if ( a1 == NULL )
		return a2;

	if ( a2 == NULL )
		return a1;

	if ( !a1->isREL ) a1->isREL = a2->isREL;
	if ( !a1->isABS ) a1->isABS = a2->isABS;
	if ( !a1->isOVR ) a1->isOVR = a2->isOVR;
	if ( !a1->isBEG ) a1->isBEG = a2->isBEG;
	if ( !a1->isEND ) a1->isEND = a2->isEND;

	if ( a1->addr == NULL )
	{
		a1->addr = a2->addr;
		a2->addr = NULL;
	}

	delAttr(a2);
	return a1;
}

////////////////////////////////////////////////////////////////////////////////
int memcasecmp(char *s1, char *s2, int size)
{
	while ( size-- )
	{
		char c1 = toupper(*s1++);
		char c2 = toupper(*s2++);
		if ( c1 != c2 )	return c1 - c2;
	}
	return 0;
}