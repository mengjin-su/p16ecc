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

P16link :: P16link(Memory *ram, Memory *rom)
{
	memset(this, 0, sizeof(P16link));
	dataMem = ram;
	codeMem = rom;
}

P16link :: ~P16link()
{
	deleteSymbols(&symbList);
	if ( fcallMgr ) delete fcallMgr;
}

bool P16link :: scanInclusion(void)
{
	Symbol *sym_list = NULL;

	logSegmentSymbols(codeSegGroup, &sym_list, true);
	logSegmentSymbols(dataSegGroup, &sym_list);
	logSegmentSymbols(miscSegGroup, &sym_list);

	// remove unused segments (code & data)
	searchUsedSegment(sym_list);
	removeUnusedSegment(&codeSegGroup);
	removeUnusedSegment(&dataSegGroup);

	deleteSymbols(&sym_list);
	return (errorCount == 0);
}

void P16link :: logSegmentSymbols(Segment *seglist, Symbol **symlist, bool incl_func_name)
{
	for (Segment *sp = seglist; sp; sp = sp->next)
	{
		int stype = sp->type();
		int addr  = sp->memAddr;
		for (line_t *lp = sp->lines; lp; lp = lp->next)
		{
			item_t *ip = lp->items;
			int ltype = lp->type;
			int value;
			char buf[4096];

			switch ( ltype )
			{
				case 'U':
					if ( ip && ip->type == TYPE_SYMBOL && incl_func_name )
					{
						sprintf(buf, "%s_$data$", ip->data.str);
						logSymbol(symlist, ltype, buf, sp, 0);
					}
					break;

				case 'G':
				case 'L':
					if ( !strstr(ip->data.str, "$init$") &&
						 (stype == CONST_SEGMENT || stype == DATA_SEGMENT) )
					{
						value = sp->size();
						sprintf(buf, "%s$sizeof$", ip->data.str);
						logSymbol(symlist, ltype, buf, sp, value);
					}
					logSymbol(symlist, ltype, ip->data.str, sp, addr);
					break;

				case 'E':
					if ( ip->next )
					{
						value = ip->next->data.val;
						logSymbol(symlist, ltype, ip->data.str, sp, value);
					}
					break;

				case 'W':
					addr += itemCount(ip);
					break;

				case 'R':
					addr += ip->data.val;
					break;

				case 'J':
				case 'K':
					addr += (itemCount(ip) == 1)? 1: lp->insert;
					break;
			}
		}
	}
}

void P16link :: searchUsedSegment(Symbol *slist)
{
	for (bool done = false; !done;)
	{
		done = true;
		for (Segment *segp = codeSegGroup; segp; segp = segp->next)
		{
			// those segments must be included...
			if ( (segp->isName("CODE0") ||
				  segp->isName("CODE1") ||
				  segp->isName("CODEi")  ) && !segp->isUsed )
			{
				segp->isUsed = 1;
				done = false;
			}

			if ( segp->isUsed )
			{
				for (line_t *lp = segp->lines->next; lp; lp = lp->next)
				{
					if ( lp->type == 'W' )
						for (item_t *ip = lp->items; ip; ip = ip->next)
						{
							if ( searchUsedSegment(segp, ip, slist) )
								done = false;
						}
				}
			}
		}
	}
}

bool P16link :: searchUsedSegment(Segment *segp, item_t *ip, Symbol *slist)
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
			return ( searchUsedSegment(segp, ip->left,  slist) ||
					 searchUsedSegment(segp, ip->right, slist) );

		case '~':
			return searchUsedSegment(segp, ip->left,  slist);
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
