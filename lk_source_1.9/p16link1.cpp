#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "memory.h"
#include "segment.h"
#include "symbol.h"
#include "p16link.h"

void P16link :: assignSegmentsAddress(void)
{
	scanFuncLocalData();
	scanFcall(codeSegGroup);

	while ( !errorCount )
	{
		codeMem->reset();
		dataMem->reset();
		deleteSymbols(&symbList);

		// assign data addresses...
		assignSegmentMem(dataSegGroup, RAM_LOC_ABS);	// assign address for ABS data
		assignSegmentMem(dataSegGroup, RAM_LOC_LINEAR);	// assign address for linear data
		assignSegmentMem(dataSegGroup, RAM_LOC_FLOAT);	// assign address for float data
		logSegmentSymbols(dataSegGroup, &symbList);		// log data symbols

		assignFuncLocalData(&symbList);					// assign address for func local data

		// assign code addresses...
		assignSegmentMem(codeSegGroup, ROM_LOC_ABS);	// assign address for ABS code
		assignSegmentMem(codeSegGroup, ROM_LOC_FLOAT);	// assign address for float code
		logSegmentSymbols(codeSegGroup, &symbList);		// log code symbols

		// assign fuse addresses...
		assignSegmentMem(fuseSegGroup, ROM_LOC_ABS);	// log fuse contents

		logSegmentSymbols(miscSegGroup, &symbList);

		if ( confirmSegmentMem() ) break;				// confirm code assignments
	}
/*
	for (Symbol *sp = symbList; sp; sp = sp->next)
		printf("%s (%c), value = %d (0x%X)\n", sp->name, sp->type, sp->value, sp->value);
*/
}

void P16link :: scanFuncLocalData(void)
{
	for (Segment *sp = codeSegGroup; sp; sp = sp->next)
	for (line_t *lp = sp->lines; lp; lp = lp->next)
	{
		if ( lp->type == 'U' )
		{
			item_t *ip0 = lp->items;
			item_t *ip1 = ip0? ip0->next: NULL;
			if ( ip1 && ip0->type == TYPE_SYMBOL &&	ip1->type == TYPE_VALUE )
				sp->dataSize = ip1->data.val;
			else
			{
				printf("%s #%d, invalid format!\n", lp->fname, lp->lineno);
				errorCount++;
			}
		}
	}
}

#if 0
void P16link :: scanFuncCalls(void)
{
	fcallMgr = new FcallMgr;
	for (Segment *sp = codeSegGroup; sp; sp = sp->next)
	{
		bool add_list = false;
		for (line_t *lp = sp->lines; lp; lp = lp->next)
			switch ( lp->type )
			{
				case 'U':	// segment data specifier
					fcallMgr->addFunc(lp, sp);
					add_list = true;
					break;

				case 'F':	// function call
					if ( add_list )
						fcallMgr->addFcall(lp);
					break;

				case 'G':	// global label
					if ( add_list )
						fcallMgr->setGlobal(lp, lp->items->data.str);
					break;
			}
	}

	fcallMgr->createShareGroups();
}
#endif