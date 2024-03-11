#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
extern "C" {
#include "../common.h"
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "pic16e.h"
#include "pic16e_asm.h"
#include "pic16e_reg.h"
#include "pic16e_inst.h"

static int totalParSize;
static int totalTmpSize;
static int totalVarSize;

static void tmpCollect(Pnode *head);
static int tmpMaxSize(Pnode *head, int index);
static int listData(Fnode *fp, Item **list, Pnode *pnp);

void PIC16E :: funcBeg(Pnode *pnp)
{
	char *fname = curFunc->name;
	bool is_isr = ISR_FUNC(curFunc);
	char func_dname[4096];
	char *s = STRBUF();

	ASM_OUTP("\n");

	// make local data index name...
	sprintf(func_dname, "%s_$data$", fname);

	// it's the 'main' function?
	if ( strcmp(fname, "main") == 0 )
	{
		// reset vector init.
		ASM_CODE(_SEGMENT, "CODE0 (ABS, =0x0000)");
		ASM_CODE(_NOP);
		ASM_CODE(_MOVLP, "main >> 8");
		ASM_CODE(_GOTO, "main");
		ASM_OUTP("\n");

		// create static data init. link
		ASM_CODE(_SEGMENT, "CODEi (REL, BEG)");
		ASM_LABL((char*)"_$init$", true);
		ASM_CODE(_CLRF, ZERO_LOC);	// ZERO_LOC := b'00000000
		ASM_CODE(_CLRF, FLAG_LOC);	// bit0: FSR1_SAVED, bit1: ISR_LOCK
		ASM_OUTP("\n");

		ASM_CODE(_SEGMENT, "CODEi (REL, END)");
		ASM_CODE(_RETURN);
		ASM_OUTP("\n");

		ASM_CODE(_SEGMENT, "ISR (REL, BEG)");
		ASM_CODE(_CLRF, PCLATH);	// set up PCLATH
		// get stack pointer if needed
		if ( isrStackSet )
		{
			ASM_CODE(_BTFSS, FLAG_LOC, 0);
			ASM_CODE(_BRA, ".+5");
			ASM_CODE(_MOVF, BUFFERED_FSR1L, _W_);
			ASM_CODE(_MOVWF, FSR1L);
			ASM_CODE(_MOVF, BUFFERED_FSR1H, _W_);
			ASM_CODE(_MOVWF, FSR1H);
			ASM_OUTP("\n");
		}
		for (int i = 0; i < accSave; i++)
		{
			ASM_CODE(_MOVF, 0x70+i, _W_);
			ASM_CODE(_MOVWI, "--INDF1");
		}
		ASM_OUTP("\n");

		ASM_CODE(_SEGMENT, "ISR (REL, END)");
		for (int i = accSave; i--;)
		{
			ASM_CODE(_MOVIW, "INDF1++");
			ASM_CODE(_MOVWF, 0x70+i);
		}
		ASM_CODE(_RETFIE);
		ASM_OUTP("\n");
	}

	for (int sequence = 0;;)
	{
		int offset, depth = sequence++;
		Dnode *dnp = curFunc->getData(STATIC_DATA, &depth, &offset);
		if ( dnp == NULL ) break;

		ASM_CODE(_SEGMENT, "BANKi (REL)");
		sprintf(s, "%s\t%d\n", _RS, dnp->size());
		ASM_LABL(dnp->nameStr(), false, s);
	}

	Item *list = NULL;
	int data_size = listData(curFunc, &list, pnp);

	if ( is_isr )
	{
		sprintf(s, "ISR (REL) %s:%d", fname, data_size);
		regPCLATH->set(0);
	}
	else
	{
		sprintf(s, "CODE2 (REL) %s:%d", fname, data_size);
		regPCLATH->set(fname);
	}
	ASM_CODE(_SEGMENT, s);

	// output function local variables...
	while ( list )
	{
		ASM_OUTP(list->val.s);
		Item *tmp = list->next;
		delete list; list = tmp;
	}

	if ( !is_isr )
		ASM_LABL(fname, !STATIC_FUNC(curFunc));

	regBSR->reset();
	regWREG->reset();

	if ( totalParSize > 0 )
	{	// get function parameter from stack (pointed by FSR1)
		s = STRBUF();
		if ( totalParSize <= 2 )	// simple move
		{
			regBSR->load(func_dname);
			for (int i = totalParSize; i--;)
			{
				ASM_CODE(_MOVIW, "INDF1++");
				sprintf(s, "%s+%d", func_dname, i);
				ASM_CODE(_MOVWF, s);
			}
		}
		else						// using system lib routine
		{
			sprintf(s, "%s+%d", func_dname, totalParSize);
			ASM_CODE(_MOVLW, s);
			ASM_CODE(_MOVWF, FSR0L);
			sprintf(s, "(%s+%d)>>8", func_dname, totalParSize);
			ASM_CODE(_MOVLW, s);
			ASM_CODE(_MOVWF, FSR0H);
			ASM_CODE(_MOVLB, totalParSize);
			call((char*)"_copyPar");
			regBSR->set(0);
		}
	}

	if ( strcmp(fname, "main") == 0 )
	{
		call((char*)"_$init$");

		// init. stack pointer
		ASM_CODE(_MOVLW, "_$$"); 	ASM_CODE(_MOVWF, FSR1L);
		ASM_CODE(_MOVLW, "_$$>>8");	ASM_CODE(_MOVWF, FSR1H);
	}
}

void PIC16E :: funcEnd(void)
{
	if ( ISR_FUNC(curFunc) )
		regPCLATH->load(0);
	else
	{
		regPCLATH->load(curFunc->name);
		ASM_CODE(_RETURN);
	}

	// list all function called ...
	if ( curFunc->fcall ) ASM_LABL((char*)"; function(s) called:");
	for (NameList *fcall = curFunc->fcall; fcall; fcall = fcall->next)
		ASM_CODE(_FCALL, curFunc->name, fcall->name);
}

/////////////////////////////////////////////////////////////////////////////////
static unsigned int tempIndexMask[4];

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
				int isize = sizer(ip->attr, ATTR_SIZE);

				if ( isize > size )
					size = isize;
			}
		}
		head = head->next;
	}

	return size;
}

