#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "flink.h"
#include "dlink.h"
#include "sizer.h"

///////////////////////////////////////////////////////////////////
Fnode :: Fnode(char *_name, int _type)
{
	memset(this, 0, sizeof(Fnode));
	name   = dupStr(_name);
	ftype  = _type;
	dlink  = new Dlink(PAR_DLINK);
}

Fnode :: ~Fnode()
{
	free(name);
	if ( attr ) delAttr(attr);
	delName(&fcall);
}

attrib *Fnode :: parAttr(int index)
{
	Dnode *dp = dlink->dlist;
	for (int i = 0; dp; i++, dp = dp->next)
	{
		if ( i == index )
			return dp->attr;
	}
	return NULL;
}

int Fnode :: parSize(void)
{
	int n = 0;
	for(Dnode *dnp = dlink->dlist; dnp; dnp = dnp->next)
		n += dnp->size();
	return n;
}

Dnode *Fnode ::	getData(Dnode *dnp, FDATA_t type, int *depth, int *offset)
{
	for (; dnp; dnp = dnp->next)
	{
		attrib *attr = dnp->attr;
		if ( (attr->isStatic && type == STATIC_DATA) ||
			 (!attr->isStatic && type == GENERIC_DATA) )
		{
			if ( *depth == 0 )
				return dnp;

			(*depth)--;
			*offset += dnp->size();
		}
	}
	return NULL;
}

Dnode *Fnode ::	getData(Dlink *dlp, FDATA_t type, int *depth, int *offset)
{
	Dnode *d;
	int offs = *offset;
	for(; dlp; dlp = dlp->next)
	{
		*offset = offs;
		d = getData(dlp->dlist, type, depth, offset);
		if ( d ) return d;

		d = getData(dlp->child, type, depth, offset);
		if ( d ) return d;
	}
	return NULL;
}

Dnode *Fnode ::	getData(FDATA_t type, int *depth, int *offset)
{
	*offset = 0;

	// skip parameter link
	return getData(dlink->child, type, depth, offset);
}

///////////////////////////////////////////////////////////////////
Flink :: ~Flink()
{
	while ( flist ) {
		Fnode *next = flist->next;
		delete flist;
		flist = next;
	}
}

Fnode *Flink :: search(char *name)
{
	for (Fnode *lk = flist; lk; lk = lk->next)
	{
		if ( strcmp(name, lk->name) == 0 )
			return lk;
	}
	return NULL;
}

void Flink :: add(Fnode *fp)
{
	if ( flist == NULL )
		flist = fp;
	else
	{
		Fnode *lk = flist;
		while ( lk->next ) lk = lk->next;
		lk->next = fp;
	}
}
