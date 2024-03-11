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

#define INIT_CODE(s)		(strcmp(s->name(), "CODEi") == 0)
#define INIT_CODE_BEG(s)	(INIT_CODE(s) && s->isBEG)
#define ISR_CODE(s)			(strcmp(s->name(), "CODE1") == 0)
#define ISR_CODE_BEG(s)		(ISR_CODE(s) && s->isBEG)

void P16link :: removeExtraJKLines(void)
{
	for (Segment *seg = codeSegGroup; seg; seg = seg->next)
	{
		line_t *last_line = NULL;
		for (line_t *lp1 = seg->lines; lp1; lp1 = lp1->next)
		{
			line_t *lp2 = lp1->next;
			if ( (lp1->type == 'J' || lp1->type == 'K') &&
				 lp2 && lp2->type == lp1->type )
			{
				item_t *ip1p = lp1->items; 
				item_t *ip2p = lp2->items; 
				if ( itemCount(ip1p) == 2 && itemCount(ip2p) == 2 )
				{
					freeItem(ip1p->next);
					ip1p->next = ip2p->next;
					ip2p->next = NULL;
					
					lp1->next = lp2->next;
					freeLine(lp2);
					
					if ( last_line ) lp1 = last_line;
				}
			}
			last_line = lp1;
		}
	}	
}

///////////////////////////////////////////////////////////////
int P16link :: scanSegmentGroups(void)
{
	deleteSymbols(&symbList);
	codeMem->reset();
	dataMem->reset();

	scanSegmentGroup(dataSegGroup, RAM_LOC_ABS);	// assign ABS data

	scanFuncLocalData(&symbList);					// assign func local data
	scanSegmentGroup(dataSegGroup, RAM_LOC_FLOAT);	// assign rest float data

	logSegSymbols(dataSegGroup, &symbList, 1);

	scanSegmentGroup(codeSegGroup, ROM_LOC_ABS);
	scanSegmentGroup(codeSegGroup, ROM_LOC_FLOAT);
	logSegSymbols(codeSegGroup, &symbList, 1);

	scanSegmentGroup(fuseSegGroup, ROM_LOC_ABS);
	logSegSymbols(miscSegGroup, &symbList, 1);
	return errorCount;
}

void P16link :: scanSegmentGroup(Segment *seg_p, int mode)
{
	for (; seg_p; seg_p = seg_p->next)
	{
        int   seg_type = seg_p->type();// segment type
		char *seg_name = seg_p->name();// segment name
		int	  addr = 0;
		int   flag = 0;

		if ( seg_p->isABS )
		{
			flag |= FIXED_ADDR_FLAG;
			addr  = seg_p->addr;
		}

        switch ( seg_type )
        {
            case CODE_SEGMENT:
				flag |= PAGE_CHECK_FLAG;
            case CONST_SEGMENT:
				if ( !(flag & FIXED_ADDR_FLAG) && mode == ROM_LOC_ABS )
					continue;

				if (  (flag & FIXED_ADDR_FLAG) && mode != ROM_LOC_ABS )
					continue;

				assignSegmentMem(codeMem, seg_p, addr, flag);
                break;

            case DATA_SEGMENT:
				switch ( mode )
				{
					case RAM_LOC_ABS:
						if ( !(flag & FIXED_ADDR_FLAG) )
							continue;
						break;

					case RAM_LOC_FLOAT:
						if ( (flag & FIXED_ADDR_FLAG) || isdigit(seg_name[4]) )
							continue;

						if ( !strcmp(seg_name, "BANKn") )
							flag |= LINEAR_DATA_FLAG;
						else
							continue;

				}
				assignSegmentMem(dataMem, seg_p, addr, flag);
                break;
        }
	}
}

