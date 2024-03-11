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
#include "exp.h"

#define INIT_CODE(s)	s->isName("CODEi")
#define ISR_CODE(s)		s->isName("CODE1")

void P16link :: assignSegmentMem(Segment *seg_p, int mode)
{
	for (; seg_p; seg_p = seg_p->next)
	{
		int	addr = 0;	// default start adddress
		int flag = 0;	// condition flags.

		if ( seg_p->isABS )
		{
			flag |= FIXED_ADDR_FLAG;
			addr  = seg_p->memAddr;
		}

        switch ( seg_p->type() )
        {
            case CODE_SEGMENT:
				flag |= PAGE_CHECK_FLAG;
            case CONST_SEGMENT:
				if ( !(flag & FIXED_ADDR_FLAG) && mode == ROM_LOC_ABS )
					continue;

				if (  (flag & FIXED_ADDR_FLAG) && mode != ROM_LOC_ABS )
					continue;

				assignSegmentAddr(codeMem, seg_p, addr, flag);
                break;

            case DATA_SEGMENT:
				if ( seg_p->isName("BANKn") )
					flag |= LINEAR_DATA_FLAG;

				switch ( mode )
				{
					case RAM_LOC_ABS:
						if ( !(flag & FIXED_ADDR_FLAG) )
							continue;
						break;

					case RAM_LOC_LINEAR:
						if ( (flag & FIXED_ADDR_FLAG) || !(flag & LINEAR_DATA_FLAG) )
							continue;
						break;

					case RAM_LOC_FLOAT:
						if ( (flag & FIXED_ADDR_FLAG) || (flag & LINEAR_DATA_FLAG) )
							continue;
				}
				assignSegmentAddr(dataMem, seg_p, addr, flag);
                break;
        }
	}
}

void P16link :: assignSegmentAddr(Memory *mem, Segment *seg, int addr, int flag)
{
	char *seg_name = seg->name();	// segment name
	bool multi_seg = INIT_CODE(seg) || ISR_CODE(seg);

	if ( multi_seg && !seg->isBEG )	// init/code  code
		return;

	int req_size = seg->size();
	for (Segment *ssp = seg->next; multi_seg && ssp && ssp->isName(seg_name); ssp = ssp->next)
		req_size += ssp->size();

	if ( !mem->getSpace(addr, &addr, req_size, flag) )	// don't have memory space !!!
	{
		line_t *lp = seg->lines;
		printf("%s #%d, out of memory! --- %s\n", lp->fname, lp->lineno, seg_name);
		errorCount++;
		return;
	}

	seg->memAddr = addr;
	addr += seg->size();
	for (Segment *ssp = seg->next; multi_seg && ssp && ssp->isName(seg_name); ssp = ssp->next)
	{
		ssp->memAddr = addr;
		addr += ssp->size();
	}
}

bool P16link :: confirmSegmentMem(void)
{
	bool done = true;
	for (Segment *seg = codeSegGroup; seg; seg = seg->next)
	{
		int addr = seg->memAddr;	// segment start address
		for (line_t *lp = seg->lines; lp; lp = lp->next)
		{
			int ltype  = lp->type;
			item_t *ip = lp->items;
			int item_count = itemCount(ip);
			int v, v0, v1;
			switch ( ltype )
			{
				case 'R':
					addr += ip->data.val;
					break;

				case 'W':
					for (; ip; ip = ip->next)
					{
						expValue(ip, seg, symbList, &v, addr);
						addr++;
					}
					break;

				case 'J':
				case 'K':
					if ( ip )
					{
						item_t *ip1 = ip->next;
						char  *lbl0 = expValue(ip,  seg, symbList, &v0, addr);
						char  *lbl1 = expValue(ip1, seg, symbList, &v1, addr);
						if ( ip1 && !lbl0 && !lbl1 )
						{
							if ( ltype == 'J' )	// 'J' - rom page switch
							{
								int switch_page = (v0 ^ v1) & (0xf << 11);
								if ( switch_page && !lp->insert )
								{
									lp->insert = 1;
									done = false;
								}
								if ( !switch_page && lp->insert && !lp->retry )
								{
									lp->insert = 0;
									done = false;
									lp->retry++;
								}
							}
							else				// 'K' - ram bank switch
							{
								v0 = (v0 >= 0x2000)? (v0 % 0x2000)/80: v0 >> 7;
								v1 = (v1 >= 0x2000)? (v1 % 0x2000)/80: v1 >> 7;
								if ( v0 != v1 && !lp->insert )
								{
									lp->insert = 1;
									done = false;
								}
							}
						}

						if ( !ip1 || lp->insert )
							addr++;
					}
					break;
			}
		}
	}
	return done;
}
#if 0
void P16link :: assignFuncLocalData(Symbol **symlist)
{
	for (Segment *sp = codeSegGroup; sp; sp = sp->next)
	for (line_t *lp = sp->lines; lp; lp = lp->next)
	{
		item_t *ip0 = lp->items;
		if ( lp->type == 'U' && ip0 && sp->dataSize > 0 )
		{
			std::string f_name = ip0->data.str;	f_name += "_$data$";

			int addr;
			if ( dataMem->getSpace(0, &addr, sp->dataSize) )
				logSymbol(symlist, 'U', (char*)f_name.c_str(), sp, addr);
			else
			{
				printf("local data '%s', out of memory!\n", f_name.c_str());
				errorCount++;
			}
		}
	}
}
#endif