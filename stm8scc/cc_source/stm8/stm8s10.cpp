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

void STM8S :: mulAssign(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	char *func;
	switch ( size0 )
	{
		case 1:	mulAssign8(ip0, ip1);		return;
		case 2: func = (char*)"_mul16Ptr";	break;
		case 3: func = (char*)"_mul24Ptr";	break;
		case 4:	func = (char*)"_mul32Ptr";	break;
		default:return;
	}
	delete pushStack(ip1, size0);
	setPtr(ip0, REG_X);
	call(func);
	popStack(size0);
}

void STM8S :: mulAssign8(Item *ip0, Item *ip1)
{
	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	if ( LOCAL_ADDR(ip1) )
		loadLocalAddr(ip1, reg0? REG_Y: REG_X);
	else {
		char *ld = farAcce(ip1)? _LDF: _LD;
		ASM_CODE2(ld, REG_A, acceItem(ip1, 0, 1, reg1));
		ASM_CODE2(_LD, reg0? REG_YL: REG_XL, REG_A);
	}
	ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
	ASM_CODE2(_MUL, reg0? REG_Y: REG_X, REG_A);
	ASM_CODE2(_LD, REG_A, reg0? REG_YL: REG_XL);
	ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
}

void STM8S :: divmodAssign(int code, Item *ip0, Item *ip1)
{
	if ( same(ip0, ip1) ) {
		Item *ip = intItem((code==DIV_ASSIGN)? 1: 0);
		mov(ip0, ip);
		delete ip;
		return;
	}
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign0 = ip0->acceSign();
	bool sign1 = ip1->acceSign();
	int  size  = (size0 >= size1)? size0: size1;
	char *func;

	switch ( size )
	{
		case 1:
		case 2:	if ( size1 == 2 ) divmod16(code, ip0, ip1);
				else			  divmod8 (code, ip0, ip1);
				return;
		case 3:	func = (char*)"_divmod24Ptr";		break;
		case 4:	func = (char*)"_divmod32Ptr";		break;
		default: return;
	}

	if ( size0 < size1 ) {
		divmod((code == DIV_ASSIGN)? '/': '%', ip0, ip0, ip1);
		return;
	}

	int op_flag = (code != DIV_ASSIGN)? 4: 0;
	if ( sign0 ) op_flag |= 2;	// signed = 1; unsigned = 0
	if ( sign1 ) op_flag |= 1;	// signed = 1; unsigned = 0

	delete pushStack(ip1, size);
	ASM_CODE1(_PUSH, op_flag, '#'); frameOffset++;
	setPtr(ip0, REG_X);
	call(func);
	popStack(size+1);
}

