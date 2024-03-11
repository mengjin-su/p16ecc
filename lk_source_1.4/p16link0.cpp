#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "memory.h"
#include "segment.h"
#include "symbol.h"
#include "p16link.h"
#include "exp.h"

Segment *P16link :: extractLibSegment(Segment **slist, char *lbl, Segment *seg)
{
	Segment *ss = NULL;
	for (Segment *sl = *slist; sl; sl = sl->next)
	for (line_t *lp = sl->lines; lp; lp = lp->next)
	{
		if ( (lp->type == 'G' || lp->type == 'L') &&
			 strcmp(lp->items->data.str, lbl) == 0 )
		{
			if ( strcmp(sl->fileName, seg->fileName) == 0 )
				ss = sl;
			else if ( !ss && lp->type == 'G' )
				ss = sl;
		}
	}

	if ( ss )
	{
		Segment *last = NULL;
		for (Segment *sl = *slist; sl; last = sl, sl = sl->next)
		{
			if ( ss == sl )
			{
				if ( last )	last->next = sl->next;
				else		*slist     = sl->next;

				sl->next = NULL;
				break;
			}
		}
	}
	return ss;
}

void P16link :: searchUnusedSegment(Symbol *slist)
{
	bool updated;
	do {
		updated = false;

		for (Segment *segp = codeSegGroup; segp; segp = segp->next)
		{
			int addr = segp->addr & 0x7fff;
			if ( memcmp(segp->name(), "CODE", 4) == 0 && segp->isABS &&
				(addr == 0x0000 || addr == 0x0004) && !segp->isUsed )
			{
				segp->isUsed = 1;
				updated = true;
			}

			if( strcmp(segp->name(), "CODEi") == 0 && !segp->isUsed )
			{
				segp->isUsed = 1;
				updated = true;
			}

			if ( segp->isUsed )
			{
				for (line_t *lp = segp->lines->next; lp; lp = lp->next)
				{
					switch ( lp->type )
					{
						case 'W':
						case 'J':
						case 'K':
							for (item_t *ip = lp->items; ip; ip = ip->next)
							{
								if ( searchUnusedSegment(segp, ip, slist) )
									updated = true;
							}
							break;
					}
				}
			}
		}
	} while ( updated );
}

bool P16link :: searchUnusedSegment(Segment *segp, item_t *ip, Symbol *slist)
{
	Symbol *symb;
	switch ( ip->type )
	{
		case TYPE_SYMBOL:
			symb = searchSymbol(slist, ip->data.str, segp);
			if ( symb && symb->segment && !symb->segment->isUsed )
			{
				symb->segment->isUsed = 1;
				return true;
			}
			return false;

		case ':':	case RSHIFT:   case LSHIFT:
		case '+':	case '-':	case '*':	case '/':	case '%':
		case '&':	case '|':	case '^':
			return ( searchUnusedSegment(segp, ip->left,  slist) ||
					 searchUnusedSegment(segp, ip->right, slist) );
	}
	return false;
}

void P16link :: removeUnusedSegment(Segment **segp)
{
	bool done;
	do {
		done = true;
		Segment *last = NULL;
		for (Segment *sp = *segp; sp; last = sp, sp = sp->next)
		{
			if ( !sp->isUsed )
			{
				Segment *temp = sp->next;
				sp->next = NULL;
				delete sp;

				if ( last == NULL )
					*segp = temp;
				else
					last->next = temp;

				done = false;
				break;
			}
		}
	} while ( !done );
}
