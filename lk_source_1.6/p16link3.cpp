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

#define INIT_CODE(s)	s->isName("CODEi")
#define ISR_CODE(s)		s->isName("CODE1")

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

	logSegmentSymbols(dataSegGroup, &symbList);		// log data symbols

	scanSegmentGroup(codeSegGroup, ROM_LOC_ABS);	// assign ABS code address
	scanSegmentGroup(codeSegGroup, ROM_LOC_FLOAT);	// assign float code address
	logSegmentSymbols(codeSegGroup, &symbList);		// log code symbols

	scanSegmentGroup(fuseSegGroup, ROM_LOC_ABS);
	logSegmentSymbols(miscSegGroup, &symbList);
	return errorCount;
}

void P16link :: scanSegmentGroup(Segment *seg_p, int mode)
{
	for (; seg_p; seg_p = seg_p->next)
	{
		int	addr = 0;	// default start adddress
		int flag = 0;	// condition flags.

		if ( seg_p->isABS )
		{
			flag |= FIXED_ADDR_FLAG;
			addr  = seg_p->addr;
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

				assignSegmentMem(codeMem, seg_p, addr, flag);
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

					case RAM_LOC_FLOAT:
						if ( (flag & FIXED_ADDR_FLAG) )
							continue;
				}
				assignSegmentMem(dataMem, seg_p, addr, flag);
                break;
        }
	}
}

void P16link :: assignSegmentMem(Memory *mem, Segment *seg, int addr, int flag)
{
	char *seg_name = seg->name();	// segment name
	bool multi_seg = INIT_CODE(seg) || ISR_CODE(seg);

	if ( multi_seg && !seg->isBEG && mem->type == CODE_MEMORY )	// init code or isr code
		return;

	int req_size = seg->size();
	for (Segment *ssp = seg->next; multi_seg && ssp && ssp->isName(seg_name); ssp = ssp->next)
		req_size += ssp->size();

	addr = mem->getSpace(addr, req_size, flag);
	if ( addr < 0 )	// don't have memory space !!!
	{
		line_t *lp = seg->lines;
		printf("%s #%d, out of memory! --- %s\n", lp->fname, lp->lineno, seg_name);
		errorCount++;
		return;
	}

	seg->addr = addr;
	addr += seg->size();
	for (Segment *ssp = seg->next; multi_seg && ssp && ssp->isName(seg_name); ssp = ssp->next)
	{
		ssp->addr = addr;
		addr += ssp->size();
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
		int  length = fcallHandler->funcCount;
		char func_list[length];
		int  func_vect[length];
		char buf[4096];
		memset(func_list, 0, sizeof(func_list));
		
#if 0
		for (int i = 0; i < length; i++)	// set 'main()' function local data
		{
			FuncAttr *fp = fcallHandler->getFunc(i);
			Segment  *sp = fp->segment;
			if ( sp->isName("CODE2") && strcmp(fp->funcName, "main") )
			{
				int data_size = fcallHandler->funcDataSize(i);
				sprintf(buf, "%s_$data$", fp->funcName);
				int addr = dataMem->getSpace(0, data_size);
				Symbol *s = logSymbol(symlist, 'U', buf, sp, addr);
				s->size = data_size;
				func_list[i] = 1;
				break;
			}
		}
		
		int addr, isr_data_size = 0;
		for (int I = 0; I < 2; I++)			// set ISR functions local data
		{
			if ( I )
				addr = dataMem->getSpace(0, isr_data_size);

			for (int i = 0; i < length; i++)
			{
				FuncAttr *fp = fcallHandler->getFunc(i);
				Segment  *sp = fp->segment;
				if ( sp->isName("CODE1") && !(sp->isBEG || sp->isEND) )
				{
					if ( I == 0 )
					{
						if ( isr_data_size < fcallHandler->funcDataSize(i) )
							isr_data_size = fcallHandler->funcDataSize(i);
					}
					else if ( isr_data_size > 0 && addr > 0 )
					{
						sprintf(buf, "%s_$data$", fp->funcName);
						Symbol *s = logSymbol(symlist, 'U', buf, sp, addr);
						if ( s ) s->size = fcallHandler->funcDataSize(i);
						func_list[i] = 1;
					}
				}
			}
		}
#endif
		for (int I = 0;; I++) {
			int l = 0;
			int mem_size = 0;
#if 0			
			int root_type = 0;
			for (int i = 0; i < length; i++)
			{
				if ( func_list[i] ) continue;
				
				int f_datasize = fcallHandler->funcDataSize(i);
				int f_root     = fcallHandler->funcRootType(i);
				if ( f_datasize == 0 ) continue;
				
				func_vect[l++] = i;
				
				if ( l == 1 )	// first function...
				{
					root_type = f_root;
					mem_size  = f_datasize;

					if ( f_root == (MAIN_ROOT|ISR_ROOT) || f_root == 0 ) break;
					continue;
				}

				if ( root_type == f_root )
				{	
					if ( mem_size < f_datasize )
						mem_size = f_datasize;
				}
				else
					l--;
			}
#else
			for (int i = 0; i < length; i++)
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
#endif
			if ( l == 0 )
				break;

			int addr = dataMem->getSpace(0, mem_size);
			if ( addr >= 0 )
			{
				for (int i = 0; i < l; i++)
				{
					int size = fcallHandler->funcDataSize(func_vect[i]);
					FuncAttr *fp = fcallHandler->getFunc(func_vect[i]);
					Segment  *sp = fp->segment;
					sprintf(buf, "%s_$data$", fp->funcName);
					Symbol *s = logSymbol(symlist, 'U', buf, sp, addr);
					if ( s ) s->size = size;
				}
			}
		}
	}
}