static void addItem(Item **list, Item *ip)
{
	if ( *list == NULL )
		*list = ip;
	else
	{
		Item *ihead = *list;
		while ( ihead->next ) ihead = ihead->next;
		ihead->next = ip;
	}
}

static int listData(Fnode *fp, Item **list, Pnode *pnp)
{
	char buf[1024];

	totalParSize = 0;
	totalVarSize = 0;
	totalTmpSize = 0;

	// list all parameter variables
	for (Dnode *dnp = fp->dlink->dlist; dnp; dnp = dnp->next)
	{
		sprintf(buf, "%s:\t%s\t%s_$data$+%d\n",
				dnp->nameStr(), _EQU, fp->name, totalParSize);

		addItem(list, strItem(buf));
		totalParSize += dnp->size();
	}

	// list all non-static variables
	for (int sequence = 0;;)
	{
		int offset, depth = sequence++;
		Dnode *dnp = fp->getData(GENERIC_DATA, &depth, &offset);
		if ( dnp == NULL ) break;

		sprintf(buf, "%s:\t%s\t%s_$data$+%d\n",
				dnp->nameStr(), _EQU, fp->name, totalParSize + offset);

		addItem(list, strItem(buf));
		offset += dnp->size();

		if ( totalVarSize < offset ) totalVarSize = offset;
	}

	// list all temp variables
	tmpCollect(pnp);
	for (int i = 0; i < (4*32); i++)
	{
		if ( tempIndexMask[i/32] & (1 << (i%32)) )
		{
			sprintf(buf, "%s_$%d:\t%s\t%s_$data$+%d\n",
					fp->name, i+1, _EQU, fp->name,  totalParSize+
													totalVarSize+
													totalTmpSize);
			addItem(list, strItem(buf));
			totalTmpSize += tmpMaxSize(pnp, i);
		}
	}

	return totalParSize + totalVarSize + totalTmpSize;
}

void PIC16E :: call(char *funcname)
{
	regPCLATH->load(funcname);
	ASM_CODE(_CALL, funcname);
	addName(&curFunc->fcall, funcname);
	regWREG->reset();
}