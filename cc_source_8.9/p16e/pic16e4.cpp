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

static bool selectBit(int n, int *b);

void PIC16E :: addsub(int code, Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	char *indf0, *indf1;
	char *inst = (code == ADD_ASSIGN)? _ADDWF: _SUBWF;

	if ( ip1->type == CON_ITEM )
	{
		int n = (code == ADD_ASSIGN)? ip1->val.i: -ip1->val.i;
		incValue(ip0, n);
		return;
	}

	if ( useFSR(ip0, ip1) )
	{
		if ( size0 == 1 )
		{
			indf1 = setFSR0(ip1);
			fetch(ip1, 0, indf1);
			ASM_CODE(_MOVWF, ACC0);
			indf0 = setFSR0(ip0);
			ASM_CODE(_MOVF, ACC0, _W_);
			ASM_CODE(inst, acceItem(ip0, 0, indf0), _F_);
			regWREG->reset();
			return;
		}
		Item *acc = storeToACC(ip1, size0);
		addsub(code, ip0, acc);
		delete acc;
		return;
	}

	indf1 = setFSR0(ip1);
	indf0 = setFSR0(ip0);
	for(int i = 0; i < size0; i++)
	{
		if ( i < size1 )
			fetch(ip1, i, indf1);
		else if ( !ip1->acceSign() )
			regWREG->load(0);
		else if ( i == size1 )
			EXTEND_WREG;

		char *ds = acceItem(ip0, i, indf0);
		ASM_CODE(inst, regWREG->reset(ds), _F_);

		if ( indf0 && (i+1) < size0 )
			regFSR0->inc(1, true);

		inst = (code == ADD_ASSIGN)? _ADDWFC: _SUBWFB;
	}
}

void PIC16E :: andorxor(int code, Item *ip0, int n)
{
	int  size0 = ip0->acceSize();
	char *indf = setFSR0(ip0);
	int b, incfsr = 0;

	if ( isWREG(ip0) )
	{
		ASM_CODE((code == XOR_ASSIGN)? _XORLW:
				 (code == AND_ASSIGN)? _ANDLW:
									   _IORLW, n);
		regWREG->reset();
		return;
	}
	if ( ip0->attr && ip0->attr->type == SBIT )
	{
		int addr = (ip0->val. i >> 3) & 0x7f;
		int bit  = ip0->val.i & 7;
		regBSR->load(ip0->val.i >> 3);
		switch ( code )
		{
			case XOR_ASSIGN:
				if ( n == 1 )	// skip other values
				{
					regWREG->load(1 << bit);
					ASM_CODE(_XORWF, addr, _F_);
				}
				break;

			case AND_ASSIGN:
				if ( n == 0 ) 	// skip other values
					ASM_CODE(_BCF, addr, bit);
				break;

			case OR_ASSIGN:
				if ( n == 1 ) 	// skip other values
					ASM_CODE(_BSF, addr, bit);
				break;
		}
		return;
	}
	for(int i =	0; i < size0; i++, n >>= 8)
	{
		char *inst = NULL;
		char *s = NULL;
		char *f = NULL;
		if ( selectBit(n, &b) && code == OR_ASSIGN )
		{
			inst = _BSF;
			s = acceItem(ip0, i, indf);
			f = STRBUF(); sprintf(f, "%d", b);
		}
		else if ( selectBit(~n, &b) && code == AND_ASSIGN )
		{
			inst = _BCF;
			s = acceItem(ip0, i, indf);
			f = STRBUF(); sprintf(f, "%d", b);
		}
		else
		{
			switch ( n & 0xff )
			{
				case 0x00:
					if ( code == AND_ASSIGN )
					{
						inst = _CLRF;
						s = acceItem(ip0, i, indf);
					}
					break;

				case 0xff:
					if ( code == AND_ASSIGN )
						break;
					if ( code == XOR_ASSIGN )
					{
						inst = _COMF;
						s = acceItem(ip0, i, indf);
						f = _F_;
						break;
					}
					// fall through...
				default:
					regWREG->load(n);
					inst = (code == AND_ASSIGN)? _ANDWF:
						   (code == XOR_ASSIGN)? _XORWF: _IORWF;
					s = acceItem(ip0, i, indf);
					f = _F_;
			}
		}
		if ( inst && s )
		{
			if ( indf && incfsr )
				regFSR0->inc(incfsr, true);

			ASM_CODE(inst, s, f);
			regWREG->reset(s);
			incfsr = 0;
		}
		incfsr++;
	}
}

void PIC16E :: andorxor(int code, Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int sign1 = ip1->acceSign();
	char *indf0, *indf1;
	char *inst = (code == AND_ASSIGN)? _ANDWF:
				 (code == XOR_ASSIGN)? _XORWF: _IORWF;

	if ( ip1->type == CON_ITEM )
	{
		andorxor(code, ip0, ip1->val.i);
		return;
	}
	if ( useFSR(ip0, ip1) )
	{
		if ( size0 == 1 )
		{
			indf1 = setFSR0(ip1);
			fetch(ip1, 0, indf1);
			ASM_CODE(_MOVWF, ACC0);
			indf0 = setFSR0(ip0);
			ASM_CODE(_MOVF, ACC0, _W_);
			ASM_CODE(inst, acceItem(ip0, 0, indf0), _F_);
			regWREG->reset();
			return;
		}
		Item *acc = storeToACC(ip1, size0);
		andorxor(code, ip0, acc);
		delete acc;
		return;
	}
	indf1 = setFSR0(ip1);
	indf0 = setFSR0(ip0);
	for(int i = 0; i < size0; i++)
	{
		char *ds;
		
		if ( i >= size1 && !sign1 && code != AND_ASSIGN )
			return;
		
		if ( i && indf0 )
			regFSR0->inc(1, true);

		if ( i < size1 )
			fetch(ip1, i, indf1);
		else if ( !sign1 )
		{
			ds = acceItem(ip0, i, indf0);
			asm16e->code(_CLRF, regWREG->reset(ds));
			continue;
		}
		else if ( i == size1 )
		{
			EXTEND_WREG;
		}

		ds = acceItem(ip0, i, indf0);
		asm16e->code(inst, regWREG->reset(ds), _F_);
	}
}

/////////////////////////////////////////////////////////////
static bool selectBit(int n, int *b)
{
	for (*b = 0; *b < 8; (*b)++)
		if ( (n & 0xff) == (1<<(*b)) )
			return true;

	return false;
}
