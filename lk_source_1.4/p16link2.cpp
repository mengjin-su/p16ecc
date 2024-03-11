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
#include "exp.h"
#include "fcall.h"

void P16link :: scanFuncCalls(void)
{
	fcallHandler = new FcallMgr;
	for (Segment *sp = codeSegGroup; sp; sp = sp->next)
	{
		bool add_list = false;
		for (line_t *lp = sp->lines; lp; lp = lp->next)
			switch ( lp->type )
			{
				case 'U':	// segment data specifier
					fcallHandler->addFunc(lp, sp);
					add_list = true;
					break;

				case 'F':	// function call
					if ( add_list )
						fcallHandler->addFcall(lp);
					break;

				case 'G':	// global label
					if ( add_list )
						fcallHandler->setGlobal(lp, lp->items->data.str);
					break;
			}
	}

	fcallHandler->createShareGroups();
}