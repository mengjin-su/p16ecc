#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "segment.h"
#include "symbol.h"


Symbol :: Symbol(char *sname)
{
	memset(this, 0, sizeof(Symbol));
	name = dupStr(sname);
}

Symbol :: Symbol(char *sname, int stype, Segment *seg)
{
	memset(this, 0, sizeof(Symbol));
	type = stype;
	name = dupStr(sname);
	segment = seg;
}

Symbol :: ~Symbol()
{
	free(name);
}

///////////////////////////////////////////////////////////////////
void addSymbol(Symbol **slist, Symbol *sp)
{
	if ( *slist == NULL )
		*slist = sp;
	else
	{
		Symbol *sl = *slist;
		while ( sl->next ) sl = sl->next;
		sl->next = sp;
	}
}

void addSymbol(Symbol **slist, char *file)
{
	addSymbol(slist, new Symbol(file));
}

void deleteSymbols(Symbol **slist)
{
	Symbol *sl = *slist;
	while ( sl )
	{
		Symbol *next = sl->next;
		delete sl;
		sl = next;
	}
	*slist = NULL;
}

Symbol *searchSymbol(Symbol *slist, char *str, Segment *seg)
{
	Symbol *ss = NULL;
	for (; slist; slist = slist->next)
	{
		if ( strcmp(slist->name, str) == 0 )
		{
			if ( seg == NULL ) return slist;

			// same .obj file, higher priority
			if ( strcmp(seg->fileName, slist->segment->fileName) == 0 )
			{
				if ( seg == slist->segment )	// same segment
					return slist;

				ss = slist;
			}
			else if ( !ss && (slist->type == 'E' || slist->type == 'G') )
			{
				ss = slist;
			}
		}
	}
	return ss;
}

Symbol *logSymbol(Symbol **slist, int type, char *name, Segment *seg, int val)
{
	Symbol *symb = new Symbol(name, type, seg);
	if ( symb )
		symb->value = val;

	if ( *slist == NULL )
		*slist = symb;
	else
	{
		Symbol *sp = *slist;
		while ( sp->next ) sp = sp->next;
		sp->next = symb;
	}
	return symb;
}

void printSymbol(Symbol *slist)
{
	for (; slist; slist = slist->next)
		printf("%s: (%c) - 0x%x\n", slist->name, slist->type, slist->value);
}