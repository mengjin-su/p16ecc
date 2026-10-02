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

void STM8S :: jzjnz(int code, Item *ip0, Item *ip1)
{
	if ( IS_BF_VAR(ip0->attr) )
	{
		Item *ip;
		
		if ( BF_SIZE(ip0->attr) == 1 )	// single bit test
		{
			ip = intItem(BF_OFFSET(ip0->attr));
			jbzjbnz((code == P_JZ)? P_JBZ: P_JBNZ, ip0, ip, ip1);
		}
		else
		{
			ip = intItem(0);
			jeqjne((code == P_JZ)? P_JEQ: P_JNE, ip0, ip, ip1);
		}
		delete ip;
		return;
	}

	int  size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	int  jr_code;
	
	if ( ip0->type == IMMD_ITEM || ip0->type == LBL_ITEM )
	{
		if ( code == P_JNZ )
			ASM_CODE1(__JP, ip1->val.s);
		return;
	}

	for (int i = 0; i < size0; i++)
	{
		if ( farAcce(ip0) ) {
			ASM_CODE2(_LDF, REG_A, acceItem(ip0, i, 1, reg0));
			if ( i )
				ASM_CODE2(_OR, REG_A, 0);
			if ( size0 > 1 && (i+1) < size0 )
				ASM_CODE2(_LD, 0, REG_A);
		}
		else if ( size0 == 2 ) {
			ASM_CODE2(_LDW, REG_X, acceItem(ip0, i, 2, reg0));
			i++;
		}
		else if ( i == 0 )
			ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
		else
			ASM_CODE2(_OR, REG_A, acceItem(ip0, i, 1, reg0));
	}

	jr_code = (code == P_JZ)? J_CODE_EQ: J_CODE_NE;
	ASM_CODE2(__JP, jr_code, ip1->val.s);
}

void STM8S :: jbzjbnz(int code, Item *ip0, Item *ip1, Item *ip2)
{
	char *reg0 = setPtr(ip0, 0);
	int offset = ip1->val.i/8;
	int bit    = ip1->val.i%8;
	int jr_code;

	if ( isDirect(ip0) ) {
		char *s = STRBUF();
		char *inst = (code == P_JBZ)? __JBZ:__JBNZ;
		sprintf(s, "%s, %d", acceItem(ip0, offset, 1, reg0), bit);
		ASM_CODE2(inst, s, ip2->val.s);
		return;
	}

	if ( ip0->type == IMMD_ITEM || ip0->type == LBL_ITEM )
	{
		if ( isLocal(ip0) ) {
			ASM_CODE2(_LDW, REG_X, REG_SP);
			ASM_CODE2(_ADDW, REG_X, localOffset(ip0), '#');
			if ( offset == 0 ) ASM_CODE2(_LD, REG_A, REG_XL);
			if ( offset == 1 ) ASM_CODE2(_LD, REG_A, REG_XH);
			if ( offset >= 2 ) {
				if ( code == P_JBZ )
					ASM_CODE1(__JP, ip2->val.s);
				return;
			}
			ASM_CODE2(_AND, REG_A, 1 << bit, '#');
		}
		else {
			char *s = STRBUF();
			sprintf(s, "(%s)&%d", itemStr(ip0, offset, 1, reg0), 1 << bit);
			ASM_CODE2(_LD, REG_A, s, '#');
		}

		jr_code = (code == P_JBZ)? J_CODE_EQ: J_CODE_NE;
		ASM_CODE2(__JP, jr_code, ip2->val.s);
		return;
	}

	char *inst = farAcce(ip0)? _LDF: _LD;
	ASM_CODE2(inst, REG_A, acceItem(ip0, offset, 1, reg0));

	if ( bit == 7 )
		jr_code = (code == P_JBZ)? J_CODE_PL: J_CODE_MI;
	else
	{
		ASM_CODE2(_AND, REG_A, 1<<bit, '#');
		jr_code = (code == P_JBZ)? J_CODE_EQ: J_CODE_NE;
	}
	ASM_CODE2(__JP, jr_code, ip2->val.s);
}

