#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "dlink.h"
#include "sizer.h"

Dnode :: Dnode(char *_name, attrib *_attr, int _index)
{
	memset(this, 0, sizeof(Dnode));
	name  = dupStr(_name);
	attr  = cloneAttr(_attr);
	index = _index;
    atAddr= -1;
}

Dnode :: ~Dnode()
{
	if ( name ) free(name);
	delAttr(attr);
	delNode(parp);
}

char *Dnode :: nameStr(void)
{
	if ( func ) // function owner
	{
		static char buf[1024];
		if ( index > 0 )
			sprintf(buf, "%s_$%d_%s", func, index, name);
		else
			sprintf(buf, "%s_$_%s", func, name);
		return buf;
	}
	else
		return name;
}

int Dnode :: size(void)
{
	return sizer.size(attr, TOTAL_SIZE);
}

void Dnode :: nameUpdate(char *_name)
{
	if ( name ) free(name);
	name = dupStr(_name);
}

void Dnode :: dimUpdate(node* np)
{
	if ( attr->dimVect ) free(attr->dimVect);
	attr->dimVect = makeDim(np);
}

bool Dnode :: dimCheck(node *np)
{
	if ( attr->dimVect == NULL && np == NULL ) return true;
	if ( attr->dimVect != NULL && np == NULL ) return false;
	if ( attr->dimVect == NULL && np != NULL ) return false;
	if ( attr->dimVect[0] != np->list.nops )   return false;
	return true;
}

bool Dnode :: fptrCheck(node *np)
{
	if ( attr->isFptr != np->id.fp_decl ) return false;

	np = np->id.parp;
	if ( parp == NULL && np == NULL ) return true;
	if ( parp == NULL || np == NULL ) return false;
	if ( LIST_LENGTH(parp) != LIST_LENGTH(np) ) return false;
	if ( elipsis != np->list.elipsis ) return false;

	for (int i = 0; i < LIST_LENGTH(parp); i++)
	{
		node *p1 = LIST_NODE(parp, i);
		node *p2 = LIST_NODE(np,   i);
		if ( cmpAttr(p1->id.attr, p2->id.attr)  )
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
Dlink :: Dlink(int type)
{
	dtype = type;
	dlist = NULL;
	parent = NULL;
	child = NULL;
	next  = NULL;
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
