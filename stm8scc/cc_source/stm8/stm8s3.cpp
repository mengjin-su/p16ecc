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
#include "../option.h"
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../const.h"
#include "../nlist.h"
#include "stm8s.h"
#include "stm8s_reg.h"
#include "stm8s_inst.h"
#include "stm8s_asm.h"

void STM8S :: incValue(Item *ip, int value)
{
	int  size0 = ip->acceSize();
	char *reg = setPtr(ip, 0);
	bool started = false;

	if ( IS_BF_VAR(ip->attr) )
	{
		size0 = BF_SIZE(ip->attr);
		uint8_t offset = BF_OFFSET(ip->attr);
		uint8_t mask = ((1 << size0) - 1) << offset;
		value = (value << offset) & mask;

		ASM_CODE2(_LD, REG_A, acceItem(ip, 0, 1, reg));
		ASM_CODE2(_ADD, REG_A, value, '#');
		if ( (offset + size0) < 8 )
		{
			ASM_CODE2(_AND, REG_A, mask, '#');
			ASM_CODE2(_LD, 0, REG_A);
			ASM_CODE2(_LD, REG_A, acceItem(ip, 0, 1, reg));
			ASM_CODE2(_AND, REG_A, ~mask, '#');
			ASM_CODE2(_ADD, REG_A, 0);
		}
		ASM_CODE2(_LD, acceItem(ip, 0, 1, reg), REG_A);
		return;
	}

	for (int i = 0; i < size0; i++)
	{
		int v8  = (value >> (i*8)) & 0xff;
		int v16 = (value >> (i*8)) & 0xffff;
		if ( (v8 == 1 || v8 == 0xff) && !started && (i+1) == size0 )
		{
			if ( v8 == 1 )
				ASM_CODE1(_INC, acceItem(ip, i, 1, reg));
			else
				ASM_CODE1(_DEC, acceItem(ip, i, 1, reg));
		}
		else if ( (v16 || started) && (i+1) < size0 )
		{
			char *r16 = reg? REG_Y: REG_X;

			if ( reg ) {
				ASM_CODE2(_LDW, r16, reg);
				ASM_CODE2(_LDW, r16, acceItem(ip, i, 2, r16));
				if ( started ) {
					ASM_CODE1(_JRNC, ".+2");
					ASM_CODE1(_INCW, r16);	// add CARRY bit
				}
			}
			else
			{
				ASM_CODE2(_LDW, r16, acceItem(ip, i, 2, reg));
				if ( started ) {
					ASM_CODE1(_JRNC, ".+1");
					ASM_CODE1(_INCW, r16);	// add CARRY bit
				}
			}

			if ( v16 ) {
				if ( (i+2) == size0 && (v16 == 1 || v16 == 0xffff) )
					ASM_CODE1((v16 == 1)?_INCW: _DECW, r16);
				else
					ASM_CODE2(_ADDW, r16, v16, '#');
			}
			ASM_CODE2(_LDW, acceItem(ip, i, 2, reg), r16);
			i++; started = true;
		}
		else if ( v8 || started )
		{
			char *inst = started? _ADC: _ADD;
			ASM_CODE2(_LD, REG_A, acceItem(ip, i, 1, reg));
			ASM_CODE2(inst, REG_A, v8, '#');
			ASM_CODE2(_LD, acceItem(ip, i, 1, reg), REG_A);
			started = true;
		}
	}
}

void STM8S :: neg(Item *ip0)
{
	int size0 = ip0->acceSize();
	char *reg = setPtr(ip0, 0);
	if ( size0 == 1 )
	{
		ASM_CODE1(_NEG, acceItem(ip0, 0, 1, reg));
		return;
	}
	if ( size0 == 2 )
	{
		char *r16 = reg? REG_Y: REG_X;
		if ( reg )
		{
			ASM_CODE2(_LDW, r16, reg);
			ASM_CODE2(_LDW, r16, acceItem(ip0, 0, 2, r16));
		}
		else
			ASM_CODE2(_LDW, r16, acceItem(ip0, 0, 2, reg));

		ASM_CODE1(_NEGW, r16);
		ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg), r16);
		return;
	}
	for (int i = 0; i < size0; i++)
	{
		ASM_CODE1(_CLR, REG_A);
		ASM_CODE2(i? _SBC: _SUB, REG_A, acceItem(ip0, i, 1, reg));
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg), REG_A);
	}
}

