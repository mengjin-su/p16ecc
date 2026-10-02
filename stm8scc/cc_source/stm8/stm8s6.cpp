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

void STM8S :: add(Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int size2 = ip2->acceSize();

	if ( (farAcce(ip2) && !farAcce(ip1)) || (LOCAL_ADDR(ip2) && !LOCAL_ADDR(ip1)) ) {
		add(ip0, ip2, ip1);
		return;
	}
	if ( (size2 < size0 && ip2->acceSign()) || farAcce(ip2) || LOCAL_ADDR(ip2) ) {
		mov(ip0, related(ip0, ip1)? ip1: ip2);
		add(ip0, related(ip0, ip1)? ip2: ip1);
		return;
	}
	if ( LOCAL_ADDR(ip1) || (usePtr(ip0, ip1, ip2) && size0 > 1) ) {
		mov(ip0, related(ip0, ip2)? ip2: ip1);
		add(ip0, related(ip0, ip2)? ip1: ip2);
		return;
	}
	if ( farAcce(ip1) || usePtr(ip1) || usePtr(ip2) ) {
		if ( (size0 <= size1 || !ip1->acceSign()) && (size0 <= size2 || !ip2->acceSign()) )
			add8(ip0, ip1, ip2);
		else {
			mov(ip0, related(ip0, ip1)? ip1: ip2);
			add(ip0, related(ip0, ip1)? ip2: ip1);
		}
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *r16  = reg0? REG_Y: REG_X;
	bool const1= ip1->type == CON_ITEM || (ip1->type == IMMD_ITEM && !LOCAL_ADDR(ip1));
	bool const2= ip2->type == CON_ITEM || (ip2->type == IMMD_ITEM && !LOCAL_ADDR(ip2));
	for (int i = 0; i < size0; i++) {
		if ( (i+1) < size0 && ((i+1) < size1 || const1) && ((i+1) < size2 || const2) )
		{
			ASM_CODE2(_LDW, r16, acceItem(ip1, i, 2));
			if ( i > 0 ) {
				ASM_CODE1(_JRNC, (strcmp(r16, REG_Y) == 0)? ".+2": ".+1");
				ASM_CODE1(_INCW, r16);
			}
			ASM_CODE2(_ADDW, r16, acceItem(ip2, i, 2));
			ASM_CODE2(_LDW, acceItem(ip0, i++, 2, reg0), r16);
		}
		else
		{
			char *inst = i? _ADC: _ADD;
			if ( i < size1 )
				ASM_CODE2(_LD,  REG_A, acceItem(ip1, i, 1));
			else
				ASM_CODE1(_CLR,  REG_A);

			if ( i < size2 )
				ASM_CODE2(inst, REG_A, acceItem(ip2, i, 1));
			else
				ASM_CODE2(inst, REG_A, 0, '#');
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: add8(Item *ip0, Item *ip1, Item *ip2)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	int  size2 = ip2->acceSize();
	char *reg0 = NULL, *reg1 = NULL, *reg2 = NULL;
	if ( size0 > 1 ) {
		reg0 = setPtr(ip0, 0);
		if ( size1 > 1 ) reg1 = setPtr(ip1, reg0? 1: 0);
		if ( size2 > 1 ) reg2 = setPtr(ip2, (reg0||reg1)? 1: 0);
	}

	for (int i = 0; i < size0; i++) {
		char *inst = i? _ADC: _ADD;

		if ( reg1 == NULL ) reg1 = setPtr(ip1, 0);
		if ( i < size1 ) {
			char *load = farAcce(ip1)? _LDF: _LD;
			ASM_CODE2(load, REG_A, acceItem(ip1, i, 1, reg1));
		}
		else
			ASM_CODE1(_CLR, REG_A);

		if ( reg2 == NULL )	reg2 = setPtr(ip2, 0);
		if ( i < size2 )
			ASM_CODE2(inst, REG_A, acceItem(ip2, i, 1, reg2));
		else
			ASM_CODE2(inst, REG_A, 0, '#');

		if ( reg0 == NULL ) reg0 = setPtr(ip0, 0);
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}

void STM8S :: sub(Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int size2 = ip2->acceSize();

	if ( LOCAL_ADDR(ip1) || (usePtr(ip0, ip1, ip2) && size0 > 1) ) {
		mov(ip0, ip1);
		sub(ip0, ip2);
		return;
	}
	if ( LOCAL_ADDR(ip2) || farAcce(ip2) ) {
		Item *ip = pushStack(ip2, size0);
		sub(ip0, ip1, ip);
		popStack(size0);
		delete ip;
		return;
	}
	if ( farAcce(ip1) || usePtr(ip1) || usePtr(ip2) ) {
		if( (size0 <= size1 || !ip1->acceSign()) && (size0 <= size2 || !ip2->acceSign()) )
			sub8(ip0, ip1, ip2);
		else {
			mov(ip0, ip1);
			sub(ip0, ip2);
		}
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *r16  = reg0? REG_Y: REG_X;
	bool const1= ip1->type == CON_ITEM || (ip1->type == IMMD_ITEM && !LOCAL_ADDR(ip1));
	bool const2= ip2->type == CON_ITEM || (ip2->type == IMMD_ITEM && !LOCAL_ADDR(ip2));
	for (int i = 0; i < size0; i++) {
		if ( (i+1) < size0 &&
			 ((i+1) < size1 || const1) && ((i+1) < size2 || const2) )
		{
			ASM_CODE2(_LDW, r16, acceItem(ip1, i, 2));
			if ( i > 0 ) {
				ASM_CODE1(_JRNC, reg0? ".+2": ".+1");
				ASM_CODE1(_DECW, r16);
			}
			ASM_CODE2(_SUBW, r16, acceItem(ip2, i, 2));
			ASM_CODE2(_LDW, acceItem(ip0, i++, 2, reg0), r16);
		}
		else
		{
			char *inst = i? _SBC: _SUB;
			if ( i < size1 )
				ASM_CODE2(_LD,  REG_A, acceItem(ip1, i, 1));
			else
				ASM_CODE1(_CLR,  REG_A);

			if ( i < size2 )
				ASM_CODE2(inst, REG_A, acceItem(ip2, i, 1));
			else
				ASM_CODE2(inst, REG_A, 0, '#');
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
	}
}

void STM8S :: sub8(Item *ip0, Item *ip1, Item *ip2)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	int  size2 = ip2->acceSize();
	char *reg0 = NULL, *reg1 = NULL, *reg2 = NULL;
	if ( size0 > 1 ) {
		reg0 = setPtr(ip0, 0);
		reg1 = setPtr(ip1, reg0? 1: 0);
		reg2 = setPtr(ip2, (reg0||reg1)? 1: 0);
	}

	for (int i = 0; i < size0; i++) {
		char *inst = i? _SBC: _SUB;

		if ( reg1 == NULL ) reg1 = setPtr(ip1, 0);
		if ( i < size1 ) {
			char *load = farAcce(ip1)? _LDF: _LD;
			ASM_CODE2(load, REG_A, acceItem(ip1, i, 1, reg1));
		}
		else
			ASM_CODE1(_CLR, REG_A);

		if ( reg2 == NULL )	reg2 = setPtr(ip2, 0);
		if ( i < size2 )
			ASM_CODE2(inst, REG_A, acceItem(ip2, i, 1, reg2));
		else
			ASM_CODE2(inst, REG_A, 0, '#');

		if ( reg0 == NULL ) reg0 = setPtr(ip0, 0);
		ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}
}