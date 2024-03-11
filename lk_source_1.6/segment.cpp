#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "segment.h"

Segment	*libSegGroup = NULL;
Segment	*codeSegGroup= NULL;
Segment	*dataSegGroup= NULL;
Segment	*fuseSegGroup= NULL;
Segment	*miscSegGroup= NULL;

///////////////////////////////////////////////////////////////////////
Segment :: Segment(char *filename)
{
	memset(this, 0, sizeof(Segment));
	fileName = dupStr(filename);
}

Segment :: ~Segment()
{
	free(fileName);
	freeLines(lines);
}

void Segment :: init(void)
{
	if ( lines && lines->type == 'S' && type() != MISC_SEGMENT )
	{
		for (item_t *ip = lines->items; ip; ip = ip->next)
		{
			switch ( ip->type )
			{
				case TYPE_SYMBOL:
					if ( strcmp(ip->data.str, "REL") == 0 )	isREL = 1;
					if ( strcmp(ip->data.str, "ABS") == 0 )	isABS = 1;
					if ( strcmp(ip->data.str, "BEG") == 0 )	isBEG = 1;
					if ( strcmp(ip->data.str, "END") == 0 )	isEND = 1;
					break;
				case TYPE_VALUE:
					if ( isABS ) addr = ip->data.val;
					break;
			}
		}
	}
}

void Segment :: addLine(line_t *lp)
{
	if ( lines == NULL )
		lines = lp;
	else
	{
		line_t *lptr = lines;
		while ( lptr->next )
			lptr = lptr->next;
		lptr->next = lp;
	}
}

int Segment :: type(void)
{
	if ( lines && lines->type == 'S' && lines->items && lines->items->type == TYPE_SYMBOL )
	{
		if ( memcmp(lines->items->data.str, "BANK", 4) == 0 )
			return DATA_SEGMENT;

		if ( memcmp(lines->items->data.str, "CODE", 4) == 0 )
			return CODE_SEGMENT;

		if ( memcmp(lines->items->data.str, "FUSE", 4) == 0 )
			return FUSE_SEGMENT;

		if ( memcmp(lines->items->data.str, "CONST",5) == 0 )
			return CONST_SEGMENT;
	}
	return MISC_SEGMENT;
}

int Segment :: lineCount(void)
{
	int n = 0;
	for (line_t *lp = lines; lp; lp = lp->next)
		n++;

	return n;
}

int Segment :: size(void)
{
	int size = 0;
	for (line_t *lp = lines; lp; lp = lp->next)
	{
		int cnt = itemCount(lp->items);

		switch ( lp->type )
		{
			case 'W':	size += cnt;							break;
			case 'R':	size += cnt? lp->items->data.val: 0;	break;
			case 'K':
			case 'J':	if ( type() == CODE_SEGMENT && cnt > 0 )
						{
							if ( cnt == 1 ) size++;
							else size += (cnt? lp->insert: 0);
						}
		}
	}
	return size;
}

char *Segment :: name(void)
{
	if ( lines && lines->items && lines->items->type == TYPE_SYMBOL )
		return lines->items->data.str;

	return (char*)"";
}

void Segment :: print(void)
{
	printf("Type=%d, name=%s", type(), name());
	line_t *lp = lines->next;
	if ( lp && (lp->type == 'U' || lp->type == 'G' || lp->type == 'L') )
	{
		item_t *ip = lp->items;
		if ( ip->type == TYPE_SYMBOL )
			printf(", Title=%s", ip->data.str);
	}
	printf(", file=%s, addr=0x%x, size=%d\n", fileName, addr&0x7fff, size());
}

///////////////////////////////////////////////////////////////
void deleteSegments(Segment *list)
{
	while ( list )
	{
		Segment *next = list->next;
		delete list;
		list = next;
	}
}

void addSegment(Segment *seg, int lib)
{
	Segment **listp;
	int type = seg->type();

	if ( lib )
		listp = &libSegGroup;
	else
		switch ( type )
		{
			case CONST_SEGMENT:
			case CODE_SEGMENT:	listp = &codeSegGroup;	break;
			case DATA_SEGMENT:	listp = &dataSegGroup;	break;
			case FUSE_SEGMENT:	listp = &fuseSegGroup;	break;
			default:			listp = &miscSegGroup;	break;
		}

	Segment *sp = *listp, *last = NULL;
	if ( sp == NULL )
	{
		*listp = seg;
		return;
	}

	for (; sp; last = sp, sp = sp->next)
	{
		if ( type == MISC_SEGMENT )
			continue;

		bool insert = false;
		int cmp_result = strcmp(seg->name(), sp->name());

		if ( cmp_result > 0 ) continue;
		if ( cmp_result < 0 ) insert = true;
		if ( cmp_result == 0 && sp->isBEG ) continue;

		if ( insert || sp->isEND || seg->isBEG )
		{
			seg->next = sp;
			if ( last )	last->next = seg;
			else *listp = seg;
			return;
		}
	}

	last->next = seg;
}

void printfSegments(Segment *slist)
{
	for (; slist; slist = slist->next)
	{
//		slist->print();
		int type = slist->type();
		printf("Type=%d, file=%s/%d", type, slist->fileName, slist->lines->lineno);
		switch ( type )
		{
			case CODE_SEGMENT:
			case DATA_SEGMENT:
			case CONST_SEGMENT:
			case FUSE_SEGMENT:
				printf(" name=%s", slist->name());
				if ( slist->isREL ) printf(" REL");
				if ( slist->isABS ) printf(" ABS");
				if ( slist->isBEG ) printf(" BEG");
				if ( slist->isEND ) printf(" END");
				if ( slist->isABS ) printf(" Addr=%d", slist->addr);
				break;
		}

		int size = slist->size();
		if ( size > 0 ) printf(" size=%d", size);
		printf("\n");
	}
}
