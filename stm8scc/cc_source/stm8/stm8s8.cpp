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

#define Z_FLAG_NEEDED(c)	(c == P_JGT || c == P_JLE)

static int convertCompare(int code)
{
	switch ( code )
	{
		case P_JGE:	return P_JLE;
		case P_JLT:	return P_JGT;
		case P_JGT:	return P_JLT;
		case P_JLE:	return P_JGE;
		default:	return code;
	}
}

bool signedItem1(Item *ip1, int size0)
{
	if ( ip1->type != CON_ITEM ) return ip1->acceSign();
	if ( ip1->acceSign() ) return true;
	if ( ip1->acceSize() >= size0 ) {
		return (ip1->val.i & (1 << (8*size0-1)))? false: true;
	}
	return true;
}

void STM8S :: cmpJump(int code, Item *ip0, Item *ip1, Item *ip2)
{
	if ( ip0->type == CON_ITEM ) {
		cmpJump(convertCompare(code), ip1, ip0, ip2);
		return;
	}

	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int size  = (size0 < size1)? size1: size0;
	bool signed_cmp = (ip0->acceSign() && signedItem1(ip1, size0));
	bool saveFlagZ  = Z_FLAG_NEEDED(code);

	if ( !(farAcce(ip0) || LOCAL_ADDR(ip0)) &&
		  (farAcce(ip1) || LOCAL_ADDR(ip1))	) {
		cmpJump(convertCompare(code), ip1, ip0, ip2);
		return;
	}
	if ( farAcce(ip1) || LOCAL_ADDR(ip1) ||
		 (size0 > size1 && ip1->type != CON_ITEM) ) {
		Item *acc = storeToACC(ip1, size);
		cmpJump(code, ip0, acc, ip2);
		delete acc;
		return;
	}
	if ( isGeneric(ip0) && !isGeneric(ip1) &&
		 (size0 >= size1 || ip0->type == CON_ITEM) ) {
		cmpJump(convertCompare(code), ip1, ip0, ip2);
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	if ( size0 == 2 && size == 2 && isGeneric(ip1) ) {
		if ( LOCAL_ADDR(ip0) )
			loadLocalAddr(ip0, REG_X);
		else
			ASM_CODE2(_LDW, REG_X, acceItem(ip0, 0, 2, reg0));

		ASM_CODE2(_CPW, REG_X, acceItem(ip1, 0, 2));
		cmpJump(code, ip2, signed_cmp);
	}
	else {
		char *r16 = (reg0 || reg1)? REG_Y: REG_X;
		char *r8  = (reg1 != NULL)? REG_YL: REG_XL;
		if ( LOCAL_ADDR(ip0) )	loadLocalAddr(ip0, r16);

		for (int i = 0; i < size; i++)
		{
			if ( i < size0 ) {
				if ( LOCAL_ADDR(ip0) ) {
					char *r8 = (reg0 || reg1)? (i? REG_YH: REG_XH):
											   (i? REG_YL: REG_XL);
					ASM_CODE2(_LD, REG_A, r8);
				}
				else if ( farAcce(ip0) )
					ASM_CODE2(_LDF, REG_A, acceItem(ip0, i, 1, reg0));
				else
					ASM_CODE2(_LD,  REG_A, acceItem(ip0, i, 1, reg0));

				if ( size0 < size && ip0->acceSign() && (i+1) == size0 )
					ASM_CODE2(_LD, r8, REG_A);
			}
			else if ( !ip0->acceSign() )
				ASM_CODE1(_CLR, REG_A);
			else {
				ASM_CODE2(_LD, REG_A, r8);
				if ( i == size0 ) {
					call((char*)"_extendRegA");
					if ( (i+1) < size )
						ASM_CODE2(_LD, r8, REG_A);
				}
			}

			ASM_CODE2(i? _SBC: _SUB, REG_A, acceItem(ip1, i, 1, reg1));

			if ( size > 1 && saveFlagZ && (i+1) < size )
			{
				ASM_CODE1(_PUSH, REG_A);
				frameOffset++;
			}
		}

		if ( size > 1 && saveFlagZ )
		{
			switch ( size )
			{
				case 2:	call((char*)"_decodeZflag8");	break;
				case 3:	call((char*)"_decodeZflag16");	break;
				case 4:	call((char*)"_decodeZflag24");	break;
			}
			popStack(size-1);
		}
		cmpJump(code, ip2, signed_cmp);
	}
}

void STM8S :: cmpJump(int code, Item *ip, bool signed_cmp)
{
	int  jr_code = 0;
	switch ( code )
	{
		case P_JGE:	jr_code = signed_cmp? J_CODE_SGE: J_CODE_UGE;	break;
		case P_JGT:	jr_code = signed_cmp? J_CODE_SGT: J_CODE_UGT;	break;
		case P_JLE:	jr_code = signed_cmp? J_CODE_SLE: J_CODE_ULE;	break;
		case P_JLT:	jr_code = signed_cmp? J_CODE_SLT: J_CODE_ULT;	break;
	}
	ASM_CODE2(__JP, jr_code, ip->val.s);
}
