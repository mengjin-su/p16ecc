#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "dlink.h"

Dlink :: Dlink(int type)
{
	dtype = type;
	dlist = NULL;
	parent = NULL;
	child = NULL;
	next  = NULL;
//	owner = NULL;
}

Dlink :: ~Dlink()
{
	while ( dlist ) {
		Dnode *dp = dlist->next;
		delete dlist;
		dlist = dp;
	}
	if ( child ) delete child;
	if ( next )  delete next;
}

int Dlink :: dataCount(void)
{
	int n = 0;
	for (Dnode *dp = dlist; dp; dp = dp->next) n++;
	return n;
}

void Dlink :: add(Dnode *dp)
{
	if ( dp )
	{
		Dnode *dnp = dlist;
		if ( dnp == NULL )
			dlist = dp;
		else
		{
			while ( dnp->next ) dnp = dnp->next;
			dnp->next = dp;
		}
	}
}

Dnode *Dlink :: add(node *np, attrib *attr, int index)
{
	Dnode *dp   = new Dnode(np->id.name, attr, index);
	dp->parp    = cloneNode(np->id.parp);
	dp->elipsis = np->id.parp? np->id.parp->list.elipsis: 0;
	add(dp);
	return dp;
}

void Dlink :: addChild(Dlink *lk)
{
	lk->parent = this;

	if ( child == NULL )
		child = lk;
	else
	{
		Dlink *dlk = child;
		while ( dlk->next ) dlk = dlk->next;
		dlk->next = lk;
	}
}

Dnode *Dlink :: search(char *name, int search_mode)
{
	Dnode *dp = search(name);	// search in local list.

	if ( dp != NULL )		return dp;
	if ( parent == NULL )	return NULL;

	if ( search_mode == LOCAL_SEARCH )
	{
		if ( dtype == PAR_DLINK || parent->dtype != PAR_DLINK )
			return NULL;
	}

	return parent->search(name, search_mode);
}

Dnode *Dlink :: search(char *name)
{
	for (Dnode *list = dlist; list; list = list->next)
	{
		if ( strcmp(name, list->name) == 0 )
			return list;
	}
	return NULL;
}

Dnode *Dlink :: get(int index)
{
	for (Dnode *dp = dlist; dp; dp = dp->next, index--)
		if ( index == 0 ) return dp;

	return NULL;
}

bool Dlink :: nameCheck(char *name, int end_index)
{
	for (int i = 0; i < dataCount() && i < end_index; i++)
		if ( strcmp(name, get(i)->name) == 0 )
			return false;

	return true;
}

void Dlink :: display(int spaces)
{
#define PRINT_SP(s) for(int i = 0; i++ < (s);) printf(" ")

	PRINT_SP(spaces); printf("DLIST:\n");
	for (Dnode *dp = dlist; dp; dp = dp->next)
		dp->display(spaces+1);

	if ( next )
		next->display(spaces);

	if ( child ) {
		PRINT_SP(spaces); printf("CHILD:\n");
		child->display(spaces+2);
	}
}

/************************************************************************/
void delDlinks(Dlink *dlink)
{
	if ( dlink )
	{
		while ( dlink->dlist ) {	// delete Dnodes
			Dnode *next = dlink->dlist->next;
			delete dlink->dlist;
			dlink->dlist = next;
		}
		delDlinks(dlink->next);		// delete siblings
		delDlinks(dlink->child);	// delete children
		delete dlink;				// delete itself
	}
}
