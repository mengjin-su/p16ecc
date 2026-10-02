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
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../const.h"
#include "../nlist.h"
#include "stm8s.h"
#include "stm8s_reg.h"
#include "stm8s_inst.h"
#include "stm8s_asm.h"

void STM8S :: mov(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();

	if ( IS_BF_VAR(ip0->attr) )
	{
		moveToBF(ip0, ip1);
		return;
	}
	if ( IS_BF_VAR(ip1->attr) )
	{
		moveFromBF(ip0, ip1);
		return;
	}

	if ( equivalent(ip0, ip1) )
		return;
	
	if ( size0 > 4 )	// big data block move.
	{
		movBlock(ip0, ip1);
		return;
	}	

	if ( (ip0->type == ACC_ITEM || ip0->type == TEMP_ITEM) && related(ip0, ip1) && size1 >= size0 )
	{	// special move operation for big-endian structure (moving starts at higher byte)
		for (int i = size0; i > 0; i--) {
			if ( i >= 2 ) {	// move 2 bytes at a time
				ASM_CODE2(_LDW, REG_X, acceItem(ip1, i-2, 2));
				ASM_CODE2(_LDW, acceItem(ip0, i-2, 2), REG_X);
				i--;
			}
			else if ( ip0->type != ACC_ITEM ) {
				ASM_CODE2(_LD, REG_A, acceItem(ip1, i-1, 1));
				ASM_CODE2(_LD, acceItem(ip0, i-1, 1), REG_A);
			}
			else
				ASM_CODE2(_MOV, acceItem(ip0, i-1, 1), acceItem(ip1, i-1, 1));
		}
		return;
	}

	if ( ip1->type == CON_ITEM || ip1->type == IMMD_ITEM || ip1->type == LBL_ITEM )
	{
		movImmd(ip0, ip1);
		return;
	}

	if ( isDirect(ip0) && isDirect(ip1) && size0 == 1 )
	{
		ASM_CODE2(_MOV, acceItem(ip0, 0, 1), acceItem(ip1, 0, 1));
		return;
	}

	bool sign1 = ip1->acceSign();
	bool flag0 = (size0 <= 2 && size0 <= size1);
	bool flag1 = (size1 <= 2 && size0 >= size1);
	char *reg0 = flag0? setPtr2(ip0, 0)			: setPtr(ip0, 0);
	char *reg1 = flag1? setPtr2(ip1, reg0? 1: 0): setPtr(ip1, reg0? 1: 0);

	if ( farAcce(ip1) )
	{
		for (int i = 0; i < size0; i++)
		{
			if ( i >= size1 )
			{
				if ( sign1 )
				{
					if ( size1 == i ) call((char*)"_extendRegA");
					ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
				}
				else
					ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
			}
			else
			{
				ASM_CODE2(_LDF, REG_A, acceItem(ip1, i, 1, reg1));
				ASM_CODE2(_LD , acceItem(ip0, i, 1, reg0), REG_A);
			}
		}
		return;
	}

	for (int i = 0; i < size0; i++)
	{
		if ( i >= size1 )
		{
			if ( size1 == i && sign1 )
				call((char*)"_extendSign");

			if ( sign1 )
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			else
				ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
		}
		else if ( (size0 - i) > 1 && (size1 - i) > 1 && !(reg0 && reg1) )
		{
			char *r16 = (reg1 || reg0)? REG_Y: REG_X;
			if ( reg1 ) {
				if ( (i+2) >= size1 || (i+2) >= size0 )
					r16 = reg1;
				else
					ASM_CODE2(_LDW, r16, reg1);

				ASM_CODE2(_LDW, r16, acceItem(ip1, i, 2, r16));
			}
			else
				ASM_CODE2(_LDW, r16, acceItem(ip1, i, 2, reg1));

			ASM_CODE2(_LDW, acceItem(ip0, i, 2, reg0), r16);
			i++;
		}
		else if ( (size0 - i) == 2 && (size1 - i) > 1 && (reg0 && reg1) && ip1->bias == 0 )
		{
			ASM_CODE2(_LDW, reg1, acceItem(ip1, i, 2, reg1));
			ASM_CODE2(_LDW, acceItem(ip0, i, 2, reg0), reg1);
			i++;
		}
		else
		{
			ASM_CODE2(_LD, REG_A, acceItem(ip1, i, 1, reg1));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: movImmd(Item *ip0, Item *ip1)
{
    int size0 = ip0->acceSize();
	char *reg0 = (size0 == 2)? setPtr2(ip0, 0): setPtr(ip0, 0);;

	if ( LOCAL_ADDR(ip1) )
	{
		char *r16 = reg0? REG_Y : REG_X;
		char *rl  = reg0? REG_YL: REG_XL;
		loadLocalAddr(ip1, r16);
		if ( size0 == 1 ) {
			ASM_CODE2(_LD, REG_A, rl);
			ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		}
		else
		{
			ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), r16);
			for (int i = 2; i < size0; i++)
				ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
		}
		return;
	}

	if ( isDirect(ip0) && size0 == 1 )
	{
		if ( ip1->type == CON_ITEM && !(ip1->val.i & 0xff) )
			ASM_CODE1(_CLR, acceItem(ip0, 0, 1));
		else
			ASM_CODE2(_MOV, acceItem(ip0, 0, 1), acceItem(ip1, 0, 1));
		return;
	}

	for (int i = 0; i < size0; i++)
	{
		int n = ip1->val.i >> (i*8);
		if ( ip1->type == CON_ITEM && (n & 0xff) == 0 )
		{
			ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
		}
		else if ( (size0 - i) > 1 )
		{
			char *r16 = reg0? REG_Y: REG_X;
			ASM_CODE2(_LDW, r16, acceItem(ip1, i, 2));
			ASM_CODE2(_LDW, acceItem(ip0, i, 2, reg0), r16);
			i++;
		}
		else
		{
			ASM_CODE2(_LD, REG_A, acceItem(ip1, i, 1));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: movBlock(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int lbl = pcoder->getLbl();

	setPtr(0, ip0);
	setPtr(2, ip1);
	ASM_CODE2(_LDW, REG_X, size0, '#');
	ASM_LABL(lbl);
	ASM_CODE1(_DECW, REG_X);

	char *inst = farAcce(ip1)? _LDF: _LD;
	ASM_CODE2(inst, REG_A, (char*)"([2], %X)");
	ASM_CODE2(_LD, (char*)"([0], %X)", REG_A);

	ASM_CODE1(_TNZW, REG_X);
	ASM_CODE_LBL(_JRNE, lbl);
}
