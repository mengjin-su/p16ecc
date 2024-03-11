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
#include "fwriter_hex.h"
#include "fwriter_map.h"

void P16link :: outputHex(char *filename)
{
	HexWriter *hexWriter = new HexWriter(filename);

	for (Segment *seg = codeSegGroup; seg; seg = seg->next)
	{
		int addr = seg->addr & 0x7fff;
		for (line_t *lp = seg->lines; lp; lp = lp->next)
		{
			item_t *ip = lp->items;
			int item_count = itemCount(ip);
			int w, v0, v1;
			switch ( lp->type )
			{
				case 'W':
					for (; ip; ip = ip->next)
					{
						expValue(ip, seg, symbList, &w, addr);
						hexWriter->outputWord(addr*2, w);
						addr++;
					}
					break;

				case 'R':
					addr += ip->data.val;
					break;

				case 'J':
				case 'K':
					if ( ip )
					{
						item_t *ip1 = ip->next;
						expValue(ip,  seg, symbList, &v0, addr);
						expValue(ip1, seg, symbList, &v1, addr);
						if ( !ip1 || lp->insert )
						{
							int v = (ip1 == NULL)? v0: v1;

							if ( lp->type == 'J' )	// rom page
								w = 0x3180 | ((v >> 8) & 0x7f);
							else 					// ram bank
							{
								int movlb_code = useBSR6? 0x0140: 0x0020;
								int movlb_mask = useBSR6? 0x003f: 0x001f;
								int bank_addr  = (v >= 0x2000)? (v - 0x2000)/80: v >> 7;
								w = movlb_code | (bank_addr & movlb_mask);
							}
							hexWriter->outputWord(addr*2, w);
							addr++;
						}
					}
					break;
			}
		}
	}
	for (Segment *seg = fuseSegGroup; seg; seg = seg->next)
	{
		int addr = seg->addr;
		for (line_t *lp = seg->lines; lp; lp = lp->next)
		{
			item_t *ip = lp->items;
			int item_count = itemCount(ip);
			int w;
			switch ( lp->type )
			{
				case 'W':
					for (; ip; ip = ip->next)
					{
						expValue(ip, seg, symbList, &w, addr);
						hexWriter->outputWord(addr*2, w);
						addr++;
					}
					break;
			}
		}
	}

	hexWriter->close();
	delete hexWriter;
}

void P16link :: outputMap(char *filename, Memory *data)
{
	int usedMemSize, totalMemSize;

	MapWriter *mapWriter = new MapWriter(filename);

	// output RAM map ...
	usedMemSize = dataMem->memUsed(&totalMemSize);
	mapWriter->outputSeg(symbList, usedMemSize, totalMemSize);

	// output data memory map ...
	data->printSpace(mapWriter->fout);

	// output ROM map ...
	usedMemSize = codeMem->memUsed(&totalMemSize);
	mapWriter->outputSeg(codeSegGroup, symbList, usedMemSize, totalMemSize);

	// output calling map ...
	fcallHandler->outputCallPath(mapWriter->fout);

	mapWriter->close();
	delete mapWriter;
}