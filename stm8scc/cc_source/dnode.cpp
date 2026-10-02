#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "sizer.h"
#include "dnode.h"
#include "flink.h"

Dnode :: Dnode (char *_name, attrib *_attr, int _index)
{
	memset(this, 0, sizeof(Dnode));
	name  = dupStr(_name);
	attr  = cloneAttr(_attr);
	index = _index;
    atAddr= -1;
}

Dnode :: ~Dnode ()
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
			sprintf (buf, "%s_$%d_%s", func, index, name);
		else
			sprintf (buf, "%s_$_%s", func, name);
		return buf;
	}
	else
		return name;
}

int Dnode :: size (void)
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

void Dnode :: display(int spaces)
{
	while ( spaces-- ) printf(" ");
	printf("%s  (%d)\n", nameStr(), size());
}