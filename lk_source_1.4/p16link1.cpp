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

int P16link :: scanLibInclusion(void)
{
	Symbol *symList = NULL;

	// scan segments for lib including...
	do {
		deleteSymbols(&symList);		// clear symbol list

		if ( errorCount > 0 )
			return errorCount;

		logSegLabels(codeSegGroup, &symList, 0);
		logSegLabels(dataSegGroup, &symList, 0);
		logSegLabels(miscSegGroup, &symList, 0);

	} while ( scanSegLabels(codeSegGroup, symList) ||
			  scanSegLabels(dataSegGroup, symList) ||
			  scanSegLabels(miscSegGroup, symList)  );

	// remove unused segments (code & data)
	searchUnusedSegment(symList);
	removeUnusedSegment(&codeSegGroup);
	removeUnusedSegment(&dataSegGroup);

	deleteSymbols(&symList);
	return errorCount;
}

void P16link :: logSegLabels(Segment *seglist, Symbol **symlist, int mode)
{
	for (Segment *sp = seglist; sp; sp = sp->next)
	{
		int stype = sp->type();
		int addr  = sp->addr;
		for (line_t *lp = sp->lines; lp; lp = lp->next)
		{
			item_t *ip = lp->items;
			int ltype = lp->type;
			int value;
			char buf[1024];

			switch ( ltype )
			{
				case 'U':
					if ( ip && ip->type == TYPE_SYMBOL && mode == 0 )
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

bool P16link :: scanSegLabels(Segment *seglist, Symbol *symlist)
{
	for (; seglist; seglist = seglist->next)
	for (line_t *lp = seglist->lines; lp; lp = lp->next)
	{
		switch ( lp->type )
		{
			case 'W':
			case 'R':
			case 'K':
			case 'J':
				for (item_t *ip = lp->items; ip; ip = ip->next)
				{
					int v;
					char *lbl = expValue(ip, seglist, symlist, &v, 0);
					if ( lbl )
					{	// not found in current list, then search for lib files.
						Segment *seg = extractLibSegment(&libSegGroup, lbl, seglist);
						if ( seg )	// found the seg in .lib file
						{
							addSegment(seg, 0);
							return true;
						}
						printf("%s #%d, symbol not defined: '%s'!\n",
							   lp->fname, lp->lineno, lbl);
						errorCount++;
					}
				}
				break;
		}
	}
	return false;
}
