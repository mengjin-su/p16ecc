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

void PIC16E :: leftAssign(Item *ip0, Item *ip1)
{
	if ( ip1->type == CON_ITEM )
	{
		leftAssignConst(ip0, ip1->val.i);
		return;
	}

	char *indf1 = setFSR0(ip1);
	char *shiftCount = WREG;
	if ( CONST_ID_ITEM(ip1) )
	{
		fetch(ip1, 0, indf1);
		ASM_CODE(_ADDLW, 1);
	}
	else
		ASM_CODE(_INCF, acceItem(ip1, 0, indf1), _W_);

	regWREG->reset();
	if ( useFSR(ip0) )
		ASM_CODE(_MOVWF, shiftCount = ACC0);

	char *indf0 = setFSR0(ip0);
	int size0 = ip0->acceSize();
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());
	char *shift_inst = _LSLF;
	int  offset = regFSR0->getVarOffset();
	if ( !indf0 ) regBSR->reset(ip0);

	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s);

	for (int i = 0; i < size0; i++, shift_inst = _RLF)
		ASM_CODE(shift_inst, acceItem(ip0, i, indf0), _F_);

	if ( indf0 )
		regFSR0->restore(offset);	// reset FSR0 offset

	ASM_LABL(endLbl->val.s);
	ASM_CODE(_DECFSZ, shiftCount, _F_);
	ASM_CODE(_BRA, startLbl->val.s);
	delete startLbl;
	delete endLbl;
}

void PIC16E :: leftAssignConst(Item *ip0, int n)
{
	int size0 = ip0->acceSize();
	if ( n > (size0*8) ) n = (size0*8);
	int byte_shift = n/8;
	int bit_shift  = n%8;
	char *indf0 = setFSR0(ip0);

	if ( byte_shift > 0 )
	{
		for (int i = size0; i--;)
		{
			if ( i >= byte_shift )
			{
				fetch(ip0, i-byte_shift, indf0);
				store(ip0, i, indf0);
				regWREG->reset();
			}
			else if ( indf0 && byte_shift > 1 )
			{
				regWREG->load(0);
				store(ip0, i, indf0);
			}
			else
				ASM_CODE(_CLRF, acceItem(ip0, i, indf0));
		}
	}

	if ( bit_shift > 0 )
	{
		if ( (size0 - byte_shift) == 1 && bit_shift >= 4 )
		{
			ASM_CODE(_SWAPF, acceItem(ip0, size0-1, indf0), _W_);
			ASM_CODE(_ANDLW, 0xf0);
			ASM_CODE(_MOVWF, acceItem(ip0, size0-1, indf0));
			bit_shift -= 4;
			regWREG->reset();
		}

		Item *startLbl = lblItem(pcoder->getLbl());
		int  offset = regFSR0->getVarOffset();
		int  repeat = bit_shift;
		if ( !indf0 ) regBSR->reset(ip0);
		
		if ( bit_shift > 1 && (size0 - byte_shift) > 1 )
		{
			regWREG->load(bit_shift);
			ASM_LABL(startLbl->val.s);
			repeat = 1;
		}

		while ( repeat-- )
			for (int i = byte_shift; i < size0; i++)
			{
				char *inst = (i == byte_shift)? _LSLF: _RLF;
				ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);
			}

		if ( bit_shift > 1 && (size0 - byte_shift) > 1 )
		{
			if ( indf0 ) regFSR0->restore(offset);	// reset FSR0 offset
			ASM_CODE(_DECFSZ, WREG, _F_);
			ASM_CODE(_BRA, startLbl->val.s);
			regWREG->set(0);
		}
		delete startLbl;
	}
}

