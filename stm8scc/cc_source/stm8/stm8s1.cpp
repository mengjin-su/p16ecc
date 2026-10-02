#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
extern "C" {
#include "../common.h"
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../const.h"
#include "../nlist.h"
#include "../option.h"
#include "../prescan.h"
#include "stm8s.h"
#include "stm8s_reg.h"
#include "stm8s_inst.h"
#include "stm8s_asm.h"

static int totalParSize;
static int totalTmpSize;
static int totalVarSize;
static unsigned int tempIndexMask[4];

static int tmpMaxSize(Pnode *head, int index);
static void tmpCollect(Pnode *head);

void STM8S :: funcBeg(Pnode *pnp)
{
	char *fname = curFunc->name;
	char func_dname[4096];
	char *s = STRBUF();
	attrib *attr = curFunc->attr;

//	ASM_OUTP('\n');
	sprintf(func_dname, "%s_$data$", fname);

	// allocate function static variables...
	for (int sequence = 0;;)
	{
		int offset, seq = sequence++;
		Dnode *dnp = curFunc->getData(STATIC_DATA, &seq, &offset);
		if ( dnp == NULL ) {
			if ( sequence > 0 )	ASM_OUTP('\n');
			break;
		}
		if ( dnp->attr && dnp->attr->dataBank == BANK0 )
			ASM_CODE1(_SEGMENT, "BANK0 (REL)");
		else
			ASM_CODE1(_SEGMENT, "BANKi (REL)");
		sprintf(s, "%s:\t%s\t%d\n", dnp->nameStr(), _RS, dnp->size());
		ASM_OUTP(s);
	}

	// it's the 'main' function?
	if ( strcmp(fname, "main") == 0 )
	{
		// reset vector init.
		ASM_CODE1(_SEGMENT, "CODE0 (ABS, =0x8000)");
		ASM_CODE1(_VECT, fname);
		ASM_OUTP('\n');

		// create static data init. link
		ASM_CODE1(_SEGMENT, "CODEi (REL, BEG)");
		ASM_OUTP("_$init$:\n");
		ASM_OUTP_LN();

		ASM_CODE1(_SEGMENT, "CODEi (REL, END)");
		ASM_CODE0(EXT_MODE? _RETF: _RET);
		ASM_OUTP_LN();
	}
	else if ( attr->type == INTERRUPT )
	{
		char *s = STRBUF();
		sprintf(s, "CODE0 (ABS, =0x%X)", vectorAddr(attr));
		ASM_CODE1(_SEGMENT, s);
		ASM_CODE1(_VECT, fname);
		ASM_OUTP_LN();
	}

	int data_size = listData(curFunc, pnp);

	ASM_CODE1(_SEGMENT, "CODE2 (REL)");
	ASM_LABL(fname, true);
	if ( !attr->isStatic && attr->type != INTERRUPT ) ASM_OUTP(':');
	ASM_OUTP_LN();

	if ( strcmp(fname, "main") == 0 )
	{
		// set Stack Poiner(SP), and clean up whole RAM...
		ASM_CODE2(_LDW, REG_X, stackAddr, '#');
		ASM_CODE2(_LDW, REG_SP, REG_X);
		ASM_CODE1(_CLR, "(%X)");
		ASM_CODE1(_DECW, REG_X);
		ASM_CODE1(_JRPL, ".-4");

		// call system init routine
		ASM_CODE1(EXT_MODE? _CALLF: _CALL, (char*)"_$init$");
	}
	else if ( attr->type == INTERRUPT )
	{
		for (int addr = 0; addr < accSave; addr++)
		{
			if ( (accSave - addr) >= 2 )
			{
				ASM_CODE2(_LDW, REG_X, addr);	// save ACC contents
				ASM_CODE1(_PUSHW, REG_X);
				addr++;
			}
			else
				ASM_CODE1(_PUSH, addr);
		}
	}

	switch ( data_size )
	{	// create local memory on stack
		case 0:	break;
		case 1: ASM_CODE1(_PUSH, REG_A);	break;
		case 2: ASM_CODE1(_PUSHW, REG_X);	break;
		default:ASM_CODE2(_SUB, REG_SP, data_size, '#');
	}
	frameOffset = 0;
}

void STM8S :: funcEnd(void)
{
	int data_size = totalTmpSize + totalVarSize;
	switch ( data_size )
	{	// remove local memory on stack (if any)
		case 0:	break;
		case 1: ASM_CODE1(_POP, REG_A);		break;
		case 2: ASM_CODE1(_POPW, REG_X);	break;
		default:ASM_CODE2(_ADD, REG_SP, data_size, '#');
	}

	if ( curFunc->attr->type == INTERRUPT )
	{
		for (int addr = accSave; addr > 0;)
		{
			if ( addr & 1 )
			{
				addr--;
				ASM_CODE1(_POP, addr);		// restore ACC contentsa
			}
			else
			{
				addr -= 2;
				ASM_CODE1(_POPW, REG_X);
				ASM_CODE2(_LDW, addr, REG_X);
			}
		}
		ASM_CODE0(_IRET);
	}
	else if ( strcmp(curFunc->name, "main") == 0 )
	{
		ASM_OUTP("_$iret$::\n");
		ASM_CODE0(_IRET);
	}
	else
		ASM_CODE0(EXT_MODE? _RETF: _RET);

	// list all function called ...
	if ( curFunc->fcall )
		ASM_OUTP((char*)"; function(s) called:\n");

	for (NameList *fcall = curFunc->fcall; fcall; fcall = fcall->next)
		ASM_CODE2((char*)".fcall", curFunc->name, fcall->name);

	delLocalData(&localDataList);
}

int STM8S :: listData(Fnode *fp, Pnode *pnp)
{
	char *fname = fp->name;
	char buf[1024];
	//fp->dlink->display(0); // debug display ...

	// list all non-static variables
	int final_offset = 0;
	for (int sequence = 0;;)
	{
		int offset, seq = sequence++;
		Dnode *dnp = fp->getData(GENERIC_DATA, &seq, &offset);
		if ( dnp == NULL ) break; // finish searching...

		sprintf(buf, "; %s: offset = %d\n", dnp->nameStr(), offset+1);
		ASM_OUTP(buf);
		addLocalData(&localDataList, dnp->nameStr(), offset+1);
		offset += dnp->size();

		if ( final_offset < offset )
			final_offset = offset;
	}
	totalVarSize = final_offset;

	// list all temp variables
	totalTmpSize = 0;
	tmpCollect(pnp);
	for (int i = 0; i < (int)sizeof(tempIndexMask)*8; i++)
	{
		if ( tempIndexMask[i/32] & (1 << (i%32)) )
		{
			int offset = final_offset+1;
			sprintf(buf, "; %s_$%d: offset = %d\n", fname, i+1, offset);
			ASM_OUTP(buf);

			sprintf(buf, "%s_$%d", fname, i+1);
			addLocalData(&localDataList, buf, offset);

			totalTmpSize += tmpMaxSize(pnp, i);
			final_offset += tmpMaxSize(pnp, i);
		}
	}

	final_offset += EXT_MODE? 3: 2;
	totalParSize = 0;
	// list all parameter variables
	for (Dnode *dnp = fp->dlink->dlist; dnp; dnp = dnp->next)
	{
		int offset = final_offset+1;
		sprintf(buf, "; %s: offset = %d\n", dnp->nameStr(), offset);
		ASM_OUTP(buf);

		addLocalData(&localDataList, dnp->nameStr(), offset);

		totalParSize += dnp->size();
		final_offset += dnp->size();
	}

	return /*totalParSize + */totalVarSize + totalTmpSize;
}

int STM8S :: vectorAddr(attrib *attr)
{
//	PreScan prescan(nlist);
//	attr->atAddr = prescan.scan((node*)attr->atAddr);

	conNode_t *np = (conNode_t*)attr->atAddr;
	return (np->type == NODE_CON)? np->value: -1;
}

static void tmpCollect(Pnode *head)
{
	memset(tempIndexMask, 0, sizeof(tempIndexMask));

    while ( head && head->type != P_FUNC_END )
    {
        for (int i = 0; i < 3; i++)
        {
            Item *ip = head->items[i];
            if ( ip && ip->type == TEMP_ITEM )
			{
				int I = ip->val.i;
				if ( I < (32*4) )
					tempIndexMask[I/32] |= 1 << (I%32);
			}
        }
        head = head->next;
    }
}

static int tmpMaxSize(Pnode *head, int index)
{
	int size = 0;
    while ( head && head->type != P_FUNC_END )
    {
        for (int i = 0; i < 3; i++)
        {
            Item *ip = head->items[i];
            if ( ip && ip->val.i == index &&
				 (ip->type == TEMP_ITEM || ip->type == INDIR_ITEM) )
			{
				int isize = sizer.size(ip->attr, ATTR_SIZE);

				if ( isize > size )
					size = isize;
			}
		}
		head = head->next;
	}

	return size;
}

void addLocalData(LocalData **list, char *name, int offset)
{
	LocalData *data = new LocalData;
	data->name = dupStr(name);
	data->offset = offset;
	data->next = *list;
	*list = data;
}

void delLocalData(LocalData **list)
{
	while ( *list != NULL )
	{
		LocalData *tmp = (*list)->next;
		free((*list)->name);
		delete *list;
		*list = tmp;
	}
}

LocalData *getLocalData(LocalData *list, char *name)
{
	for (; list ; list = list->next) {
		if ( strcmp(list->name, name) == 0 )
			return list;

		char *p = name;
		while ( *p == '(' ) p++;
		int len = strlen(list->name);
		if ( memcmp(p, list->name, len) == 0 )
		{
			if ( p[len] == '+' )
				return list;
		}
	}
	return NULL;
}
