#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>

#include "common.h"
#include "define.h"
#include "asm.h"
//#include "symbol.h"

line_t 	*line_ptr = NULL;
int    	error_cnt = 0;
char	*cur_file;

////////////////////////////////////////////////////////////////////////////////
char *MALLOC(int length)
{
    char *p = (char *)malloc(length);

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
    char *p = MALLOC(strlen(str) + 1);
    strcpy(p, str);
    return p;
}

////////////////////////////////////////////////////////////////////////////////
line_t *newLine(char *label, int inst, item_t *items)
{
    line_t *lp = (line_t *)MALLOC(sizeof(line_t));

	addLabel(lp, label);

    lp->inst  = inst;
    lp->items = items;
    lp->fname = dupStr(cur_file);
    return lp;
}

////////////////////////////////////////////////////////////////////////////////
line_t *endLine(line_t *lp)
{
	while ( lp->next != NULL )
		lp = lp->next;

	return lp;
}

////////////////////////////////////////////////////////////////////////////////
void addLabel(line_t *lp, char *label)
{
	if ( label && lp )
	{
        if ( strstr(label, "::") )
        	lp->global_lbl = 1;

		// remove ending ':' characters
		int len = strlen(label);
        while ( label[--len] == ':' ) label[len] = '\0';

        lp->label = dupStr(label);
	}
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

    switch ( lp->inst )
    {
		case SEGMENT:
            delArea(lp->area);
			break;

		default:
            freeItems(lp->items);
	}

    free(lp);
}

////////////////////////////////////////////////////////////////////////////////
void freeLines(line_t *lp)
{
	while ( lp != NULL )
	{
		line_t *tmp = lp->next;

        freeLine(lp);
		lp = tmp;
	}
}

////////////////////////////////////////////////////////////////////////////////
line_t *newSegLine(char *name, area_t *ap, item_t *item)
{
    line_t *p = (line_t *)MALLOC(sizeof(line_t));

	p->inst = SEGMENT;
	p->area = ap;

	if ( ap == NULL )
        p->area = (area_t *)MALLOC(sizeof(area_t));

	p->area->name = name;
	p->fname = dupStr(cur_file);
	p->mem_item = item;
	return p;
}

////////////////////////////////////////////////////////////////////////////////
void freeItems(item_t *ip)
{
    while ( ip != NULL )
    {
        item_t *tmp = ip;
        ip = ip->next;
        freeItem(tmp);
    }
}


////////////////////////////////////////////////////////////////////////////////
void freeItem(item_t *ip)
{
    if ( ip->type == TYPE_SYMBOL ||
         ip->type == TYPE_STRING )
        free(ip->data.str);

    free(ip);
}

////////////////////////////////////////////////////////////////////////////////
item_t *newItem(int type)
{
    item_t *p = (item_t *)MALLOC(sizeof(item_t));
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

item_t *regItem(int reg_index)
{
    item_t *p   = newItem(TYPE_REG);
    p->data.val = reg_index;
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
	for (ip = lp->items; ip != NULL; ip = ip->next)
        n++;

    return n;
}

////////////////////////////////////////////////////////////////////////////////
item_t *itemPtr(line_t *lp, int index)
{
	item_t *ip = lp->items;
    while ( ip != NULL && index-- ) ip = ip->next;
    return ip;
}

////////////////////////////////////////////////////////////////////////////////
int itemStrCheck(item_t *iptr, char *str)
{
	if ( iptr == NULL )
		return 0;

	switch ( iptr->type )
	{
        case TYPE_SYMBOL:
        	return (strcmp (str, iptr->data.str) == 0);

        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '&':
        case '|':
        case '^':
        case RSHIFT:
        case LSHIFT:
            return (itemStrCheck(iptr->left, str) || itemStrCheck(iptr->left, str));

        case UMINUS:
        case INVERSE:
            return itemStrCheck(iptr->left, str);

		default:
			return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
area_t *newArea(item_t *ip, char assign)
{
	area_t *ap;

	if ( assign == '=' || ip->type == TYPE_SYMBOL )
	{
        ap = (area_t *)MALLOC(sizeof(area_t));

		if ( assign == '=' )
			ap->item = ip;
		else
		{
            if ( strcasecmp(ip->data.str, "ABS") == 0 )
				ap->isABS = 1;
            else if ( strcasecmp(ip->data.str, "OVR") == 0 )
				ap->isOVR = 1;
            else if ( strcasecmp(ip->data.str, "REL") == 0 )
				ap->isREL = 1;
            else if ( strcasecmp(ip->data.str, "BEG") == 0 )
				ap->isBEG = 1;
            else if ( strcasecmp(ip->data.str, "END") == 0 )
				ap->isEND = 1;

            freeItem(ip);
		}

		return ap;
	}

    freeItem(ip);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void delArea(area_t *ap)
{
	if ( ap != NULL )
	{
		free (ap->name);
		freeItems (ap->item);
		free (ap);
	}
}

////////////////////////////////////////////////////////////////////////////////
area_t *mergeArea (area_t *a1, area_t *a2)
{
	if ( a1 == NULL )
		return a2;

	if ( a2 == NULL )
		return a1;

	if ( !a1->isREL )	a1->isREL = a2->isREL;
	if ( !a1->isABS )	a1->isABS = a2->isABS;
	if ( !a1->isOVR )	a1->isOVR = a2->isOVR;
	if ( !a1->isBEG )	a1->isBEG = a2->isBEG;
	if ( !a1->isEND )	a1->isEND = a2->isEND;

	if ( a1->item == NULL )
	{
		a1->item = a2->item;
		a2->item = NULL;
	}

	delArea (a2);
	return a1;
}

////////////////////////////////////////////////////////////////////////////////
int safeStr(char *str)
{
	int i, size = strlen(str);

	if ( str[0] == '(' && str[size-1] == ')' )
		return 1;

	if ( size == 1 && *str == '.' )
		return 1;

	for (i = 0; i < size; i++)
	{
		char c = toupper(str[i]);

		if ( !(isdigit(c) || c == '_' || c == '$' || (c >= 'A' && c <= 'Z')) )
			return 0;
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
char charConst (char *str)
{
	char c = *str++;
	unsigned char v;

	if ( c == '\\' )
	{
		c = *str++;

		switch ( c )
		{
			case 'r':	c = '\r';	break;
			case 'n':	c = '\n';	break;
			case 't':	c = '\t';	break;
			case 'b':	c = '\b';	break;
			case '0':	v = 0;
						while ( *str >= '0' && *str <= '7' )
						{
							c = *str++ - '0';
							v = (v << 3) + c;
						}
						c = v;
		}
	}

	return c;
}

////////////////////////////////////////////////////////////////////////////////
int  stringCount (char *in_str, char *out_str, char zero_end)
{
	int i, size = strlen (in_str);
	int length = 0;

	for (i = 0; *in_str && i < size; i++)
	{
		char c = *in_str++;

		if ( c == '\\' && ++i < size )
		{
			unsigned char v;
			c = *in_str++;

			switch ( c )
			{
				case 'r':	c = '\r';	break;
				case 'n':	c = '\n';	break;
				case 't':	c = '\t';	break;
				case 'b':	c = '\b';	break;
				case '0':	v = 0;
							while ( ++i < size && *in_str >= '0' && *in_str <= '7' )
							{
								c = *in_str++ - '0';
								v = (v << 3) + c;
							}
							c = v;
			}
		}

		length++;

		if ( out_str != NULL )
			*out_str++ = c;
	}

	if ( out_str != NULL )
		*out_str = 0;

	if ( zero_end )
		length++;

	return length;
}