void STM8S :: divmod8(int code, Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, 1);
	char *r16 = reg1? REG_X: REG_X;
	char *r8  = strcmp(r16, REG_X)? REG_YL:
									REG_XL;
	bool signed_op = (ip0->acceSign() || ip1->acceSign());

	if ( signed_op )
		ASM_CODE1(_PUSH, 0, '#'), frameOffset++;

	if ( size0 == 2 ) {
		ASM_CODE2(_LDW, r16, acceItem(ip0, 0, 2, reg0));
		if ( ip0->acceSign() ) {
			ASM_CODE1(_JRPL, strcmp(r16, REG_X)? ".+4": ".+3");
			ASM_CODE1(_NEGW, r16);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
	}
	else {
		ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
		if ( ip0->acceSign() ) {
			ASM_CODE1(_JRPL, ".+3");
			ASM_CODE1(_NEG, REG_A);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
		ASM_CODE1(_CLRW, r16);
		ASM_CODE2(_LD, r8, REG_A);
	}

	ASM_CODE2(_LD, REG_A, acceItem(ip1, 0, 1, reg1));
	if ( ip1->acceSign() ) {
		ASM_CODE1(_JRPL, ".+3");
		ASM_CODE1(_NEG, REG_A);
		ASM_CODE1(_CPL, "(1, %SP)");
	}
	ASM_CODE2(_DIV, r16, REG_A);
	if ( code == DIV_ASSIGN ) {
		if ( signed_op ) {
			ASM_CODE1(_TNZ, "(1, %SP)");
			ASM_CODE1(_JREQ, strcmp(r16, REG_X)? ".+2": ".+1");
			ASM_CODE1(_NEGW, r16);
		}
		if ( size0 == 1 )
			ASM_CODE2(_LD, REG_A, r8);
	}
	else {
		if ( signed_op ) {
			ASM_CODE1(_TNZ, "(1, %SP)");
			ASM_CODE1(_JREQ, ".+1");
			ASM_CODE1(_NEG, REG_A);
		}
	}
	if ( reg0 && strcmp(reg0, r16) == 0 )
		reg0 = setPtr(ip0, 1);

	if ( code == DIV_ASSIGN && size0 == 2 )
		ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), r16);
	else {
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		if ( size0 == 2 ) {
			if ( signed_op ) {
				call((char*)"_extendRegA");
				ASM_CODE2(_LD, acceItem(ip0, 1, 1, reg0), REG_A);
			}
			else
				ASM_CODE1(_CLR, acceItem(ip0, 1, 1, reg0));
		}
	}
	if ( signed_op ) popStack(1);
}

void STM8S :: divmod16(int code, Item *ip0, Item *ip1)
{
	int  size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, 1);
	char *r16 = REG_X;
	char *r8  = REG_XL;
	bool signed_op = (ip0->acceSign() || ip1->acceSign());

	if ( signed_op )
		ASM_CODE1(_PUSH, 0, '#'), frameOffset++;

	if ( size0 == 2 ) {
		ASM_CODE2(_LDW, r16, acceItem(ip0, 0, 2, reg0));
		if ( ip0->acceSign() ) {
			ASM_CODE1(_JRPL, strcmp(r16, REG_X)? ".+4": ".+3");
			ASM_CODE1(_NEGW, r16);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
	}
	else {
		ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
		if ( ip0->acceSign() ) {
			ASM_CODE1(_JRPL, ".+3");
			ASM_CODE1(_NEG, REG_A);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
		ASM_CODE1(_CLRW, r16);
		ASM_CODE2(_LD, r8, REG_A);
	}
	if ( LOCAL_ADDR(ip1) )
		loadLocalAddr(ip1, REG_Y);
	else if ( farAcce(ip1) ) {
		delete pushStack(ip1, 2);
		ASM_CODE1(_POPW, REG_Y); frameOffset -= 2;
	}
	else
		ASM_CODE2(_LDW, REG_Y, acceItem(ip1, 0, 2, reg1));
	if ( ip1->acceSign() ) {
		ASM_CODE1(_JRPL, ".+4");
		ASM_CODE1(_NEGW, REG_Y);
		ASM_CODE1(_CPL, "(1, %SP)");
	}

	ASM_CODE2(_DIVW, REG_X, REG_Y);

	if ( signed_op ) {
		ASM_CODE1(_POP, REG_A); frameOffset--;
		ASM_CODE1(_TNZ, REG_A);
		if ( code == DIV_ASSIGN ) {
			ASM_CODE1(_JREQ, ".+1");
			ASM_CODE1(_NEGW, REG_X);
		}
		else {
			ASM_CODE1(_JREQ, ".+2");
			ASM_CODE1(_NEGW, REG_Y);
		}
	}
	reg0 = setPtr(ip0, (code == DIV_ASSIGN)? 1: 0);
	if ( size0 == 1 ) {
		if ( code == DIV_ASSIGN )
			ASM_CODE2(_LD, REG_A, REG_XL);
		else
			ASM_CODE2(_LD, REG_A, REG_YL);
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
	}
	else {
		if ( code == DIV_ASSIGN )
			ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), REG_X);
		else
			ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), REG_Y);
	}
}