void P16link :: assignSegmentMem(Memory *mem, Segment *seg, int addr, int flag)
{
	char *sname= seg->name();	// segment name

	if ( INIT_CODE(seg) || ISR_CODE(seg) )		// init code
	{
		if ( !(INIT_CODE_BEG(seg) || ISR_CODE_BEG(seg)) )
			return;

		// process all init code...
		int init_size = 0;	// all those spaces are continious.
		for (Segment *isp = seg; isp && INIT_CODE(isp); isp = isp->next)
			init_size += isp->size();

		addr = mem->getSpace(addr, init_size, 0); // allow crossing pages!
		if ( addr < 0 )	// don't have memory space !!!
		{
			line_t *lp = seg->lines;
			printf("%s #%d, out of memory! --- %s\n",
					lp->fname, lp->lineno, sname? sname: "");
			errorCount++;
			return;
		}

		for (Segment *isp = seg; isp && INIT_CODE(isp); isp = isp->next)
		{
			isp->addr = addr;
			addr += isp->size();
		}
		return;
	}

	if ( mem )
		addr = mem->getSpace(addr, seg->size(), flag);

	if ( addr < 0 )	// don't have memory space !!!
	{
		line_t *lp = seg->lines;
		printf("%s #%d, out of memory! --- %s , flags = %d\n",
				lp->fname, lp->lineno, sname? sname: "", flag);
		errorCount++;
	}
	else //if ( !(flag & FIXED_ADDR_FLAG) )
	{
		seg->addr = addr;
	}
}

int P16link :: confirmSegmentMem(void)
{
	int ret = 0;
	for (Segment *seg = codeSegGroup; seg; seg = seg->next)
	{
		int addr = seg->addr;	// segment start address
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
						expValue(ip,  seg, symbList, &v0, addr);
						expValue(ip1, seg, symbList, &v1, addr);
						if ( ip1 )
						{
							if ( ltype == 'J' )	// 'J' - rom page switch
							{
								int switch_page = (v0 ^ v1) & (0xf << 11);
								if ( switch_page && !lp->insert )
								{
									lp->insert = 1;
									ret = 1;
								}
								if ( !switch_page && lp->insert && !lp->retry )
								{
									lp->insert = 0;
									ret = 1;
									lp->retry++;
								}
							}
							else				// 'K' - ram page switch
							{
								if ( v0 & 0x2000 ) v0 = (v0 - 0x2000)/80;
								else			   v0 = v0 >> 7;
								if ( v1 & 0x2000 ) v1 = (v1 - 0x2000)/80;
								else			   v1 = v1 >> 7;

								if ( v0 != v1 && !lp->insert )
								{
									lp->insert = 1;
									ret = 1;
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
	return ret;
}

void P16link :: scanFuncLocalData(Symbol **symlist)
{
	if ( fcallHandler->funcCount > 0 )
	{
		char func_list[fcallHandler->funcCount];
		int  func_vect[fcallHandler->funcCount];
		memset(func_list, 0, sizeof(func_list));

		for (int I = 0;;I++) {
			int l = 0;
			int mem_size = 0;
			for (int i = 0; i < fcallHandler->funcCount; i++)
			{
				int f_datasize = fcallHandler->funcDataSize(i);
				if ( func_list[i] == 0 && f_datasize > 0 &&
					 fcallHandler->testFuncCall(func_vect, l, i) )
				{
					func_vect[l++] = i;
					func_list[i]   = 1;
					if ( mem_size < f_datasize )
						mem_size = f_datasize;
				}
			}

			if ( l == 0 )
				break;

			int addr = dataMem->getSpace(0, mem_size, 0);
			if ( addr > 0 )
			{
				for (int i = 0; i < l; i++)
				{
					char buf[4096];					
					int size = fcallHandler->funcDataSize(func_vect[i]);
					FuncAttr *fp = fcallHandler->getFunc(func_vect[i]);
					Segment *seg = fp->segment;
					sprintf(buf, "%s_$data$", fp->funcName);
					Symbol *s = logSymbol(symlist, 'U', buf, seg, addr);
					if ( s ) s->size = size;
				}
			}
		}
	}
}
