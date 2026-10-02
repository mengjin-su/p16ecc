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

static int logicAssign(int code)
{
	switch ( code )
	{
		case '&':	return AND_ASSIGN;
		case '|':	return OR_ASSIGN;
		case '^':	return XOR_ASSIGN;
	}
	return 0;
}

void STM8S :: andorxor(int code, Item *ip0, Item *ip1, Item *ip2)
{
	if ( same(ip1, ip2) )
	{
		mov(ip0, ip1);
		return;
	}
	if ( same(ip0, ip1) )
	{
		andorxor(logicAssign(code), ip0, ip2);
		return;
	}
	if ( same(ip0, ip2) )
	{
		andorxor(logicAssign(code), ip0, ip1);
		return;
	}
	if ( ip1->type == CON_ITEM )
	{
		andorxor(code, ip0, ip2, ip1->val.i);
		return;
	}
	if ( ip2->type == CON_ITEM )
	{
		andorxor(code, ip0, ip1, ip2->val.i);
		return;
	}
	if ( ((farAcce(ip1) || LOCAL_ADDR(ip1)) && (farAcce(ip2) || LOCAL_ADDR(ip2))) ||
		 (usePtr(ip0) && (usePtr(ip1) || farAcce(ip1) || LOCAL_ADDR(ip1)) &&
						 (usePtr(ip2) || farAcce(ip2) || LOCAL_ADDR(ip2)) )			)
	{
		mov(ip0, ip1);
		andorxor(logicAssign(code), ip0, ip2);
		return;
	}
	if ( farAcce(ip2) || LOCAL_ADDR(ip2) ) {
		andorxor(code, ip0, ip2, ip1);
		return;
	}
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	int  size2 = ip2->acceSize();
	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	char *reg2 = setPtr(ip2, (reg0||reg1)? 1: 0);
	char *r16  = (reg0||reg1||reg2)? REG_Y: REG_X;
	if ( LOCAL_ADDR(ip1) ) loadLocalAddr(ip1, r16);
	for (int i = 0; i < size0; i++) {
		if ( ((i >= size1 || i >= size2) && code == '&') ||
			  (i >= size1 && i >= size2) ) 				{
			ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
			continue;
		}
		if ( i >= size1 ) {
			ASM_CODE2(_LD, REG_A, acceItem(ip2, i, 1, reg2));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			continue;
		}
		if ( i >= size2 ) {
			char *ld = farAcce(ip1)? _LDF: _LD;
			ASM_CODE2(ld, REG_A, acceItem(ip1, i, 1, reg1));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			continue;
		}
		char *inst = (code == '|')? _OR:
					 (code == '&')? _AND: _XOR;
		if ( LOCAL_ADDR(ip1) ) {
			char *r8 = i? (strcmp(r16, REG_X)? REG_YH: REG_XH):
						  (strcmp(r16, REG_X)? REG_YL: REG_XL);
			ASM_CODE2(_LD, REG_A, r8);
		}
		else {
			char *ld = farAcce(ip1)? _LDF: _LD;
			ASM_CODE2(ld, REG_A, acceItem(ip1, i, 1, reg1));
		}
		ASM_CODE2(inst, REG_A, acceItem(ip2, i, 1, reg2));
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: andorxor(int code, Item *ip0, Item *ip1, int n)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	char *r16  = reg0? REG_Y: REG_X;
	if ( LOCAL_ADDR(ip1) ) loadLocalAddr(ip1, r16);
	for (int i = 0; i < size0; i++)
	{
		int num = (n >> (i*8)) & 0xff;
		char *ld = farAcce(ip1)? _LDF: _LD;
		switch ( code )
		{
			case '|':
			case '^':
				if ( i < size1 ) {
					if ( LOCAL_ADDR(ip1) ) {
						char *r8 = i? (strcmp(r16, REG_X)? REG_YH: REG_XH):
									  (strcmp(r16, REG_X)? REG_YL: REG_XL);
						ASM_CODE2(_LD, REG_A, r8);
					}
					else
						ASM_CODE2(ld, REG_A, acceItem(ip1, i, 1, reg1));
					if ( num ) {
						char *op = (code == '|')? _OR: _XOR;
						ASM_CODE2(op, REG_A, num, '#');
					}
				}
				else if ( num )
					ASM_CODE2(_LD, REG_A, num, '#');
				else {
					ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
					continue;
				}
				break;

			case '&':
				if ( i >= size1 || num == 0 ) {
					ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
					continue;
				}
				if ( LOCAL_ADDR(ip1) ) {
					char *r8 = i? (strcmp(r16, REG_X)? REG_YH: REG_XH):
								  (strcmp(r16, REG_X)? REG_YL: REG_XL);
					ASM_CODE2(_LD, REG_A, r8);
				}
				else
					ASM_CODE2(ld, REG_A, acceItem(ip1, i, 1, reg1));
				if ( num != 0xff )
					ASM_CODE2(_AND, REG_A, num, '#');
				break;
		}

		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: pragma(Item *ip0, Item *ip1)
{
	if ( strcmp(ip0->val.s, "acc_save") == 0 )
	{
		if ( ip1 && ip1->type == CON_ITEM )
		{
			accSave = ip1->val.i;
			if ( accSave > 8 ) accSave = 8;
		}
	}
	else
		errPrint("unknown #pragma id!", ip0->val.s);
}

void STM8S :: djnz(int code, Item *ip0, Item *ip1)
{
	char *reg0 = setPtr(ip0, 0);
	char *inst = (code == P_DJNZ)? _DEC: _INC;
	ASM_CODE1(inst, acceItem(ip0, 0, 1, reg0));
	ASM_CODE2(__JP, J_CODE_NE, ip1->val.s);
}