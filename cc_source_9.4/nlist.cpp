#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "common.h"
#include "nlist.h"
extern "C" {
#include "cc.h"
}

///////////////////////////////////////////////////////////////
Nnode :: Nnode(int _type, char *_name, node *np1, node *np2)
{
	memset(this, 0, sizeof(Nnode));
	type  = _type;
	name  = dupStr(_name);
	np[0] = np1;
	np[1] = np2;
}

Nnode :: ~Nnode()
{
	if ( name ) free(name);
	delAttr(attr);
	delNode(np[0]);
	delNode(np[1]);	
}

int Nnode :: nops(void)
{
	if ( np[0] == NULL ) return 0;
	if ( np[1] == NULL ) return 1;
	return 2;
}

///////////////////////////////////////////////////////////////
Nlist :: Nlist()
{
	list = NULL;
	addLayer();
}

Nlist :: ~Nlist()
{
	while ( list ) {
		Nnode *next = list->next;
		delete list;
		list = next;
	}
}

void Nlist :: addLayer(void)
{
	Nnode *p  = new Nnode(0);
	p->next   = list;
	p->parent = list;
	list      = p;
}

void Nlist :: delLayer(void)
{
	Nnode *parent = list->parent;
	if ( parent == NULL ) return;
	
    while ( list )
	{
        Nnode *next = list->next;

        delete list;
        if ( next == parent ) break;	// end of chain, done.
		list = next;
	}
	list = parent;
}

Nnode *Nlist :: search(char *name, int type)
{
	for(Nnode *p = list; p; p = p->next)
	{
		if ( p->name && strcmp(p->name, name) == 0 )
		{
			if ( type <= 0 ) return p;
			if ( type == p->type ) return p;
		}			
	}
	return NULL;
}

Nnode *Nlist :: add(char *name, int type, node *np1, node *np2)
{
	if ( find(name, type) ) return NULL;

	Nnode *p = new Nnode(type, name, np1, np2);
	if ( np1 ) delSrc(np1), np1->id.src = NULL;
	if ( np2 ) delSrc(np2), np2->id.src = NULL;

	if ( type == DEFINE )
	{
		Nnode *p1 = list;
		while ( p1->parent ) p1 = p1->next;
		
		p->next = p1;				// p1 -> the most outside layer.
		
		for (Nnode *p2 = list; p2->parent; p2 = p2->next)
		{
			if ( p2->parent == p1 ) p2->parent = p;
			if ( p2->next   == p1 ) p2->next   = p;
		}
		
		if ( list == p1 ) list = p;	// don't forget this!
	}
	else
	{
		p->next   = list;
		p->parent = list->parent;
		list = p;
	}
	return p;
}

Nnode *Nlist :: add(Nnode *p)
{
	if ( p && (p->type > 0) )
	{
		Nnode *np = add(p->name, p->type);
		if ( np )
		{
			np->attr  = cloneAttr(p->attr);
			np->np[0] = cloneNode(p->np[0]);
			np->np[1] = cloneNode(p->np[1]);
			return np;
		}
	}
	return NULL;
}

void Nlist :: del(char *name)	// only for #define statement...
{
	for (Nnode *p1 = list; p1; p1 = p1->next)
	{
		if ( p1->type == DEFINE && strcmp(p1->name, name) == 0 )
		{
			for (Nnode *p2 = list; p2 != p1; p2 = p2->next)
			{
				if ( p2->next   == p1 )	p2->next   = p1->next;
				if ( p2->parent == p1 )	p2->parent = p1->next;
			}

			if ( list == p1 ) list = p1->next;

			delete p1;
			return;
		}
	}
}

attrib *Nlist :: search(attrib *ap)
{
	if ( ap->type == TYPEDEF_NAME )
	{
		Nnode *nnp = search(ap->typeName, TYPEDEF);
		return nnp? search(nnp->attr): NULL;
	}

	return ap;
}

Nnode *Nlist :: find(char *name, int type)
{
	for (Nnode *p = list; p; p = p->next)
	{
		if ( p->type == type && strcmp(p->name, name) == 0 )
			return p;
		
		if ( type != DEFINE && p->next == p->parent )
			break;
	}
	return NULL;
}