void STM8S :: pcall(Item *ip0)
{
	char *reg0 = setPtr(ip0, 0);
	char *s = STRBUF();
	if ( farAcce(ip0) )
	{
		for (int i = 3; i--;) {
			ASM_CODE2(_LDF, REG_A, acceItem(ip0, i, 1, reg0));
			ASM_CODE2(_LD,  2-i, REG_A);
		}
		sprintf(s, "[0]");
	}
	else if ( isLocal(ip0) && EXT_MODE )
	{
		for (int i = 3; i--;) {
			ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
			ASM_CODE2(_LD, 2-i, REG_A);
		}
		sprintf(s, "[0]");

	}
	else if ( isLocal(ip0) )
	{
		ASM_CODE2(_LDW, REG_X, acceItem(ip0, 0, 2, reg0));
		sprintf(s, "(%s)", REG_X);
	}
	else
	{
		sprintf(s, "[%s]", ip0->val.s);
	}
	call(s);
}

void STM8S :: jzjnz_incdec(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int  size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	char *r16  = reg0? REG_Y: REG_X;
	int  n = ip1->val.i;
	if ( code == P_JZ_DEC || code == P_JNZ_DEC ) n = -n;

	if ( size0 == 1 )
	{
		ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
		if ( (n & 0xff) == 0x01 ) {
			ASM_CODE1(_INC, acceItem(ip0, 0, 1, reg0));
			ASM_CODE1(_TNZ, REG_A);
		}
		else if ( (n & 0xff) == 0xff ) {
			ASM_CODE1(_DEC, acceItem(ip0, 0, 1, reg0));
			ASM_CODE1(_TNZ, REG_A);
		}
		else {
			ASM_CODE2(_ADD, REG_A, n, '#');
			ASM_CODE2(_LD, acceItem(ip0, 0, 2, reg0), REG_A);
			ASM_CODE2(_SUB, REG_A, n, '#');
		}
	}
	else if ( size0 == 2 )
	{
		ASM_CODE2(_LDW, r16, acceItem(ip0, 0, 2, reg0));

		if ( (n & 0xffff) == 0x0001 )
			ASM_CODE1(_INCW, r16);
		else if ( (n & 0xffff) == 0xffff )
			ASM_CODE1(_DECW, r16);
		else
			ASM_CODE2(_ADDW, r16, n, '#');

		ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), r16);

		if ( (n & 0xffff) == 0x0001 )
			ASM_CODE1(_DECW, r16);
		else if ( (n & 0xffff) == 0xffff )
			ASM_CODE1(_INCW, r16);
		else
			ASM_CODE2(_SUBW, r16, n, '#');
	}
	else {
		for (int i = 0; i < size0; i++)
			ASM_CODE2(i? _OR: _LD, REG_A, acceItem(ip0, i, 1, reg0));

		ASM_CODE1(_PUSH, REG_CC); frameOffset++;

		for (int i = 0; i < size0; i++) {
			if ( (i+1) < size0 ) {
				if ( reg0 ) {
					ASM_CODE2(_LDW, r16, reg0);
					ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, r16));
				}
				else
					ASM_CODE2(_LDW, r16, acceItem(ip0, i, 2, reg0));

				if ( i ) {
					ASM_CODE1(_JRNC, reg0? ".+2": ".+1");
					ASM_CODE1(_INCW, r16);
				}

				if ( (n >> (i*8)) & 0xffff )
					ASM_CODE2(_ADDW, r16, n>>(i*8), '#');

				ASM_CODE2(_LDW, acceItem(ip0, i, 2, reg0), r16);
				i++;
			}
			else {
				ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
				ASM_CODE2(_ADC, REG_A, n >> (i*8), '#');
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			}
		}

		ASM_CODE1(_POP, REG_CC); frameOffset--;
	}

	int jr_code = (code == P_JZ_INC || code == P_JZ_DEC)? J_CODE_EQ: J_CODE_NE;
	ASM_CODE2(__JP, jr_code, ip2->val.s);
}