void PIC16E :: rightAssign(Item *ip0, Item *ip1, bool sshift)
{
	if ( ip1->type == CON_ITEM )
	{
		rightAssignConst(ip0, ip1->val.i, sshift);
		return;
	}

	char *indf1 = setFSR0(ip1);
	char *shiftCount = WREG;
	if ( CONST_ID_ITEM(ip1) )
	{
		fetch(ip1, 0, indf1);
		ASM_CODE(_ADDLW, 1);
	}
	else
		ASM_CODE(_INCF, acceItem(ip1, 0, indf1), _W_);

	regWREG->reset();
	if ( useFSR(ip0) )
		ASM_CODE(_MOVWF, shiftCount = ACC0);

	char *indf0 = setFSR0(ip0);
	int  size0  = ip0->acceSize();
	int  offset = regFSR0->getVarOffset();
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());

	if ( !indf0 ) regBSR->reset(ip0);
	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s);

	for (int i = size0; i--;)
	{
		if ( i == (size0-1) && sshift )
			ASM_CODE(_ASRF, acceItem(ip0, i, indf0), _F_);
		else if ( i == (size0-1) )
			ASM_CODE(_LSRF, acceItem(ip0, i, indf0), _F_);
		else
			ASM_CODE(_RRF,  acceItem(ip0, i, indf0), _F_);
	}

	if ( indf0 ) regFSR0->restore(offset);
	ASM_LABL(endLbl->val.s);
	ASM_CODE(_DECFSZ, shiftCount, _F_);
	ASM_CODE(_BRA, startLbl->val.s);
	delete startLbl;
	delete endLbl;
}

void PIC16E :: rightAssignConst(Item *ip0, int n, bool sshift)
{
	int size0 = ip0->acceSize();
	char *indf0 = setFSR0(ip0);
	if ( n > (size0*8) ) n = (size0*8);
	int byte_shift = n/8;
	int bit_shift  = n%8;

	if ( byte_shift > 0 )
		for (int i = 0; i < size0; i++)
		{
			if ( (i+byte_shift) < size0 )
			{
				fetch(ip0, i+byte_shift, indf0);
				store(ip0, i, indf0);
				regWREG->reset();
			}
			else if ( sshift )	// signed shift
			{
				if ( (i+byte_shift) == size0 )
				{
					if ( byte_shift >= size0 ) fetch(ip0, size0-1, indf0);
					EXTEND_WREG;
				}
				store(ip0, i, indf0);
			}
			else
				ASM_CODE(_CLRF, acceItem(ip0, i, indf0));
		}

	if ( bit_shift > 0 )
	{
		if ( (size0 - byte_shift) == 1 && bit_shift >= 4 && !sshift )
		{
			ASM_CODE(_SWAPF, acceItem(ip0, 0, indf0), _W_);
			ASM_CODE(_ANDLW, 0x0f);
			ASM_CODE(_MOVWF, acceItem(ip0, 0, indf0));
			bit_shift -= 4;
			regWREG->reset();
		}

		Item *startLbl = lblItem(pcoder->getLbl());
		int offset = regFSR0->getVarOffset();
		int repeat = bit_shift;
		if ( !indf0 ) regBSR->reset(ip0);

		if ( bit_shift > 1 && (size0 - byte_shift) > 1 )
		{
			regWREG->load(bit_shift);
			ASM_LABL(startLbl->val.s);
			repeat = 1;
		}

		while ( repeat-- )
			for (int i = size0 - byte_shift; i--; )
			{
				char *shift_inst = sshift? _ASRF: _LSRF;
				if ( (i+1) == (size0-byte_shift) )	// highest byte
					ASM_CODE(shift_inst, acceItem(ip0, i, indf0), _F_);
				else
					ASM_CODE(_RRF,  acceItem(ip0, i, indf0), _F_);
			}

		if ( bit_shift > 1 && (size0 - byte_shift) > 1 )
		{
			if ( indf0 ) regFSR0->restore(offset);	// reset FSR0 offset
			ASM_CODE(_DECFSZ, WREG, _F_);
			ASM_CODE(_BRA, startLbl->val.s);
			regWREG->set(0);
		}
		delete startLbl;
	}
}