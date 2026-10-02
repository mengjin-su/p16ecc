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

void STM8S :: divmod(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size1 = ip1->acceSize();
	int size2 = ip2->acceSize();
	int size  = (size1 >= size2)? size1: size2;
	char *func;
	switch ( size )
	{
		case 1:
		case 2:	if ( size2 == 1 ) divmod8(code, ip0, ip1, ip2);
				else			  divmod16(code, ip0, ip1, ip2);
			    return;
		case 3:	func = (char*)"_divmod24";	break;
		case 4:	func = (char*)"_divmod32";	break;
		default: return;
	}
	delete pushStack(ip1, size);	// push X
	delete pushStack(ip2, size);	// push Y
	int n = (code == '/')? 0: 4;
	if ( ip1->acceSign() ) n |= 1;
	if ( ip2->acceSign() ) n |= 2;
	ASM_CODE1(_PUSH, n, '#'), frameOffset++;
	call(func);

	int  size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	char *r16  = reg0? REG_Y: REG_X;
	bool signed_op = ip1->acceSign() || ip2->acceSign();

	for (int i = 0; i < size0; i++) {
		char *s = STRBUF();
		if ( (size0 - i) >= 2 && (size - i) >= 2 ) {
			sprintf(s, "(%d, %s)", size*2-i, REG_SP);
			ASM_CODE2(_LDW, r16, s);
			ASM_CODE2(_LDW, acceItem(ip0, i++, 2, reg0), r16);
		}
		else if ( size > i ) {
			sprintf(s, "(%d, %s)", size*2-i+1, REG_SP);
			ASM_CODE2(_LD, REG_A, s);
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
		else if ( signed_op ) {
			if ( i == size ) call((char*)"_extendSign");
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
		else
			ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
	}
	popStack(size*2+1);
}

void STM8S :: divmod8(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int  size1 = ip1->acceSize();
	char *reg1 = setPtr(ip1, 0);
	char *reg2 = setPtr(ip2, 1);
	char *r16 = reg2? REG_X: REG_X;
	char *r8  = strcmp(r16, REG_X)? REG_YL: REG_XL;
	bool signed_op = (ip1->acceSign() || ip2->acceSign());

	if ( signed_op )
		ASM_CODE1(_PUSH, 0, '#'), frameOffset++;

	if ( size1 == 2 ) {
		if ( LOCAL_ADDR(ip1) )
			loadLocalAddr(ip1, r16);
		else if ( farAcce(ip1) ) {
			delete pushStack(ip1, 2);
			ASM_CODE1(_POPW, r16); frameOffset -= 2;
		}
		else
			ASM_CODE2(_LDW, r16, acceItem(ip1, 0, 2, reg1));

		if ( ip1->acceSign() ) {
			ASM_CODE1(_JRPL, strcmp(r16, REG_X)? ".+4": ".+3");
			ASM_CODE1(_NEGW, r16);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
	}
	else {
		char *ld = farAcce(ip1)? _LDF: _LD;
		ASM_CODE2(ld, REG_A, acceItem(ip1, 0, 1, reg1));
		if ( ip1->acceSign() ) {
			ASM_CODE1(_JRPL, ".+3");
			ASM_CODE1(_NEG, REG_A);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
		ASM_CODE1(_CLRW, r16);
		ASM_CODE2(_LD, r8, REG_A);
	}
	char *ld = farAcce(ip2)? _LDF: _LD;
	ASM_CODE2(ld, REG_A, acceItem(ip2, 0, 1, reg2));
	if ( ip2->acceSign() ) {
		ASM_CODE1(_JRPL, ".+3");
		ASM_CODE1(_NEG, REG_A);
		ASM_CODE1(_CPL, "(1, %SP)");
	}

	ASM_CODE2(_DIV, r16, REG_A);

	if ( signed_op ) {
		ASM_CODE1(_TNZ, "(1, %SP)");
		if ( code == '/' ) {
			ASM_CODE1(_JREQ, strcmp(r16, REG_X)? ".+2": ".+1");
			ASM_CODE1(_NEGW, r16);
		}
		else {
			ASM_CODE1(_JREQ, ".+1");
			ASM_CODE1(_NEG, REG_A);
		}
	}

	int size0 = ip0->acceSize();
	if ( code == '/' && size0 >= 2 ) {
		char *reg0 = setPtr(ip0, 1);
		ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), r16);
		for (int i = 2; i < size0; i++) {
			if ( signed_op ) {
				if ( i == 2 ) call((char*)"_extendSign");
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			}
			else
				ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
		}
	}
	else {
		if ( code == '/' )	ASM_CODE2(_LD, REG_A, r8);
		if ( ip0->attr->dataBank == CONST && EXT_MODE ) ASM_CODE1(_PUSH, REG_A), frameOffset++;
		char *reg0 = setPtr(ip0, 0);
		if ( ip0->attr->dataBank == CONST && EXT_MODE ) ASM_CODE1(_POP,  REG_A), frameOffset--;
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		for (int i = 1; i < size0; i++) {
			if ( signed_op ) {
				if ( i == 1 ) call((char*)"_extendRegA");
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			}
			else
				ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
		}
	}
	if ( signed_op ) popStack(1);
}

void STM8S :: divmod16(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int  size1 = ip1->acceSize();
	char *reg1 = setPtr(ip1, 0);
	char *reg2 = setPtr(ip2, 1);
	char *r16 = REG_X;
	char *r8  = REG_XL;
	bool signed_op = (ip1->acceSign() || ip2->acceSign());

	if ( signed_op )
		ASM_CODE1(_PUSH, 0, '#'), frameOffset++;

	if ( size1 == 2 ) {
		if ( LOCAL_ADDR(ip1) )
			loadLocalAddr(ip1, r16);
		else if ( farAcce(ip1) ) {
			delete pushStack(ip1, 2);
			ASM_CODE1(_POPW, r16); frameOffset -= 2;
		}
		else
			ASM_CODE2(_LDW, r16, acceItem(ip1, 0, 2, reg1));

		if ( ip1->acceSign() ) {
			ASM_CODE1(_JRPL, strcmp(r16, REG_X)? ".+4": ".+3");
			ASM_CODE1(_NEGW, r16);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
	}
	else {
		char *ld = farAcce(ip1)? _LDF: _LD;
		ASM_CODE2(ld, REG_A, acceItem(ip1, 0, 1, reg1));
		if ( ip1->acceSign() ) {
			ASM_CODE1(_JRPL, ".+3");
			ASM_CODE1(_NEG, REG_A);
			ASM_CODE1(_CPL, "(1, %SP)");
		}
		ASM_CODE1(_CLRW, r16);
		ASM_CODE2(_LD, r8, REG_A);
	}

	if ( LOCAL_ADDR(ip2) )
		loadLocalAddr(ip2, REG_Y);
	else if ( farAcce(ip2) ) {
		delete pushStack(ip2, 2);
		ASM_CODE1(_POPW, REG_Y), frameOffset -= 2;
	}
	else
		ASM_CODE2(_LDW, REG_Y, acceItem(ip2, 0, 2, reg2));

	if ( ip2->acceSign() ) {
		ASM_CODE1(_JRPL, ".+4");
		ASM_CODE1(_NEGW, REG_Y);
		ASM_CODE1(_CPL, "(1, %SP)");
	}

	ASM_CODE2(_DIVW, REG_X, REG_Y);

	if ( signed_op ) {
		ASM_CODE1(_POP, REG_A), frameOffset--;
		ASM_CODE1(_TNZ, REG_A);
		if ( code == '/' ) {
			ASM_CODE1(_JREQ, ".+1");
			ASM_CODE1(_NEGW, REG_X);
		}
		else {
			ASM_CODE1(_JREQ, ".+2");
			ASM_CODE1(_NEGW, REG_Y);
		}
	}

	int size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, (code == '/')? 1: 0);
	r16 = (code == '/')? REG_X:  REG_Y;
	r8  = (code == '/')? REG_XL: REG_YL;
	if ( size0 == 1 ) {
		ASM_CODE2(_LD, REG_A, r8);
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
	}
	else {
		ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), r16);
		for (int i = 2; i < size0; i++) {
			if ( signed_op ) {
				if ( i == 2 ) call((char*)"_extendSign");
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			}
			else
				ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
		}
	}
}