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

static bool selectBit(int n, int *b);
static bool deselectBit(int n, int *b);

void STM8S :: add(Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();

	if ( ip1->type == CON_ITEM )
	{
		incValue(ip0, ip1->val.i);
		return;
	}

	if ( !farAcce(ip1) && size0 >= 2 && size1 >= 2 &&
		 (isGeneric(ip0) || isGeneric(ip1)) )
	{
		add16(ip0, ip1);
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	char *inst = _ADD;
	for (int i = 0; i < size0; i++, inst = _ADC) {
		if ( i < size1 ) {
			if ( farAcce(ip1) )
				ASM_CODE2(_LDF, REG_A, acceItem(ip1, i, 1, reg1));
			else if ( LOCAL_ADDR(ip1) ) {
				if ( i == 0 ) {
					loadLocalAddr(ip1, reg0? REG_Y: REG_X);
					ASM_CODE2(_LD, REG_A, reg0? REG_YL: REG_XL);
				}
				else if ( i == 1 )
					ASM_CODE2(_LD, REG_A, reg0? REG_YH: REG_XH);
				else
					ASM_CODE1(_CLR, REG_A);
			}
			else
				ASM_CODE2(_LD , REG_A, acceItem(ip1, i, 1, reg1));

			if ( size1 < size0 && ip1->acceSign() && (i+1) == size1 ) {
				ASM_CODE1(_PUSH, REG_A);
				frameOffset++;
			}
		}
		else if ( !ip1->acceSign() )
			ASM_CODE1(_CLR, REG_A);
		else {
			if ( i == size1 ) {
				ASM_CODE1(_POP, REG_A),	frameOffset--;
				call((char*)"_extendRegA");
				if ( (i+1) < size0 )
					ASM_CODE1(_PUSH, REG_A), frameOffset++;
			}
			else if ( (i+1) < size0 )
				ASM_CODE2(_LD, REG_A, (char*)"(1, %SP)");
			else {
				ASM_CODE1(_POP, REG_A);
				frameOffset--;
			}
		}

		ASM_CODE2(inst, REG_A, acceItem(ip0, i, 1, reg0));
		ASM_CODE2(_LD,  acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: add16(Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	char *reg0 = setPtr2(ip0, 0);
	char *reg1 = setPtr2(ip1, reg0? 1: 0);
	char *r16;
	bool sign_in_rega = false;

	for (int i = 0; i < size0; i++)
	{
		if ( (i+1) < size0 && ((i+1) < size1 || ip1->type == IMMD_ITEM) ) {
			if ( isGeneric(ip0) ) {
				if ( reg1 && (size1-i) > 2 && (size0-i) > 2 ) {
					ASM_CODE2(_LDW, r16 = REG_Y, reg1);
					ASM_CODE2(_LDW, r16, acceItem(ip1, i, 2, r16));
				}
				else if ( LOCAL_ADDR(ip1) ) {
					if ( i == 0 )
						loadLocalAddr(ip1, r16 = REG_X);
					else
						ASM_CODE1(_CLRW, r16 = REG_X);
				}
				else {
					r16 = reg1? reg1: REG_X;
					ASM_CODE2(_LDW, r16, acceItem(ip1, i, 2, reg1));
				}

				if ( size1 < size0 && (i+2) == size1 && ip1->acceSign() ) {
					ASM_CODE2(_LD, REG_A, strcmp(reg1, REG_X)? REG_YH: REG_XH);
					sign_in_rega = true;
				}

				if ( i ) {
					ASM_CODE1(_JRNC, strcmp(r16, REG_X)? ".+2": ".+1");
					ASM_CODE1(_INCW, r16);
				}
				ASM_CODE2(_ADDW, r16, acceItem(ip0, i, 2, reg0));
			}
			else {
				if ( reg0 ) {
					ASM_CODE2(_LDW, r16 = REG_Y, reg0);
					ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, r16));
				}
				else
					ASM_CODE2(_LDW, r16 = REG_X, acceItem(ip0, i, 2, reg0));

				if ( i ) {
					ASM_CODE1(_JRNC, strcmp(r16, REG_X)? ".+2": ".+1");
					ASM_CODE1(_INCW, r16);
				}
				ASM_CODE2(_ADDW, r16, acceItem(ip1, i, 2, reg1));
			}
			ASM_CODE2(_LDW, acceItem(ip0, i++, 2, reg0), r16);
		}
		else {
			if ( i < size1 )
				ASM_CODE2(_LD, REG_A, acceItem(ip1, i, 1, reg1));
			else if ( !ip1->acceSign() )	// unsigned ip1
				ASM_CODE1(_CLR, REG_A);
			else if ( i == size1 ) {		//
				if ( !sign_in_rega )
					ASM_CODE2(_LD, REG_A, acceItem(ip1, size1-1, 1, reg1));
				call((char*)"_extendRegA");
				if ( (i+1) < size0 )
					ASM_CODE1(_PUSH, REG_A), frameOffset++;
			}
			else if ( (i+1) < size0 )
				ASM_CODE2(_LD, REG_A, (char*)"(1 + %SP)");
			else
				ASM_CODE1(_POP, REG_A),	frameOffset--;

			ASM_CODE2(_ADC, REG_A, acceItem(ip0, i, 1, reg0));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: sub(Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();

	if ( ip1->type == CON_ITEM )
	{
		incValue(ip0, -ip1->val.i);
		return;
	}

	if ( !farAcce(ip1) && size1 >= size0 && isGeneric(ip1) )
	{
		sub16(ip0, ip1);
		return;
	}

	if ( farAcce(ip1) || LOCAL_ADDR(ip1) || (size0 > size1 && ip1->acceSign()) )
	{
		Item *ip = pushStack(ip1, size0);
		sub(ip0, ip);
		popStack(size0);
		delete ip;
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	char *inst = _SUB;
	for (int i = 0; i < size0; i++, inst = _SBC)
	{
		ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
		if ( i < size1 )
			ASM_CODE2(inst, REG_A, acceItem(ip1, i, 1, reg1));
		else
			ASM_CODE2(inst, REG_A, (char*)"#0");
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: sub16(Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
//	int  size1 = ip1->acceSize();
	char *reg0 = setPtr2(ip0, 0);
	char *r16  = reg0? REG_Y: REG_X;
	for (int i = 0; i < size0; i++) {
		if ( (i+1) < size0 ) {
			if ( reg0 ) {
				ASM_CODE2(_LDW, r16, reg0);
				ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, r16));
			}
			else
				ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, reg0));

			if ( i > 0 ) {
				ASM_CODE1(_JRNC, strcmp(r16, REG_X)? ".+2": ".+1");
				ASM_CODE1(_DECW, r16);
			}
			ASM_CODE2(_SUBW, r16, acceItem(ip1, i, 2));
			ASM_CODE2(_LDW, acceItem(ip0, i++, 2, reg0), r16);
		}
		else {
			ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
			ASM_CODE2(i? _SBC: _SUB, REG_A, acceItem(ip1, i, 1));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: andorxor_bit(int code, Item *ip0, Item *ip1)
{
	if ( ip1->type == CON_ITEM ) {
		switch ( code )
		{
			case AND_ASSIGN:
				if ( ip1->val.i == 0 )	mov(ip0, ip1);
				break;

			case OR_ASSIGN:
				if ( ip1->val.i != 0 )	mov(ip0, ip1);
				break;

			case XOR_ASSIGN:
				if ( ip1->val.i != 0 )
					ASM_CODE2(_BCPL, ip0->val.i >> 3, ip0->val.i & 7, '#');
		}
	}
	else {
		char *s = STRBUF();
		sprintf(s, "0x%x, #%d", ip1->val.i >> 3, ip1->val.i & 7);

		switch ( code )
		{
			case AND_ASSIGN:
				ASM_CODE2(_BTJT, s, (char*)".+4");
				ASM_CODE2(_BRES, ip0->val.i >> 3, ip0->val.i & 7, '#');
				break;

			case OR_ASSIGN:
				ASM_CODE2(_BTJF, s, (char*)".+4");
				ASM_CODE2(_BSET, ip0->val.i >> 3, ip0->val.i & 7, '#');
				break;

			case XOR_ASSIGN:
				ASM_CODE2(_BTJF, s, (char*)".+4");
				ASM_CODE2(_BCPL, ip0->val.i >> 3, ip0->val.i & 7, '#');
		}
	}
}

void STM8S :: andorxor(int code, Item *ip0, int n)
{
	int size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	for (int i = 0; i < size0; i++)
	{
		int  b, v = (n >> (i*8)) & 0xff;
		char *inst = (code == AND_ASSIGN)? _AND:
					 (code == XOR_ASSIGN)? _XOR: _OR;
		bool dir_access = ip0->type == DIR_ITEM || (ip0->type == ID_ITEM && !isLocal(ip0));

		if ( code == AND_ASSIGN && deselectBit(v, &b) && dir_access )
		{
			ASM_CODE2(_BRES, acceItem(ip0, i, 1, reg0), b, '#');
			continue;
		}
		if ( code == OR_ASSIGN && selectBit(v, &b) && dir_access )
		{
			ASM_CODE2(_BSET, acceItem(ip0, i, 1, reg0), b, '#');
			continue;
		}
		if ( code == XOR_ASSIGN && selectBit(v, &b) && dir_access )
		{
			ASM_CODE2(_BCPL, acceItem(ip0, i, 1, reg0), b, '#');
			continue;
		}
		switch ( v )
		{
			case 0x00:
				if ( code == AND_ASSIGN )
					ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
				break;
			case 0xff:
				if ( code == XOR_ASSIGN )
					ASM_CODE1(_CPL, acceItem(ip0, i, 1, reg0));
				else if ( code == OR_ASSIGN ) {
					ASM_CODE2(_LD, REG_A, v, '#');
					ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
				}
				break;
			default:
				ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
				ASM_CODE2(inst, REG_A, v, '#');
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: andorxor(int code, Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();

	if ( IS_BF_VAR(ip0->attr) )
	{
		andorxor_bf(code, ip0, ip1);
		return;
	}

	if ( ip0->isBitVal() )
	{
		andorxor_bit(code, ip0, ip1);
		return;
	}

	if ( ip1->type == CON_ITEM )
	{
		andorxor(code, ip0, ip1->val.i);
		return;
	}

	if ( farAcce(ip1) ||
		 (size0 > size1 && ip1->acceSign()) || LOCAL_ADDR(ip1) )
	{
		Item *ip = pushStack(ip1, size0);
		andorxor(code, ip0, ip);
		popStack(size0);
		delete ip;
		return;
	}
	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	char *inst = (code == AND_ASSIGN)? _AND:
				 (code == XOR_ASSIGN)? _XOR: _OR;
	for (int i = 0; i < size0; i++)
	{
		if ( i < size1 ) {
			ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
			ASM_CODE2(inst, REG_A, acceItem(ip1, i, 1, reg1));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
		else if ( code == AND_ASSIGN )
			ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
	}
}

void STM8S :: andorxor_bf(int code, Item *ip0, Item *ip1)
{
	int size0 = BF_SIZE(ip0->attr);
	int shift = BF_OFFSET(ip0->attr);
	uint8_t mask  = ((1 << size0) - 1) << shift;
	char *inst = (code == AND_ASSIGN)? _AND:
				 (code == XOR_ASSIGN)? _XOR: _OR;

	if ( ip1->type == CON_ITEM )
	{
		uint8_t n = (ip1->val.i << shift) & mask;

		if ( (code == AND_ASSIGN && n == mask) || (code != AND_ASSIGN && n == 0) )
			return;

		char *reg0 = setPtr(ip0, 0);
		bool dir_access = ip0->type == DIR_ITEM || (ip0->type == ID_ITEM && !isLocal(ip0));

		if ( !reg0 && dir_access && size0 == 1 ) // 1-bit field operation...
		{
			inst = (code == AND_ASSIGN)? _BRES:
				   (code == XOR_ASSIGN)? _BCPL: _BSET;

			ASM_CODE2(inst, acceItem(ip0, 0, 1, reg0), shift, '#');
		}
		else
		{
			ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
			if ( code == AND_ASSIGN ) n |= ~mask;

			ASM_CODE2(inst, REG_A, n, '#');
			ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		}
	}
	else
	{
		Item *acc = accItem(1);
		mov(acc, ip1);
		char *reg0 = setPtr(ip0, 0);

		ASM_CODE2(_LD, REG_A, 0);
		for (int i = 0; i < shift; i++)
			ASM_CODE1(_SLL, REG_A);

		if ( code == AND_ASSIGN )
			ASM_CODE2(_OR, REG_A, ~mask, '#');
		else
			ASM_CODE2(_AND, REG_A, mask, '#');

		ASM_CODE2(inst, REG_A, acceItem(ip0, 0, 1, reg0));
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		delete acc;
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

static bool deselectBit(int n, int *b)
{
	for (*b = 0; *b < 8; (*b)++)
		if ( ((n & 0xff) ^ 0xff) == (1<<(*b)) )
			return true;

	return false;
}