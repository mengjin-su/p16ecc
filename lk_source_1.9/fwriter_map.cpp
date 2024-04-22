#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "fwriter_map.h"
#include "exp.h"
#include "disasm.h"

MapWriter :: MapWriter(char *filename)
{
	fout = fopen(filename, "w");
	if ( fout == NULL ) fout = stdout;
	fprintf(fout, ">>>>>>>>>>>> MAP OUTPUT <<<<<<<<<<<<\n\n");
}

void MapWriter :: close(void)
{
	if ( fout ) fclose(fout);
}

void MapWriter :: outputSeg(Symbol *slist, int used, int total)
{
	float pct = (float)used*100/(float)total;
	fprintf(fout, "- DATA MEMORY %d bytes(%.2f%c) used\n", used, pct, '%');

	for (; slist; slist = slist->next)
	{
		Segment *seg = slist->segment;
		char *fname = seg->lines->fname;	// file name
		char *name  = slist->name;			// symbol name
		int  laddr;
			
		if ( seg->type() == DATA_SEGMENT &&
			 !strstr(name, "$sizeof$")   &&
			 !strstr(name, "$init$")      )
		{
			int size = seg->size();
			int addr = slist->value;

			if ( addr >= 0x2000 )				// in linear space..
			{
				laddr = addr;
				addr  = ((addr&0x1fff)/80)*128 + 0x20 + (addr&0x1fff)%80;
			}
			else if ( (addr & 0x7f) < 0x20 )	// in register space..
			{
				laddr = addr;
			}
			else if ( (addr & 0x7f) < 0x70 )	// in generic RAM space..
			{
				laddr = (addr>>7)*80 + (addr&0x7f) - 0x20 + 0x2000;
			}

			fprintf(fout, "%s  %s (%d): 0x%X", fname, name, size, laddr);
			fprintf(fout, " [%d:0x%02X]\n", addr>>7, addr&0x7f);
		}
		
		if ( seg->type() == CODE_SEGMENT && seg->dataSize && slist->type == 'U' && strstr(name, "_$data$") )
		{
			int size = seg->dataSize;
			int addr = slist->value;

			if ( addr >= 0x2000 )
			{
				laddr = addr;
				addr  = ((addr&0x1fff)/80)*128 + 0x20 + (addr&0x1fff)%80;
			}
			else
			{
				laddr = (addr>>7)*80 + (addr&0x7f) - 0x20 + 0x2000;
			}
			fprintf(fout, "%s  %s (%d): 0x%X", fname, name, size, laddr);
			fprintf(fout, " [%d:0x%02X]\n", addr>>7, addr&0x7f);
		}
	}
}

void MapWriter :: outputSeg(Segment *seg, Symbol *slist, int used, int total)
{
	float pct = (float)used*100/(float)total;
	fprintf(fout, "\n- CODE MEMORY %d words(%.2f%c) used\n", used, pct, '%');

	for (; seg; seg = seg->next)
	{
		int addr = seg->memAddr;
		int type = seg->type();

		if ( type == CODE_SEGMENT || type == CONST_SEGMENT )
			addr &= 0x7fff;

		for (line_t *lp = seg->lines; lp; lp = lp->next)
		{
			item_t *ip = lp->items;
			int v;
			switch ( lp->type )
			{
				case 'S':	// segment..
					outputSkip0();
					fprintf(fout, "(%s line#%d, %d words)\n",
								   seg->fileName, lp->lineno, seg->size());
					break;

				case 'G':
					outputSkip0();
					fprintf(fout, "%s::\n", ip->data.str);
					break;

				case 'L':
					outputSkip0();
					fprintf(fout, "%s:\n", ip->data.str);
					break;

				case ';':
					if ( !seg->isLIB )
					{
						outputSkip();
						fprintf(fout, "%s\n", ip->data.str);
					}
					break;

				case 'W':
					for (; ip; ip = ip->next, addr++)
					{
						expValue(ip, seg, slist, &v, addr);
						outputInst(addr, v);
					}
					break;

				case 'R':
					addr += seg->size();
					break;

				case 'J':	// page select
				case 'K':	// bank select
					if ( lp->insert || itemCount(ip) == 1 )
					{
						if ( itemCount(ip) == 1 )
							expValue(ip, seg, slist, &v, addr);
						else
							expValue(ip->next, seg, slist, &v, addr);

						if ( lp->type == 'J' )	// rom page select
							outputInst(addr, 0x3180|((v >> 8) & 0x7f));
						else 					// ram bank select
						{
							int movlb_code = useBSR6? 0x0140: 0x0020;
							int movlb_mask = useBSR6? 0x003f: 0x001f;
							int bank_addr  = (v >= 0x2000)? (v - 0x2000)/80: v >> 7;
							outputInst(addr, movlb_code | (bank_addr & movlb_mask));
						}
						addr++;
					}
					break;
			}
		}
	}
}

void MapWriter :: outputSkip(void)
{
	fprintf(fout, "          \t\t");
}

void MapWriter :: outputSkip0(void)
{
	fprintf(fout, "          \t");
}

void MapWriter :: outputInst(int addr, int inst)
{
	fprintf(fout, "%04X: %04X\t\t", addr, inst);
	fprintf(fout, "%s\n", p16disasm(addr, inst));
}