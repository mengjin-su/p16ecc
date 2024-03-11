#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "memory.h"
#include "segment.h"
#include "symbol.h"
#include "p16link.h"
#include "fcall.h"

void P16link :: scanFcall(Segment *segp)
{
	fcallMgr = new FcallMgr();

	for (; segp; segp = segp->next)
	{
		if ( segp->type() != CODE_SEGMENT ) continue;

		bool add_list = false;
		for (line_t *lp = segp->lines; lp; lp = lp->next)
			switch ( lp->type )
			{
				case 'U':	// segment data specifier
					fcallMgr->addFcallName(lp, segp);
					add_list = true;
					break;

				case 'F':	// function call
					if ( add_list )
						fcallMgr->addFcallLink(lp);
					break;

				case 'G':	// global label
					if ( add_list )
						fcallMgr->setGlobal(lp, lp->items->data.str);
					break;
			}
	}

	fcallMgr->createBasicMatrix();
	fcallMgr->createExtendMatrix();
}

/* function to allocate functions' internval variables using overlap sharing stratagy */
void P16link :: assignFuncLocalData(Symbol **symlist)
{
	char func_list[fcallMgr->funcCount];
	int  call_list[fcallMgr->funcCount];
	memset(func_list, 0, fcallMgr->funcCount);

	for (bool done = false; !done;)
	{
		int addr, length = 0;
		int max_size = 0;

		for (int i = 0; i < fcallMgr->funcCount; i++)
		{
			FuncAttr *fp1 = fcallMgr->getFunc(i);
			if ( !func_list[i] && fp1->dataSize )
			{
				if ( fcallMgr->joinInGroup(call_list, length, i) )
				{
					call_list[length++] = i;
					func_list[i] = 1;
					if ( max_size < fp1->dataSize ) max_size = fp1->dataSize;
				}
			}
		}

		if ( max_size && dataMem->getSpace(0, &addr, max_size) )
		{
			for (int i = 0; i < length; i++)
			{
				FuncAttr *fp = fcallMgr->getFunc(call_list[i]);
				std::string f_name = fp->funcName;	f_name += "_$data$";
				logSymbol(symlist, 'U', (char*)f_name.c_str(), fp->segment, addr);
			}
		}
		done = (max_size == 0);
	}
}