void STM8S :: neg(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();

	if ( same(ip0, ip1) )
	{
		neg(ip0);
		return;
	}

	if ( ip1->type == IMMD_ITEM || ip1->type == LBL_ITEM )
	{	// 'move' & 'complete' -- not efficient
		mov(ip0, ip1);
		neg(ip0);
		return;
	}

	if ( size1 == 1 || size0 == 1 || farAcce(ip1) )
	{
		neg8(ip0, ip1);
		return;
	}

	char *reg1 = setPtr(ip1, 0);
	char *reg0 = setPtr(ip0, reg1? 1: 0);
	if ( reg0 && reg1 )
	{
		for (int i = 0; i < size0; i++)
		{
			if ( i < size1 ) {
				ASM_CODE2(_LD, REG_A, acceItem(ip1, i, 1, reg1));

				if ( i == 0 )
					ASM_CODE1(_NEG, REG_A);
				else {
					ASM_CODE2(_XOR, REG_A, 255, '#');
					ASM_CODE2(_ADC, REG_A, 0, '#');
				}
			}
			else if ( i == size1 ) {	// extend sign of A
				ASM_CODE1(_RLC, REG_A);
				ASM_CODE1(_CLR, REG_A);
				ASM_CODE2(_SBC, REG_A, 0, '#');
			}

			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
		return;
	}

	char *r16 = (reg0 || reg1)? REG_Y: REG_X;
	ASM_CODE2(_LDW, r16, acceItem(ip1, 0, 2, reg1));
	ASM_CODE1(_NEGW, r16);
	ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), r16);
	for (int i = 2; i < size0; i++)
	{
		if ( i < size1 )
		{
			ASM_CODE2(_LD, REG_A, acceItem(ip1, i, 1, reg1));
			ASM_CODE2(_XOR, REG_A, 255, '#');
			ASM_CODE2(_ADC, REG_A, 0, '#');
			r16 = NULL;
		}
		else if ( i == size1 )
		{
			if ( r16 ) ASM_CODE1(_RLCW, r16);
			else	   ASM_CODE1(_RLC,  REG_A);
			ASM_CODE1(_CLR, REG_A);
			ASM_CODE2(_SBC, REG_A, 0, '#');
		}
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: neg8(Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	char *reg1 = setPtr(ip1, 0);
	char *reg0 = setPtr(ip0, reg1? 1: 0);
	char *r16  = (reg1 || !reg0)? REG_X: REG_Y;
	char *rl   = (reg1 || !reg0)? REG_XL: REG_YL;
	for (int i = 0; i < size0; i++)
	{
		if ( i < size1 )
		{
			char *load = farAcce(ip1)? _LDF: _LD;
			ASM_CODE2(load, REG_A, acceItem(ip1, i, 1, reg1));
			if ( i == 0 )
				ASM_CODE1(_NEG, REG_A);
			else
			{
				ASM_CODE2(_XOR, REG_A, 255, '#');
				ASM_CODE2(_ADC, REG_A, 0, '#');
			}
			if ( size1 < size0 && (i+1) == size1 )
				ASM_CODE1(_CLRW, r16);
		}
		else
		{
			if ( i == size1 ) {
				ASM_CODE1(_JRPL, (reg1 || !reg0)? ".+1": ".+2");
				ASM_CODE1(_DECW, r16);
			}

			if ( (i+1) < size0 ) {
				ASM_CODE2(_LDW, acceItem(ip0, i++, 2, reg0), r16);
				continue;
			}
			ASM_CODE2(_LD, REG_A, rl);
		}
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: compl1(Item *ip0)
{
	int size0 = ip0->acceSize();
	char *reg = setPtr(ip0, 0);

	for (int i = 0; i < size0; i++)
		ASM_CODE1(_CPL, acceItem(ip0, i, 1, reg));
}

void STM8S :: compl1(Item *ip0, Item *ip1)
{
	if ( same(ip0, ip1) )
	{
		compl1(ip0);
		return;
	}

	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	char *reg1 = setPtr(ip1, 0);
	char *reg0 = setPtr(ip0, reg1? 1: 0);
	char *load = farAcce(ip1)? _LDF: _LD;
	if ( (reg0 && reg1) || farAcce(ip1) || size0 == 1 || size1 == 1 )
	{
		for (int i = 0; i < size0; i++)
		{
			if ( i < size1 ) {
				ASM_CODE2(load, REG_A, acceItem(ip1, i, 1, reg1));
				ASM_CODE1(_CPL, REG_A);
			}
			else if ( i == size1 )
				ASM_CODE2(_LD, REG_A, 255, '#');

			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
		return;
	}

	char *r = (reg0 || reg1)? REG_Y: REG_X;
	for (int i = 0; i < size0; i++)
	{
		if ( (i+1) < size1 && (i+1) < size0 )
		{	// 16-bit operation
			ASM_CODE2(_LDW, r, acceItem(ip1, i, 2, reg1));
			ASM_CODE1(_CPLW, r);
			ASM_CODE2(_LDW, acceItem(ip0, i, 2, reg0), r);
			i++;
			continue;
		}

		if ( i < size1 ) {
			ASM_CODE2(load, REG_A, acceItem(ip1, i, 1, reg1));
			ASM_CODE1(_CPL, REG_A);
		}
		else if ( i == size1 )
			ASM_CODE2(_LD, REG_A, 255, '#');

		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: compl1FromBit(Item *ip0, Item *ip1)
{
	char *reg0 = setPtr(ip0, 0);
	char *s = STRBUF();
	sprintf(s, "%d, #%d", ip1->val.i>>3, ip1->val.i&7);

	ASM_CODE1(_CLR, REG_A);
	ASM_CODE2(_BTJT, s, (char*)".+1");
	ASM_CODE1(_DEC, REG_A);
	for (int i = 0; i < ip0->acceSize(); i++)
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
}

void STM8S :: compl1ToBit(Item *ip0, Item *ip1)
{
	int  size1 = ip1->acceSize();
	char *reg1 = setPtr(ip1, 0);
	char *load = farAcce(ip1)? _LDF: _LD;
	for (int i = 0; i < size1; i++)
	{
		if ( i == 0 || farAcce(ip1) )
			ASM_CODE2(load, REG_A, acceItem(ip1, i, 1, reg1));
		else
			ASM_CODE2(_OR, REG_A, acceItem(ip1, i, 1, reg1));

		if ( farAcce(ip1) && i > 0 )
		{
			ASM_CODE2(_OR, REG_A, 0);
			if ( (i+1) < size1 )
				ASM_CODE2(_OR, 0, REG_A);
		}
	}

	ASM_CODE2(_SUB, REG_A, 1, '#');
	ASM_CODE2(_BCCM, ip0->val.i>>3, ip0->val.i&7, '#');
}

void STM8S :: notop(Item *ip0)
{
	int  size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	if ( size0 == 1 )
	{
		ASM_CODE1(_CLR, REG_A);
		ASM_CODE1(_TNZ, acceItem(ip0, 0, 1, reg0));
		ASM_CODE1(_JRNE, ".+1");
		ASM_CODE1(_DEC, REG_A);
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		return;
	}

	char *r = reg0? REG_Y: REG_X;
	ASM_CODE1(_CLRW, r);
	for (int i = 0; i < size0; i++) {
		char *inst = i? _OR: _LD;
		ASM_CODE2(inst, REG_A, acceItem(ip0, i, 1, reg0));
	}
	ASM_CODE1(_JRNE, reg0? ".+2": ".+1");
	ASM_CODE1(_DECW, r);
	for (int i = 0; i < size0; i++)
	{
		if ( (i+1) < size0 )
			ASM_CODE2(_LDW, acceItem(ip0, i++, 2, reg0), r);
		else
		{
			ASM_CODE2(_LD, REG_A, reg0? REG_YL: REG_XL);
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: notop(Item *ip0, Item *ip1)
{
	if ( same(ip0, ip1) )
	{
		notop(ip0);
		return;
	}

	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	char *reg1= setPtr(ip1, 0);
	char *r16 = reg1? REG_Y: REG_X;
	ASM_CODE1(_CLRW, r16);
	for (int i = 0; i < size1; i++)
	{
		if ( farAcce(ip1) ) {
			ASM_CODE2(_LDF, REG_A, acceItem(ip1, i, i, reg1));
			if ( i )
				ASM_CODE2(_OR, REG_A, 0);
			ASM_CODE2(_LD, 0, REG_A);
		}
		else
			ASM_CODE2(i? _OR:_LD, REG_A, acceItem(ip1, i, 1, reg1));
	}
	ASM_CODE1(_JRNE, reg1? ".+2": ".+1");
	ASM_CODE1(_DECW, r16);
	char *reg0 = setPtr(ip0, r16? 0: 1);
	ASM_CODE2(_LD, REG_A, reg1? REG_YL: REG_XL);
	for (int i = 0; i < size0; i++)
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
}