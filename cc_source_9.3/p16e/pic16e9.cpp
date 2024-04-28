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

	if ( related(ip0, ip2) )
	{
		char *indf2 = setFSR0(ip2);
		ASM_CODE(_INCF, acceItem(ip2, 0, indf2), _W_);
		ASM_CODE(_MOVWI, "--INDF1");
		leftOprIndf1(ip0, ip1);
		return;
	}

	if ( ip0->acceSize() == 1 && ip0->type == DIR_ITEM && ip0->attr->isVolatile &&
		 !(ip2->type == CON_ITEM && ip2->val.i == 1) 							 )
	{
		Item *acc = accItem(newAttr(CHAR));
		if ( ip2->type == ACC_ITEM )
		{
			ASM_CODE(_INCF, ACC0, _W_);
			ASM_CODE(_MOVWI, "--INDF1");
			leftOprIndf1(acc, ip1);
		}
		else
		{
			mov(acc, ip1);
			leftAssign(acc, ip2);
		}
		mov(ip0, acc);
		delete acc;
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
	int byte_shift = ip2->val.i/8;
	int bit_shift  = ip2->val.i%8;

	if ( byte_shift >= size0 )
	{
		Item *ip = intItem(0);
		mov(ip0, ip);
		delete ip;
		return;
	}

	if ( ip2->val.i == 1 && size0 <= size1 )	// simply shift + mov
	{
		leftOpr(ip0, ip1);
		return;
	}

	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	for (int i = 0; i < size0; i++)
	{
		if ( i < byte_shift )
		{
			if ( indf0 && size0 > 1 )
			{
				regWREG->load(0);
				store(ip0, i, indf0);
			}
			else
				ASM_CODE(_CLRF, acceItem(ip0, i, indf0));
		}
		else if ( (i-byte_shift) < size1 )
		{
			fetch(ip1, i-byte_shift, indf1);
			store(ip0, i, indf0);
			regWREG->reset();
		}
		else if ( !sign1 && indf0 == NULL )
			ASM_CODE(_CLRF, acceItem(ip0, i, indf0));
		else
		{
			if ( (i-byte_shift) == size1 )
			{
				if ( sign1 )
					EXTEND_WREG;
				else
					regWREG->load(0);
			}
			store(ip0, i, indf0);
		}
	}
	
	if ( bit_shift > 0 )
	{
		if ( (size0 - byte_shift) == 1 )
		{
			if ( bit_shift < 4 && (indf0 == NULL || size0 == 1) )
				while ( bit_shift-- ) ASM_CODE(_LSLF, acceItem(ip0, size0 - 1, indf0), _F_);
			else
			{				
				if ( bit_shift >= 4 )
				{
					ASM_CODE(_SWAPF, WREG, _W_);
					ASM_CODE(_ANDLW, 0xf0);
					bit_shift -= 4;
				}
				while ( bit_shift-- ) ASM_CODE(_LSLF, WREG, _W_);
				store(ip0, size0-1, indf0);
				regWREG->reset();
			}
			return;
		}
		
		char *inst = _LSLF;
		Item *startLbl = lblItem(pcoder->getLbl());
		if ( bit_shift > 1 )
		{
			regWREG->load(bit_shift);
			ASM_LABL(startLbl->val.s);
		}
		for(int i = byte_shift; i < size0; i++, inst = _RLF)
			ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);

		if ( bit_shift > 1 )
		{
			ASM_CODE(_DECFSZ, WREG, _F_);
			ASM_CODE(_BRA, startLbl->val.s);
		}
		delete startLbl;
	}
}

void PIC16E :: leftOpr(Item *ip0, Item *ip1)
{
	int   size0 = ip0->acceSize();
	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	for(int i = 0; i < size0; i++)
	{
		char *inst = (i == 0)? _LSLF: _RLF;

		ASM_CODE(inst, acceItem(ip1, i, indf1), _W_);
		store(ip0, i, indf0);
		regWREG->reset();
	}
}

