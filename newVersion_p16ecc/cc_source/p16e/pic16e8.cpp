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

void PIC16E :: leftOpr(Item *ip0, Item *ip1, Item *ip2)
{
	if ( same(ip0, ip1) )
	{
		leftAssign(ip0, ip2);
		return;
	}
	if ( related(ip0, ip2) || related(ip1, ip2) )
	{
		ASM_CODE(_INCF, acceItem(ip2, 0, setFSR0(ip2)), _W_);
		ASM_CODE(_MOVWI, "--INDF1");
		leftOprIndf1(ip0, ip1);
		return;
	}
	if ( useFSR(ip0, ip1) || ip2->type != CON_ITEM )
	{
		mov(ip0, ip1);
		leftAssign(ip0, ip2);
		return;
	}

	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();
	int  n = (ip2->val.i >= size0*8)? size0*8: ip2->val.i;
	int  byte_shift = n/8;
	int  bit_shift  = n%8;
	bool shift_move = (bit_shift == 1 && (byte_shift + size1) >= size0);
	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);

	// move/shift at byte level, from ip1 to ip0
	for (int i = 0; i < size0; i++)
	{
		if ( i < byte_shift )
			ASM_CODE(_CLRF, acceItem(ip0, i, indf0));	// clear lower bytes of ip0
		else if ( shift_move )
			break;
		else if ( (i-byte_shift) < size1 )
		{
			fetch(ip1, i-byte_shift, indf1);			// move bytes from ip1 to ip0
			store(ip0, i, indf0);
			regWREG->reset();
		}
		else if ( !sign1 )
			ASM_CODE(_CLRF, acceItem(ip0, i, indf0));	// clear higher bytes of ip0
		else
		{
			if ( (i-byte_shift) == size1 )
			{
				fetch(ip1, i-byte_shift, indf1);
				EXTEND_WREG;
				regWREG->reset();
			}
			store(ip0, i, indf0);
		}
	}

	if ( bit_shift > 0 )
	{
		if ( bit_shift >= 4 && (size0 - byte_shift) == 1 )
		{
			ASM_CODE(_SWAPF, acceItem(ip0, byte_shift, indf0), _W_);
			ASM_CODE(_ANDLW, 0xf0);
			store(ip0, byte_shift, indf0);
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
				if ( shift_move )
				{
					ASM_CODE(inst, acceItem(ip1, i-byte_shift, indf1), _W_);
					store(ip0, i, indf0);
				}
				else
					ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);
			}

		if ( bit_shift > 1 && (size0 - byte_shift) > 1 )
		{
			if ( indf0 ) regFSR0->restore(offset);	// reset FSR0 offset
			ASM_CODE(_DECFSZ, WREG, _F_);
			ASM_CODE(_BRA, startLbl->val.s);
		}
		delete startLbl;
		regWREG->reset();
	}
}

void PIC16E :: leftOprIndf1(Item *ip0, Item *ip1)
{
	mov(ip0, ip1);

	int size0 = ip0->acceSize();
	char *indf0 = setFSR0(ip0);
	int  offset = regFSR0->getVarOffset();
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());

	if ( !indf0 ) regBSR->reset(ip0);
	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s);
	char *inst = _LSLF;

	for(int i = 0; i < size0; i++, inst = _RLF)
		ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);

	if ( indf0 ) regFSR0->restore(offset);
	ASM_LABL(endLbl->val.s);
	ASM_CODE(_DECFSZ, INDF1, _F_);
	ASM_CODE(_BRA, startLbl->val.s);
	ASM_CODE(_ADDFSR, INDF1, 1);
	delete startLbl;
	delete endLbl;
}

