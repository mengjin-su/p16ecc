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
		leftAssignConst(ip0, ip1);
		return;
	}

	char *shiftCount = NULL;
	char *indf1;
	if ( ip1->type == ACC_ITEM || ip1->type == TEMP_ITEM )
	{
		shiftCount = acceItem(ip1, 0);
		ASM_CODE(_INCF, shiftCount, _F_);
	}
	else if ( useFSR(ip0, ip1) )
	{
		indf1 = setFSR0(ip1);
		ASM_CODE(_INCF, acceItem(ip1, 0, indf1), _W_);
		ASM_CODE(_MOVWF, ACC0);
		shiftCount = ACC0;
		regWREG->reset();
	}

	char *indf0 = setFSR0(ip0);
	if ( shiftCount == NULL )
	{
		indf1 = setFSR0(ip1);
		if ( CONST_ID_ITEM(ip1) )
		{
			fetch(ip1, 0, indf1);
			ASM_CODE(_ADDLW, 1);
		}
		else
			ASM_CODE(_INCF, acceItem(ip1, 0, indf1), _W_);

		shiftCount = WREG;
		regWREG->set(0);
	}
	
	int size0 = ip0->acceSize();
	bool dir0 = (size0 == 1 && ip0->type == DIR_ITEM && ip0->attr->isVolatile);
	if ( dir0 ) {
		ASM_CODE(_MOVWF, shiftCount = ACC0);
		ASM_CODE(_MOVF, acceItem(ip0, 0), _W_);
	}
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());
	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s);
	for(int i = 0; i < size0; i++)
	{
		if ( dir0 )
			ASM_CODE(_LSLF, WREG, _F_);
		else if ( i == 0 )
			ASM_CODE(_LSLF, acceItem(ip0, i, indf0), _F_);
		else
			ASM_CODE(_RLF,  acceItem(ip0, i, indf0), _F_);

		if ( indf0 && size0 > 1 )
		{
			if ( (i+1) < size0 )
				regFSR0->inc(1, true);		//ASM_CODE(_ADDFSR, indf0, 1);
			else
				regFSR0->inc(1-size0, true);//ASM_CODE(_ADDFSR, indf0, 1-size0);
		}
	}
	ASM_LABL(endLbl->val.s, true);
	ASM_CODE(_DECFSZ, shiftCount, _F_);
	ASM_CODE(_BRA, startLbl->val.s);
	if ( dir0 ) ASM_CODE(_MOVWF, acceItem(ip0, 0));
	delete startLbl;
	delete endLbl;
	regWREG->reset();
}

void PIC16E :: leftAssignConst(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int n = ip1->val.i;
	if ( n > (size0*8) ) n = (size0*8);

	int byte_shift = n/8;
	int bit_shift  = n%8;
	char *indf0 = setFSR0(ip0);
	if ( ip0->type == DIR_ITEM && ip0->attr->isVolatile && size0 == 1 && n > 1 ) 
	{
		fetch(ip0, 0, indf0);
		while ( n-- ) ASM_CODE(_LSLF, WREG);
		store(ip0, 0, indf0);
		regWREG->reset();
		return;
	}
	if ( byte_shift > 0 )
	{
		for(int i = size0; i--;)
		{
			if ( i >= byte_shift )
			{
				fetch(ip0, i-byte_shift, indf0);
				store(ip0, i, indf0);
				regWREG->reset();
			}
			else if ( indf0 )
			{
				regWREG->load(0);
				store(ip0, i, indf0);
			}
			else
				ASM_CODE(_CLRF, acceItem(ip0, i));
		}
	}

	if ( bit_shift )
	{
		if ( indf0 && byte_shift > 0 )
			regFSR0->inc(byte_shift, true);	//ASM_CODE(_ADDFSR, indf0, byte_shift);
		
		if ( (size0 - byte_shift) == 1 )
		{
			if ( bit_shift >= 4 )
			{
				ASM_CODE(_SWAPF, acceItem(ip0, size0-1, indf0), _F_);
				regWREG->load(0xf0);
				ASM_CODE(_ANDWF, acceItem(ip0, size0-1, indf0), _F_);
				bit_shift -= 4;
			}

			while ( bit_shift-- )
				ASM_CODE(_LSLF, acceItem(ip0, size0-1, indf0), _F_);

			return;
		}

		Item *startLbl = lblItem(pcoder->getLbl());
		if ( bit_shift > 1 )
		{
			regWREG->load(bit_shift);
			ASM_LABL(startLbl->val.s, true);
		}
		for (int i = byte_shift; i < size0; i++)
		{
			char *inst = (i == byte_shift)? _LSLF: _RLF;
			ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);

			if ( indf0 && (size0-byte_shift) > 1 )
			{
				if ( (i+1) < size0 )
					regFSR0->inc(1, true);					//ASM_CODE(_ADDFSR, indf0, 1);
				else
					regFSR0->inc(1-size0+byte_shift, true);	//ASM_CODE(_ADDFSR, indf0, 1-size0+byte_shift);
			}
		}
		if ( bit_shift > 1 )
		{
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
		if ( sshift )	// signed shift
			rightAssignConstSigned(ip0, ip1);
		else			// unsigned shift
			rightAssignConst(ip0, ip1);
		return;
	}

	char *shiftCount = NULL;
	char *indf1;
	if ( ip1->type == ACC_ITEM || ip1->type == TEMP_ITEM )
	{
		shiftCount = acceItem(ip1, 0);
		ASM_CODE(_INCF, regWREG->reset(shiftCount), _F_);
	}
	else if ( useFSR(ip0, ip1) )
	{
		indf1 = setFSR0(ip1);
		ASM_CODE(_INCF, acceItem(ip1, 0, indf1), _W_);
		ASM_CODE(_MOVWF, ACC0);
		shiftCount = ACC0;
		regWREG->reset();
	}

	char *indf0 = setFSR0(ip0);
	if ( shiftCount == NULL )
	{
		indf1 = setFSR0(ip1);
		if ( CONST_ID_ITEM(ip1) )
		{
			fetch(ip1, 0, indf1);
			ASM_CODE(_ADDLW, 1);
		}
		else
			ASM_CODE(_INCF, acceItem(ip1, 0, indf1), _W_);

		shiftCount = WREG;
		regWREG->reset();
	}

	int size0 = ip0->acceSize();
	bool dir0 = (size0 == 1 && ip0->type == DIR_ITEM && ip0->attr->isVolatile);
	if ( dir0 ) {
		ASM_CODE(_MOVWF, shiftCount = ACC0);
		ASM_CODE(_MOVF, acceItem(ip0, 0), _W_);
	}
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());
	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s, true);

	if ( indf0 && size0 > 1 )
		ASM_CODE(_ADDFSR, indf0, size0-1);

	for(int i = size0; i--;)
	{
		if ( dir0 )
			ASM_CODE(sshift? _ASRF: _LSRF, WREG, _F_);
		else if ( i == (size0-1) && sshift )
			ASM_CODE(_ASRF, acceItem(ip0, i, indf0), _F_);
		else if ( i == (size0-1) )
			ASM_CODE(_LSRF, acceItem(ip0, i, indf0), _F_);
		else
			ASM_CODE(_RRF,  acceItem(ip0, i, indf0), _F_);

		if ( i != 0 && indf0 )
			regFSR0->inc(-1, true);	//ASM_CODE(_ADDFSR, indf0, -1);
	}
	ASM_LABL(endLbl->val.s, true);
	ASM_CODE(_DECFSZ, shiftCount, _F_);
	ASM_CODE(_BRA, startLbl->val.s);
	if ( dir0 ) ASM_CODE(_MOVWF, acceItem(ip0, 0));
	delete startLbl;
	delete endLbl;
	regWREG->reset();
}