void PIC16E :: leftOprIndf1(Item *ip0, Item *ip1)
{
	mov(ip0, ip1);

	int size0 = ip0->acceSize();
	char *indf0 = setFSR0(ip0);
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());
	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s);
	char *inst = _LSLF;
	for(int i = 0; i < size0; i++, inst = _RLF)
		ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);

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

	if ( related(ip0, ip2) )
	{
		ASM_CODE(_INCF, acceItem(ip2, 0, setFSR0(ip2)), _W_);
		ASM_CODE(_MOVWI, "--INDF1");
		rightOprIndf1(ip0, ip1);
		return;
	}
	
	if ( ip0->acceSize() == 1 && ip0->type == DIR_ITEM && ip0->attr->isVolatile &&
		 !(ip2->type == CON_ITEM && ip2->val.i == 1) 							 )
	{
		Item *acc = accItem(newAttr(CHAR + size1 - 1));		
		if ( ip2->type == ACC_ITEM )
		{
			ASM_CODE(_INCF, ACC0, _W_);
			ASM_CODE(_MOVWI, "--INDF1");
			rightOprIndf1(acc, ip1);
		}
		else
		{
			mov(acc, ip1);
			rightAssign(acc, ip2, sign1);
		}
		mov(ip0, acc);
		delete acc;
		return;
	}

	if ( useFSR(ip0, ip1) || ip2->type != CON_ITEM )
	{
		if ( size0 >= size1 )
		{
			mov(ip0, ip1);
			rightAssign(ip0, ip2, sign1);
		}
		else if ( ip1->type == ACC_ITEM || ip1->type == TEMP_ITEM )
		{
			rightAssign(ip1, ip2, sign1);
			mov(ip0, ip1);
			return;
		}
		else if ( ip2->type == ACC_ITEM )
		{
			ASM_CODE(_INCF, ACC0, _W_);
			ASM_CODE(_MOVWI, "--INDF1");
			rightOprIndf1(ip0, ip1);
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

	if ( ip2->val.i == 1 && size0 <= size1 )	// ip0 = ip1 >> 1
	{
		rightOpr(ip0, ip1);
		return;
	}

	int n = ip2->val.i;
	if ( n > size1*8 ) n = size1*8;
	int byte_shift = n / 8;
	int bit_shift  = n % 8;

	if ( (size0 + byte_shift) >= size1 || !bit_shift || !sign1 )
	{
		if ( byte_shift )
			rightOpr(ip0, ip1, byte_shift);
		else
			mov(ip0, ip1);

		if ( bit_shift )
		{
			Item *ip = intItem(bit_shift);
			rightAssign(ip0, ip, sign1);
			delete ip;
		}
		return;
	}
	
	// (size0 + byte_shift) < size1 ...
	Item *acc = NULL, *ip = ip1;
	if ( !(ip1->type == TEMP_ITEM || ip1->type == ACC_ITEM) )
	{
		attrib *attr = newAttr((size1 == 2)? INT:
							   (size1 == 3)? SHORT: LONG);
		ip = acc = accItem(attr);
	}
	rightAssign(ip, ip2, sign1);
	if ( !overlap(ip0, ip) ) mov(ip0, ip);
	if ( acc ) delete acc;
}

void PIC16E :: rightOprIndf1(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	Item *acc = NULL;
	Item *ip  = ip1;
	int size  = size1;
	if ( size0 >= size1 )
	{
		mov(ip = ip0, ip1);
		size = size0;
	}
	else if ( ip1->type != ACC_ITEM )
	{
		ip = acc = storeToACC(ip1, size1);
	}

	bool sign1 = ip1->acceSign();
	char *indf = setFSR0(ip);
	Item *startLbl = lblItem(pcoder->getLbl());
	Item *endLbl   = lblItem(pcoder->getLbl());
	ASM_CODE(_BRA, endLbl->val.s);
	ASM_LABL(startLbl->val.s);

	char *inst = sign1? _ASRF: _LSRF;
	for(int i = size; i--; inst = _RRF)
		ASM_CODE(inst, acceItem(ip, i, indf), _F_);

	ASM_LABL(endLbl->val.s);
	ASM_CODE(_DECFSZ, INDF1, _F_);
	ASM_CODE(_BRA, startLbl->val.s);
	ASM_CODE(_ADDFSR, INDF1, 1);

	if ( acc ) { mov(ip0, ip); delete acc; }
	delete startLbl;
	delete endLbl;
}

// 1-bit shift operation ...
void PIC16E :: rightOpr(Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();
	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	char *inst = sign1? _ASRF: _LSRF;
	int  start_position = (size1 > size0)? size0+1: size0;

	for (int i = start_position; i--; inst = _RRF)
	{
		ASM_CODE(inst, acceItem(ip1, i, indf1), _W_);
		if ( i < size0 ) 
			store(ip0, i, indf0);
	}
}

void PIC16E :: rightOpr(Item *ip0, Item *ip1, int byte_shift)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();
	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);

	for(int i = 0; i < size0; i++)
	{
		if ( !(byte_shift >= size1 && sign1) )
		{
			if ( i+byte_shift < size1 )
				fetch(ip1, i+byte_shift, indf1);
			else if ( !sign1 )
				regWREG->load(0);
			else if ( i+byte_shift == size1 )
				EXTEND_WREG;
		}
		else if ( i == 0 )
		{
			fetch(ip1, size1 - 1, indf1);
			EXTEND_WREG;
		}
		store(ip0, i, indf0);
	}
}
