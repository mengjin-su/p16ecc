#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <string>
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


static char *to_string(int n)	// try to mimic C++ std::to_string()
{
	static char buf[16];
	sprintf(buf, "%d", n);
	return buf;
}

void PIC16E :: getBF(Item *ip, int left_shift, int req_size, BF_Target target)
{
	if ( ip->isBF() )	// it's from a bit-field type data
	{
		char *indf = setFSR0(ip);
		int offset = BF_OFFSET(ip->attr);
		int size   = BF_SIZE(ip->attr);
		int shift  = offset - left_shift;
		uint8_t mask = (req_size < size)? ((1 << req_size) - 1) << offset:
										  ((1 <<     size) - 1) << offset;
		if ( shift > 0 ) mask >>= shift;
		else		     mask <<= abs(shift);

		switch ( shift )	// fetch the data into WREG
		{
			case 0:
				ASM_CODE(_MOVF, acceItem(ip, 0, indf), _W_);
				break;
			case 1:  case 2:  case 3:
				ASM_CODE(_RRF, acceItem(ip, 0, indf), _W_);
				shift--; break;
			case -1: case -2: case -3:
				ASM_CODE(_RLF, acceItem(ip, 0, indf), _W_);
				shift++; break;
			case 7:
				ASM_CODE(_RLF, acceItem(ip, 0, indf), _W_);
				shift = -1;	break;
			case -7:
				ASM_CODE(_RRF, acceItem(ip, 0, indf), _W_);
				shift = 1;	break;
			default:
				ASM_CODE(_SWAPF, acceItem(ip, 0, indf), _W_);
				shift += (shift > 0)? -4: 4; break;
		}

		while ( shift )	// more shift in WREG ...
		{
			if ( shift > 0 )
			{
				ASM_CODE(_RRF, WREG, _F_);
				shift--;
			}
			else
			{
				ASM_CODE(_RLF, WREG, _F_);
				shift++;
			}
		}

		if ( mask != 0xff && req_size < 8 )
			ASM_CODE(_ANDLW, mask);

		switch ( target )
		{
			case TO_ACC: 	ASM_CODE(_MOVWF, ACC0);			break;
			case TO_STACK:	ASM_CODE(_MOVWI, "--INDF1");	break;
		}
		regWREG->reset();
	}
}

void PIC16E :: movToBF(Item *ip0, Item *ip1)
{
	if ( ip1->type == CON_ITEM )
		movToBF(ip0, ip1->val.i);
	else if ( ip1->type == IMMD_ITEM )
		movToBF(ip0, ip1->val.s);
	else
	{
		int size0     = BF_SIZE(ip0->attr);
		int bit_shift = BF_OFFSET(ip0->attr);
		uint8_t mask  = ((1 << size0) - 1) << bit_shift;

		if ( ip1->isBF() )
			getBF(ip1, bit_shift, 8);
		else
		{
			char *indf1 = setFSR0(ip1);
			char *inst = _MOVF;

			if ( bit_shift && bit_shift != 4 )
			{
				inst = _RLF;
				bit_shift--;
			}
			ASM_CODE(inst, acceItem(ip1, 0, indf1), _W_);

			while ( bit_shift > 0 )
			{
				if ( bit_shift < 4 )
				{
					ASM_CODE(_RLF, WREG, _F_);
					bit_shift--;
				}
				else
				{
					ASM_CODE(_SWAPF, WREG, _F_);
					bit_shift -= 4;
				}
			}
		}

		if ( useFSR(ip0) ) ASM_CODE(_MOVWI, "--INDF1");
		char *indf0 = setFSR0(ip0);
		if ( indf0 ) ASM_CODE(_MOVIW, "INDF1++");
		ASM_CODE(_XORWF, acceItem(ip0, 0, indf0), _W_);
		ASM_CODE(_ANDLW, mask);
		ASM_CODE(_XORWF, acceItem(ip0, 0, indf0), _F_);
		regWREG->reset();
	}
}

void PIC16E :: movToBF(Item *ip0, int n)
{
	char *indf = setFSR0(ip0);
	int  size  = BF_SIZE(ip0->attr);
	int  shift = BF_OFFSET(ip0->attr);

	uint8_t mask = ((1 << size) - 1) << shift;
	n = (n << shift) & mask;

	switch ( size )
	{
		case 1:		// 1-bit bit-field data
			if ( n )
				ASM_CODE(_BSF, acceItem(ip0, 0, indf), shift);
			else
				ASM_CODE(_BCF, acceItem(ip0, 0, indf), shift);
			return;
		case 8:		// 1byte bit-field data
			if ( n == 0 )
			{
				ASM_CODE(_CLRF, acceItem(ip0, 0, indf));
				return;
			}
			ASM_CODE(_MOVLW, n);
			ASM_CODE(_MOVWF, acceItem(ip0, 0, indf));
			break;
		default:
			if ( n == 0 )							// zero the data field
			{
				ASM_CODE(_MOVLW, ~mask);
				ASM_CODE(_ANDWF, acceItem(ip0, 0, indf), _F_);
			}
			else if ( n == mask )					// 1's the data field
			{
				ASM_CODE(_MOVLW, mask);
				ASM_CODE(_IORWF, acceItem(ip0, 0, indf), _F_);
			}
			else
			{
				ASM_CODE(_MOVF, acceItem(ip0, 0, indf), _W_);
				ASM_CODE(_ANDLW, ~mask);
				ASM_CODE(_IORLW, n);
				ASM_CODE(_MOVWF, acceItem(ip0, 0, indf));
			}
			break;
	}
	regWREG->reset();
}

void PIC16E :: movToBF(Item *ip0, char *s)
{
	char *indf = setFSR0(ip0);
	int  size  = BF_SIZE(ip0->attr);
	int  shift = BF_OFFSET(ip0->attr);
	std::string str = s;

	uint8_t mask = ((1 << size) - 1) << shift;

	if ( shift )
	{
		mask <<= shift;
		str = "("+ str +")<<" + to_string(shift);
	}

	str = "(" + str + ")&" + to_string(mask);

	ASM_CODE(_MOVF, acceItem(ip0, 0, indf), _W_);
	ASM_CODE(_ANDLW, ~mask);
	ASM_CODE(_IORLW, str.c_str());
	ASM_CODE(_MOVWF, acceItem(ip0, 0, indf));
	regWREG->reset();
}

void PIC16E :: movFromBF(Item *ip0, Item *ip1)
{
	getBF(ip1, 0, BF_SIZE(ip1->attr), useFSR(ip0)? TO_STACK: TO_WREG);

	char *indf0 = setFSR0(ip0);
	if ( indf0 ) ASM_CODE(_MOVIW, "INDF1++");
	ASM_CODE(_MOVWF, acceItem(ip0, 0, indf0));
	regWREG->reset();

	for (int i = 1; i < ip0->acceSize(); i++)
		ASM_CODE(_CLRF, acceItem(ip0, i, indf0));
}