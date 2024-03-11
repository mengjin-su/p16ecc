#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "item.h"
#include "pnode.h"

////////////////////////////////////////////////////////////////
Pnode :: Pnode(int _type)
{
	memset(this, 0, sizeof(Pnode));
	type = _type;
}

////////////////////////////////////////////////////////////////
Pnode :: Pnode(int _type, Item *ip0)
{
	memset(this, 0, sizeof(Pnode));
	type = _type;
	items[0] = ip0;
}

Pnode :: ~Pnode()
{
	for (int i = 0; i < 3; i++)
		if ( items[i] ) delete items[i];

	if ( last ) last->next = next;
	if ( next )	next->last = last;
}

void Pnode :: updateItem(int index, Item *ip)
{
	if ( items[index] ) delete items[index];
	items[index] = ip;
}

void Pnode :: updateName(int index, char *str)
{
	if ( items[index] )
	{
		free(items[index]->val.s);
		items[index]->val.s = dupStr(str);
	}
}

void Pnode :: insert(Pnode *pp)
{
	if ( pp )
	{
		Pnode *end = pp->end();
		pp->last = last;
		end->next = this;

		if ( last )	last->next = pp;
		last = end;
	}
}

Pnode *Pnode :: end(void)
{
	Pnode *p = this;
	while ( p->next != NULL )
		p = p->next;
	return p;
}

/////////////////////////////////////////////////////////////
void addPnode(Pnode **list, Pnode *pnp)
{
	if ( pnp )
	{
		Pnode *lst = *list;
		pnp->last = NULL;
		pnp->next = NULL;
		if ( lst )
		{
			Pnode *end = lst->end();
			pnp->last = end;
			end->next = pnp;
		}
		else
			*list = pnp;
	}
}

void delPnodes(Pnode **list)
{
	Pnode *lst = *list;
	*list = NULL;
	while ( lst )
	{
		Pnode *next = lst->next;
		lst->last = NULL;
		lst->next = NULL;
		delete lst;
		lst = next;
	}
}
