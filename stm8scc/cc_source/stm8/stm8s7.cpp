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

void STM8S :: jeqjne(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int size  = (size0 < size1)? size1: size0;

	if ( IS_BF_VAR(ip0->attr) )
	{
		if ( ip1->type != CON_ITEM )
			errPrint("SORRY - not support this compare!");
		else
		{
			size0 = BF_SIZE(ip0->attr);
			int shift = BF_OFFSET(ip0->attr);
			uint8_t mask = ((1 << size0) - 1) << shift;
			int n = (ip1->val.i << shift) & mask;

			if ( size0 == 1 )
			{
				code = (code == P_JEQ)? (n? P_JBNZ: P_JBZ): (n? P_JBZ: P_JBNZ);
				Item *ip = intItem(shift);
				jbzjbnz(code, ip0, ip, ip2);
				delete ip;
			}
			else
			{
				char *reg0 = setPtr(ip0, 0);
				if ( farAcce(ip0) )
					ASM_CODE2(_LDF, REG_A, acceItem(ip0, 0, 1, reg0));
				else
					ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));

				ASM_CODE2(_AND, REG_A, mask, '#');
				if ( n ) ASM_CODE2(_XOR, REG_A, n, '#');
				ASM_CODE2(__JP, (code == P_JEQ)? J_CODE_EQ: J_CODE_NE, ip2->val.s);
			}
		}
		return;
	}

	if ( (ip1->type == CON_ITEM && size0 < size1) || (ip0->type == CON_ITEM && size0 > size1) )
	{
		if ( code != P_JEQ )
			ASM_CODE1(__JP, ip2->val.s);
		return;
	}
	if ( !(farAcce(ip0) || LOCAL_ADDR(ip0)) && (farAcce(ip1) || LOCAL_ADDR(ip1)) )
	{
		jeqjne(code, ip1, ip0, ip2);
		return;
	}
	if ( farAcce(ip1) || LOCAL_ADDR(ip1) || (size0 > size1 && ip1->type != CON_ITEM) )
	{
		Item *ip = pushStack(ip1, size);
		jeqjne(code, ip0, ip, ip2, size);
		delete ip;
		return;
	}
	if ( isGeneric(ip0) && !isGeneric(ip1) && (size0 >= size1 || ip0->type == CON_ITEM) )
	{
		jeqjne(code, ip1, ip0, ip2);
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	int  lbl1  = pcoder->getLbl();
	int  lbl2  = pcoder->getLbl();
	char *r16;
	if ( LOCAL_ADDR(ip0) ) {
		r16 = reg1? REG_Y: REG_X;
		loadLocalAddr(ip0, r16);
	}
	for (int i = 0; i < size; i++) {
		if ( i < size0 ) {
			if ( farAcce(ip0) || LOCAL_ADDR(ip0) || !isGeneric(ip1) ||
				 (i+1) >= size0 || (i+1) >= size1 					   ) {
				char *ld = farAcce(ip0)? _LDF: _LD;
				if ( LOCAL_ADDR(ip0) )
					ASM_CODE2(_LD, REG_A, reg1? (i? REG_YH: REG_YL):
													(i? REG_XH: REG_XL));
				else
					ASM_CODE2(ld,  REG_A, acceItem(ip0, i, 1, reg0));
				ASM_CODE2(_CP, REG_A, acceItem(ip1, i, 1, reg1));
			}
			else {
				r16 = (reg0 && (i+2) < size0)? REG_Y: REG_X;
				char *rh = strcmp(r16, REG_X)? REG_YH: REG_XH;
				if ( reg0 && (i+2) < size0 ) {
					ASM_CODE2(_LDW, r16, reg0);
					ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, r16));
				}
				else if ( !LOCAL_ADDR(ip0) )
					ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, reg0));

				if ( size0 < size1 && ip0->acceSign() && (i+2) == size0 )
					ASM_CODE2(_LD, REG_A, rh);
				ASM_CODE2(_CPW, r16, acceItem(ip1, i++, 2));
			}
		}
		else if ( !ip0->acceSign() ) {
			if ( (i+1) < size1 && !reg1 )
				ASM_CODE2(_LDW, REG_X, acceItem(ip1, i++, 2));
			else
				ASM_CODE2(_LD, REG_A, acceItem(ip1, i, 1, reg1));
		}
		else {
			if ( i == size0 ) call((char*)"_extendRegA");
			ASM_CODE2(_CP, REG_A, acceItem(ip1, i, 1, reg1));
		}

		if ( (i+1) < size )
			ASM_CODE_LBL(_JRNE, (code == P_JEQ)? lbl2: lbl1);
	}
	ASM_LABL(lbl1);
	int jr_code = (code == P_JEQ)? J_CODE_EQ: J_CODE_NE;
	ASM_CODE2(__JP, jr_code, ip2->val.s);
	ASM_LABL(lbl2);
}