void PIC16E :: rightOpr(Item *ip0, Item *ip1, Item *ip2)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();

	if ( same(ip0, ip1) )
	{
		rightAssign(ip0, ip2, sign1);
		return;
	}
	if ( related(ip0, ip2) || related(ip1, ip2) )
	{
		ASM_CODE(_INCF, acceItem(ip2, 0, setFSR0(ip2)), _W_);
		ASM_CODE(_MOVWI, "--INDF1");
		rightOprIndf1(ip0, ip1);
		return;
	}
	if ( useFSR(ip0, ip1) || ip2->type != CON_ITEM )
	{
		if ( size0 >= size1 || !sign1 )
		{
			mov(ip0, ip1);
			rightAssign(ip0, ip2, sign1);
		}
		else
		{
			Item *acc = storeToACC(ip1, size1);
			rightAssign(acc, ip2, sign1);
			mov(ip0, acc);
			delete acc;
		}
		return;
	}

	int  n = ip2->val.i;
	int  byte_shift = n / 8;
	int  bit_shift  = n % 8;
	if ( (size1 > (size0 + byte_shift) && bit_shift > 1) || size1 < size0 )
	{
		Item *acc = accItem(newAttr(CHAR + size1 - 1));
		acc->attr->isUnsigned = sign1? 0: 1;
		rightOpr(acc, ip1, ip2);
		mov(ip0, acc);
		delete acc;
		return;
	}

	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	if ( byte_shift > size1 ) byte_shift = size1;
	bool shift_move = (bit_shift == 1 && size1 >= (size0 + byte_shift));

	for (int i = 0; i < size0; i++)
	{
		if ( (byte_shift+i) < size1 )
		{
			if ( shift_move ) break;
			fetch(ip1, byte_shift+i, indf1);
			store(ip0, i, indf0);
			regWREG->reset();
		}
		else if ( sign1 )
		{
			if ( (byte_shift+i) == size1 )
			{
				if ( byte_shift >= size1 )
					fetch(ip1, size1-1, indf1);
				EXTEND_WREG;
				regWREG->reset();
			}
			store(ip0, i, indf0);
		}
		else
			ASM_CODE(_CLRF, acceItem(ip0, i, indf0));
	}

	if ( bit_shift > 0 && size1 > byte_shift )
	{
		int repeat = bit_shift;
		int offset = regFSR0->getVarOffset();
		Item *startLbl = lblItem(pcoder->getLbl());
		if ( !indf0 ) regBSR->reset(ip0);
		if ( !indf1 && shift_move ) regBSR->reset(ip1);

		if ( bit_shift > 1 )
		{
			ASM_CODE(_MOVLW, bit_shift);
			ASM_LABL(startLbl->val.s);
			repeat = 1;
		}

		while ( repeat-- > 0 )
			for (int i = size1 - byte_shift; i--;)
			{
				char *inst = _RRF;
				if ( (i+byte_shift+1) == size1 )
					inst = sign1? _ASRF: _LSRF;

				if ( shift_move )
				{
					ASM_CODE(inst, acceItem(ip1, i+byte_shift, indf1), _W_);
					if ( i < size0 ) store(ip0, i, indf0);
				}
				else if ( i < size0 )
				{
					ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);
				}

				regWREG->reset();
			}

		if ( bit_shift > 1 )
		{
			if ( indf0 || indf1 ) regFSR0->restore(offset);
			ASM_CODE(_DECFSZ, WREG, _F_);
			ASM_CODE(_BRA, startLbl->val.s);
			regWREG->reset();
		}
		delete startLbl;
	}
}

void PIC16E :: rightOprIndf1(Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();
	Item *acc  = NULL;
	Item *ip   = NULL;
	int size   = size1;
	if ( size0 >= size1 )
	{
		mov(ip = ip0, ip1);
		size = size0;
	}
	else
	{
		if ( ip1->type != ACC_ITEM )
			ip = acc = storeToACC(ip1, size1);
		else
			ip = acc = accItem(newAttr(CHAR + size1 - 1));
	}

	char *indf = setFSR0(ip);
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());
	int  offset = regFSR0->getVarOffset();

	if ( !indf ) regBSR->reset(ip);
	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s);

	char *inst = sign1? _ASRF: _LSRF;
	for(int i = size; i--; inst = _RRF)
		ASM_CODE(inst, acceItem(ip, i, indf), _F_);

	if ( indf ) regFSR0->restore(offset);
	ASM_LABL(endLbl->val.s);
	ASM_CODE(_DECFSZ, INDF1, _F_);
	ASM_CODE(_BRA, startLbl->val.s);
	ASM_CODE(_ADDFSR, INDF1, 1);

	if ( acc ) { mov(ip0, acc); delete acc; }
	delete startLbl;
	delete endLbl;
}
