#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "../common.h"
extern "C" {
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "pic16e.h"
#include "pic16e_asm.h"
#include "pic16e_reg.h"
#include "pic16e_inst.h"

void PIC16E :: mov(Item *ip0, Item *ip1)
{
	if ( same(ip0, ip1) )
		return;

	if ( ip0->attr && ip0->attr->type == SBIT )
	{
		moveToBit(ip0, ip1);
		return;
	}
	if ( ip1->attr && ip1->attr->type == SBIT )
	{
		moveFromBit(ip0, ip1);
		return;
	}
	if ( ip1->type == CON_ITEM || ip1->type == IMMD_ITEM || ip1->type == LBL_ITEM )
	{
		movImmd(ip0, ip1);
		return;
	}
	if ( useFSR(ip0, ip1) )
	{
		movIndir(ip0, ip1);
		return;
	}

	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	if ( size0 > 4 && size1 > 4 )
	{
		movBlock(ip0, ip1);
		return;
	}

	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	for(int i = 0; i < size0; i++)
	{
		if ( i < size1 )
		{
			if ( overlap(ip0, ip1) )
				continue;

			fetch(ip1, i, indf1);

			if ( isWREG(ip0) )		// is WREG target?
				return;
			if ( indf0 ) {
				ASM_CODE(_MOVWI, "INDF0++");
				regFSR0->inc(1);
			}
			else
				ASM_CODE(_MOVWF, acceItem(ip0, i));
		}
		else if ( !SIGNED(ip1) && (indf0 == NULL || size0 == (size1+1)) )
			ASM_CODE(_CLRF, acceItem(ip0, i, indf0));
		else
		{
			if ( i == size1 )
			{
				if ( SIGNED(ip1) )
				{
					if ( overlap(ip0, ip1) )
						fetch(ip1, size1-1, indf1);

					EXTEND_WREG;
				}
				else
					regWREG->load(0);
			}
			if ( indf0 ) {
				ASM_CODE(_MOVWI, "INDF0++");
				regFSR0->inc(1);
			}
			else
				ASM_CODE(_MOVWF, acceItem(ip0, i));
		}
	}
}

void PIC16E :: movImmd(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	char *indf = setFSR0(ip0);

	for (int i = 0; i < size0; i++)
	{
		if ( ip1->type == CON_ITEM )	// set a solid constant
		{
			int n = ip1->val.i >> (i*8);
			if ( n & 0xff )
			{
				regWREG->load(n & 0xff);
				if ( indf ) {
					ASM_CODE(_MOVWI, "INDF0++");
					regFSR0->inc(1);
				}
				else if ( !isWREG(ip0) )
					ASM_CODE(_MOVWF, acceItem(ip0, i));
			}
			else if ( indf )
			{
				regWREG->load(0);
				ASM_CODE(_MOVWI, "INDF0++");
				regFSR0->inc(1);
			}
			else
			{
				ASM_CODE(_CLRF, acceItem(ip0, i, indf));
			}
		}
		else if ( i < 2 )				// set immediate value
		{
			ASM_CODE(_MOVLW, acceItem(ip1, i));
			regWREG->reset();	// reset WREG unconditionally!
								// (don't know real value)
			if ( isWREG(ip0) )
				continue;
			else if ( indf ) {
				ASM_CODE(_MOVWI, "INDF0++");
				regFSR0->inc(1);
			}
			else
				ASM_CODE(_MOVWF, acceItem(ip0, i));
		}
		else							// fill zero value
		{
			ASM_CODE(_CLRF, acceItem(ip0, i, indf));
			if ( indf && (i+1) < size0 )
				regFSR0->inc(1, true);
		}
	}
}

void PIC16E :: movIndir(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();

	if ( size0 == 1 || size1 == 1 )
	{
		char *indf = setFSR0(ip1);
		fetch(ip1, 0, indf);
		ASM_CODE(_MOVWF, ACC0);
		indf = setFSR0(ip0);
		ASM_CODE(_MOVF, ACC0, _W_);
		store(ip0, 0, indf);
		regWREG->reset();

		for (int i = 1; i < size0; i++)
		{
			if ( !SIGNED(ip1) && (i+1) == size0 )
			{
				ASM_CODE(_CLRF, INDF0);
				return;
			}
			if ( i == 1 )
			{
				regWREG->load(0);
				if ( SIGNED(ip1) )
				{
					ASM_CODE(_BTFSC, ACC0, 7);
					ASM_CODE(_MOVLW, 0xff);
				}
			}
			store(ip0, i, indf);
		}
		return;
	}

	if ( size0 <= 4 && size1 <= 4 )
	{
		Item *acc = storeToACC(ip1, size0);
		mov(ip0, acc);
		delete acc;
		return;
	}

	movBlock(ip0, ip1);
}

void PIC16E :: movBlock(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int n = -size0;
	if ( size0 == 0 ) return;
	
	call((char*)"_saveFSR1");
	regWREG->reset();

	setFSR(ip1, 1);
	setFSR(ip0, 0);
	Item *lbl = lblItem(pcoder->getLbl());

	ASM_CODE(_MOVLW, n & 0xff);
	ASM_CODE(_MOVWF, 0x70);
	if ( size0 >= 256 )
	{
		ASM_CODE(_MOVLW, (n>>8) & 0xff);
		ASM_CODE(_MOVWF, 0x71);
	}
	ASM_LABL(lbl->val.s);
	ASM_CODE(_MOVIW, "INDF1++");
	ASM_CODE(_MOVWI, "INDF0++");
	ASM_CODE(_INCFSZ, 0x70, _F_);
	regFSR0->inc(1);	
	if ( size0 >= 256 )
	{
		ASM_CODE(_BRA, lbl->val.s);
		ASM_CODE(_INCFSZ, 0x71, _F_);
	}
	ASM_CODE(_BRA, lbl->val.s);
	delete lbl;
	call((char*)"_restoreFSR1");
}

void PIC16E :: moveToBit(Item *ip0, Item *ip1)
{
	if ( ip1->attr->type == SBIT )
	{
		moveBetweenBits(ip0, ip1);
		return;
	}

	int addr0 = (ip0->val.i >> 3) & 0x7f;
	if ( ip1->type == CON_ITEM )
	{
		regBSR->load(ip0->val.i >> 3);
		if ( ip1->val.i )
			ASM_CODE(_BSF, addr0, ip0->val.i&7);
		else
			ASM_CODE(_BCF, addr0, ip0->val.i&7);
	}
	else if ( ip1->type == IMMD_ITEM )
	{
		regBSR->load(ip0->val.i >> 3);
		ASM_CODE(_BSF, addr0, ip0->val.i&7);
	}
	else
	{
		char *indf = setFSR0(ip1);
		int size1 = ip1->acceSize();
		for (int i = 0; i < size1; i++)
		{
			if ( i == 0 )
				ASM_CODE(_MOVF, acceItem(ip1, i, indf), _W_);
			else
				ASM_CODE(_IORWF, acceItem(ip1, i, indf), _W_);

			if ( indf && (i+1) < size1 )
				regFSR0->inc(1, true);
		}

		Item *false_lbl = lblItem(pcoder->getLbl());
		Item *end_lbl   = lblItem(pcoder->getLbl());

		regBSR->load(ip0->val.i >> 3);
		ASM_CODE(_BTFSC, aSTATUS, 2);	// Z flag set?
		ASM_CODE(_BRA, false_lbl->val.s);
		ASM_CODE(_BSF, addr0, ip0->val.i & 7);
		ASM_CODE(_BRA, end_lbl->val.s);
		ASM_LABL(false_lbl->val.s);
		ASM_CODE(_BCF, addr0, ip0->val.i & 7);
		ASM_LABL(end_lbl->val.s);
		delete false_lbl;
		delete end_lbl;
	}
}

void PIC16E :: moveFromBit(Item *ip0, Item *ip1)
{
	char *indf = setFSR0(ip0);
	regWREG->load(0);
	ASM_CODE(_BTFSC, acceItem(ip1, 0, NULL));
	ASM_CODE(_MOVLW, 1);
	ASM_CODE(_MOVWF, acceItem(ip0, 0, indf));
	regWREG->reset();
	for(int i = 1; i < ip0->acceSize(); i++)
		ASM_CODE(_CLRF, acceItem(ip0, i, indf));
}

void PIC16E :: moveBetweenBits(Item *ip0, Item *ip1)
{
	Item *false_lbl = lblItem(pcoder->getLbl());
	Item *end_lbl   = lblItem(pcoder->getLbl());
	int addr0 = (ip0->val.i >> 3) & 0x7f;
	int addr1 = (ip1->val.i >> 3) & 0x7f;

	regBSR->load(ip1->val.i >> 3);
	if ( (ip0->val.i >> 10) != (ip1->val.i >> 10) )
	{	// bits not in the same RAM bank
		ASM_CODE(_MOVF, addr1, _W_);
		regBSR->load(ip0->val.i >> 3);
		ASM_CODE(_BTFSS, WREG, ip1->val.i & 7);
		regWREG->reset();
	}
	else
		ASM_CODE(_BTFSS, addr1, ip1->val.i & 7);

	ASM_CODE(_BRA, false_lbl->val.s);
	ASM_CODE(_BSF, addr0, ip0->val.i & 7);
	ASM_CODE(_BRA, end_lbl->val.s);
	ASM_LABL(false_lbl->val.s);
	ASM_CODE(_BCF, addr0, ip0->val.i & 7);
	ASM_LABL(end_lbl->val.s);
	delete false_lbl;
	delete end_lbl;
}