void STM8S :: jeqjne(int code, Item *ip0, Item *ip1, Item *ip2, int size1)
{
	int  size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	int  lbl   = pcoder->getLbl();
	char *r16  = REG_X;
	if ( LOCAL_ADDR(ip0) )
		loadLocalAddr(ip0, r16);
	for (int i = 0; i < size1; i++) {
		if ( i < size0 ) {
			if ( farAcce(ip0) || LOCAL_ADDR(ip0) ||
				 (i+1) >= size0 || (i+1) >= size1 ) {
				char *ld = farAcce(ip0)? _LDF: _LD;
				if ( LOCAL_ADDR(ip0) )
					ASM_CODE2(_LD, REG_A, (i? REG_XH: REG_XL));
				else
					ASM_CODE2(ld,  REG_A, acceItem(ip0, i, 1, reg0));
				ASM_CODE2(_CP, REG_A, acceItem(ip1, i, 1));
			}
			else {
				r16 = (reg0 && (i+2) < size0)? REG_Y: REG_X;
				char *rh = strcmp(r16, REG_X)? REG_YH: REG_XH;
				if ( reg0 && (i+2) < size0 ) {
					ASM_CODE2(_LDW, r16, reg0);
					ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, r16));
				}
				else if ( !LOCAL_ADDR(ip0) )
					ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, reg0));

				if ( size0 < size1 && ip0->acceSign() && (i+2) == size0 )
					ASM_CODE2(_LD, REG_A, rh);
				ASM_CODE2(_CPW, r16, acceItem(ip1, i++, 2));
			}
		}
		else if ( !ip0->acceSign() ) {
			if ( (i+1) < size1 )
				ASM_CODE2(_LDW, REG_X, acceItem(ip1, i++, 2));
			else
				ASM_CODE2(_LD, REG_A, acceItem(ip1, i, 1));
		}
		else {
			if ( i == size0 ) call((char*)"_extendRegA");
			ASM_CODE2(_CP, REG_A, acceItem(ip1, i, 1));
		}

		if ( (i+1) < size1 )
			ASM_CODE_LBL(_JRNE, lbl);	// pre-terminate if NEQ
	}
	ASM_LABL(lbl);
	popStack(size1);		// restore SP before jump
	ASM_CODE2(__JP, (code == P_JEQ)? J_CODE_EQ: J_CODE_NE, ip2->val.s);
}

void STM8S :: jeqjneBit(int code, Item *ip0, Item *ip1, Item *ip2)
{
	char *s = STRBUF();
	int  lbl1 = pcoder->getLbl();

	ASM_CODE0(_RCF);	// Carry <- 0
	sprintf(s, "%d, #%d, .+1", ip0->val.i >> 3, ip0->val.i&7);
	ASM_CODE1(_BTJF, s);
	ASM_CODE0(_SCF);	// Carry <- 1
	sprintf(s, "%d, #%d, .+1", ip1->val.i >> 3, ip1->val.i&7);
	ASM_CODE1(_BTJF, s);
	ASM_CODE0(_CCF);	// Carry ^= 1

	int jr_code = (code == P_JEQ)? J_CODE_NC: J_CODE_C;
	ASM_CODE2(__JP, jr_code, ip2->val.s);
	ASM_LABL(lbl1);
}