void PIC16E :: rightAssignConst(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int n = ip1->val.i;
	if ( n > (size0*8) ) n = (size0*8);
	char *indf0 = setFSR0(ip0);
	int byte_shift = n/8;
	int bit_shift  = n%8;
	if ( ip0->type == DIR_ITEM && ip0->attr->isVolatile && size0 == 1 && n > 1 ) {
		fetch(ip0, 0, indf0);
		while ( n-- ) ASM_CODE(_LSRF, WREG);
		store(ip0, 0, indf0);
		regWREG->reset();
		return;
	}
	
	if ( byte_shift > 0 )
	{
		for(int i = 0; i < size0; i++)
		{
			if ( i < (size0-byte_shift) )
			{
				fetch(ip0, i+byte_shift, indf0);
				store(ip0, i, indf0);
				regWREG->reset();
			}
			else if ( indf0 )
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
		if ( indf0 && size0 > (byte_shift+1) )
			regFSR0->inc(size0-byte_shift-1, true);	//ASM_CODE(_ADDFSR, indf0, size0-byte_shift-1);
		
		if ( (size0 - byte_shift) == 1 )
		{
			if ( bit_shift >= 4 )
			{
				ASM_CODE(_SWAPF, acceItem(ip0, size0-1, indf0), _F_);
				regWREG->load(0x0f);
				ASM_CODE(_ANDWF, acceItem(ip0, size0-1, indf0), _F_);
				bit_shift -= 4;
			}

			while ( bit_shift-- )
				ASM_CODE(_LSRF, acceItem(ip0, size0-1, indf0), _F_);

			return;
		}		
		
		Item *startLbl = lblItem(pcoder->getLbl());
		if ( bit_shift > 1 )
		{
			regWREG->load(bit_shift);
			ASM_LABL(startLbl->val.s, true);
		}

		for(int i = size0-byte_shift; i--; )
		{
			if ( (i+1) == (size0-byte_shift) )
				ASM_CODE(_LSRF, acceItem(ip0, i, indf0), _F_);
			else
				ASM_CODE(_RRF,  acceItem(ip0, i, indf0), _F_);

			if ( i && indf0 )
				regFSR0->inc(-1, true);	//ASM_CODE(_ADDFSR, indf0, -1);
		}

		if ( bit_shift > 1 )
		{
			ASM_CODE(_DECFSZ, WREG, _F_);
			ASM_CODE(_BRA, startLbl->val.s);
		}
		delete startLbl;
		regWREG->set(0);
	}
}

void PIC16E :: rightAssignConstSigned(Item *ip0, Item *ip1)
{
	char *indf0 = setFSR0(ip0);
	int size0 = ip0->acceSize();
	int n = ip1->val.i;
	Item *startLbl = lblItem(pcoder->getLbl());

	if ( n > (size0*8) ) n = size0*8;

	if ( n > 1 )
	{
		regWREG->load(n);
		ASM_LABL(startLbl->val.s, true);
	}

	if ( size0 > 1 && indf0 )
		regFSR0->inc(size0-1, true);	//ASM_CODE(_ADDFSR, indf0, size0-1);

	for (int i = size0; i--;)
	{
		if ( i == (size0-1) )
			ASM_CODE(_ASRF, acceItem(ip0, i, indf0), _F_);
		else
			ASM_CODE(_RRF,  acceItem(ip0, i, indf0), _F_);

		if ( i != 0 && indf0 )
			regFSR0->inc(-1, true);		//ASM_CODE(_ADDFSR, indf0, -1);
	}
	if ( n > 1 )
	{
		ASM_CODE(_DECFSZ, WREG, _F_);
		ASM_CODE(_BRA, startLbl->val.s);
		regWREG->set(0);
	}
	delete startLbl;
}
