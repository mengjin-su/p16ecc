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

void STM8S :: leftAssign(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int  lbl1 = pcoder->getLbl();
	int  lbl2 = pcoder->getLbl();

	if ( isDirectReg(ip0) && !(ip1->type == CON_ITEM && ip1->val.i == 1) )
	{
		Item *acc = accItem(size0);
		mov(acc, ip0);
		leftAssign(acc, ip1);
		mov(ip0, acc);
		delete acc;
		return;
	}
	if ( ip1->type == CON_ITEM ) {
		leftAssign(ip0, ip1->val.i);
		return;
	}
	
	delete pushStack(ip1, 1);
	ASM_CODE_LBL(_JREQ, lbl2);

	char *reg0 = setPtr(ip0, 0);
	ASM_LABL(lbl1);
	for(int i = 0; i < size0; i++) {
		char *inst = (i == 0)? _SLL: _RLC;
		ASM_CODE1(inst, acceItem(ip0, i, 1, reg0));
	}
	ASM_CODE1(_DEC, "(1, %SP)");
	ASM_CODE_LBL(_JRNE, lbl1);
	ASM_LABL(lbl2);
	popStack(1);
}

void STM8S :: leftAssign(Item *ip0, int n)
{
	int size0 = ip0->acceSize();
	if ( n == 0 ) return;
	if ( n >= size0*8 ) n = size0*8;
	int byte_shift = n / 8;
	int bit_shift  = n % 8;
	char *reg0 = setPtr(ip0, 0);

	if ( ip0->attr->isVolatile && size0 == 1 && n > 1 ) {
		ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
		while ( n-- ) ASM_CODE1(_SLL, REG_A);
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		return;
	}
	if ( byte_shift > 0 ) {
		for (int i = size0; i--;) {
			if ( i >= byte_shift ) {
				ASM_CODE2(_LD, REG_A, acceItem(ip0, i-byte_shift, 1, reg0));
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			}
			else
				ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
		}
	}
	if ( bit_shift > 0 ) {
		int  lbl = pcoder->getLbl();
		if (  bit_shift > 1 ) {
			ASM_CODE2(_LD, REG_A, bit_shift, '#');
			ASM_LABL(lbl);
		}
		for (int i = byte_shift; i < size0; i++) {
			char *inst = (i==byte_shift)? _SLL: _RLC;
			ASM_CODE1(inst, acceItem(ip0, i, 1, reg0));
		}
		if ( bit_shift > 1 ) {
			ASM_CODE1(_DEC, REG_A);
			ASM_CODE_LBL(_JRNE, lbl);
		}
	}
}

void STM8S :: rightAssign(Item *ip0, Item *ip1, bool sign0)
{
	int  size0 = ip0->acceSize();
	int  lbl1  = pcoder->getLbl();
	int  lbl2  = pcoder->getLbl();

	if ( isDirectReg(ip0) && !(ip1->type == CON_ITEM && ip1->val.i == 1) )
	{
		Item *acc = accItem(size0);
		mov(acc, ip0);
		rightAssign(acc, ip1, sign0);
		mov(ip0, acc);
		delete acc;
		return;
	}
	if ( ip1->type == CON_ITEM ) {
		rightAssign(ip0, ip1->val.i, sign0);
		return;
	}

	delete pushStack(ip1, 1);
	ASM_CODE_LBL(_JREQ, lbl2);

	char *reg0 = setPtr(ip0, 0);
	st8asm->label(lbl1);
	for(int i = size0; i; i--) {
		char *inst = (i == size0)? (sign0? _SRA: _SRL): _RRC;
		ASM_CODE1(inst, acceItem(ip0, i-1, 1, reg0));
	}
	ASM_CODE1(_DEC, "(1, %SP)");
	ASM_CODE_LBL(_JRNE, lbl1);
	ASM_LABL(lbl2);
	popStack(1);
}

void STM8S :: rightAssign(Item *ip0, int n, bool sign0)
{
	int size0 = ip0->acceSize();
	if ( n == 0 ) return;
	if ( n > size0*8 ) n = size0*8;
	int byte_shift = n / 8;
	int bit_shift  = n % 8;
	char *reg0 = setPtr(ip0, 0);

	if ( ip0->attr->isVolatile && size0 == 1 && n > 1 ) {
		ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
		while ( n-- ) ASM_CODE1(sign0? _SRA: _SRL, REG_A);
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		return;
	}
	if ( byte_shift > 0 ) {
		for (int i = byte_shift; i < size0; i++) {
			ASM_CODE2(_LD, REG_A, acceItem(ip0, i, 1, reg0));
			ASM_CODE2(_LD, acceItem(ip0, i-byte_shift, 1, reg0), REG_A);
		}
		if ( sign0 ) call((char*)"_extendRegA");
		for (int i = size0 - byte_shift; i < size0; i++)
			if ( sign0 )
				ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
			else
				ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
	}
	if ( bit_shift > 0 ) {
		int  lbl = pcoder->getLbl();
		int m = bit_shift * (size0 - byte_shift);
		if ( m > 4 ) {
			ASM_CODE2(_LD, REG_A, bit_shift, '#');
			ASM_LABL(lbl);
		}
		for(int I = 0; I < ((m <= 4)? bit_shift: 1); I++)
		for (int i = size0 - byte_shift; i; i--) {
			char *inst = (i == (size0 - byte_shift))? (sign0? _SRA: _SRL): _RRC;
			ASM_CODE1(inst, acceItem(ip0, i-1, 1, reg0));
		}
		if ( m > 4 ) {
			ASM_CODE1(_DEC, REG_A);
			ASM_CODE_LBL(_JRNE, lbl);
		}
	}
}

void STM8S :: leftOpr(Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();

	if ( isDirectReg(ip0) )
	{
		if ( (ip1->type == TEMP_ITEM || ip1->type == ACC_ITEM) && size1 >= size0 )
		{
			leftAssign(ip1, ip2);
			mov(ip0, ip1);
		}
		else
		{		
			Item *acc = accItem(size0);
			leftOpr(acc, ip1, ip2);
			mov(ip0, acc);
			delete acc;
		}
		return;
	}
	if ( same(ip0, ip1) )
	{
		leftAssign(ip0, ip2);
		return;
	}
	if ( size1 >= size0 &&
		 (ip1->type == ACC_ITEM || ip1->type == TEMP_ITEM) ) {
		leftAssign(ip1, ip2);
		mov(ip0, ip1);
		return;
	}
	if ( related(ip0, ip2) || related(ip1, ip2) || farAcce(ip1) ||
		 ip0->attr->isVolatile || (size1 != size0 && related(ip0, ip1)) )
	{
		Item *ip = pushStack(ip1, size0);
		leftAssign(ip, ip2);
		mov(ip0, ip);
		popStack(size0);
		delete ip;
		return;
	}
	if ( ip2->type == CON_ITEM ) {
		leftOpr(ip0, ip1, ip2->val.i);
		return;
	}

	mov(ip0, ip1);
	leftAssign(ip0, ip2);
}

void STM8S :: leftOpr(Item *ip0, Item *ip1, int n)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();
	if ( n > size0*8 ) n = size0*8;
	int  byte_shift = n/8;
	int  bit_shift  = n%8;
	char *ld1 = farAcce(ip1)? _LDF: _LD;

	if ( LOCAL_ADDR(ip1) ) {
		mov(ip0, ip1);
		leftAssign(ip0, n);
		return;
	}
	if ( (ip1->type == TEMP_ITEM || ip1->type == ACC_ITEM || same(ip0, ip1)) &&
		 related(ip0, ip1) 			)
	{
		leftAssign(ip1, n);
		if ( !same(ip0, ip1) ) mov(ip0, ip1);
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	for (int i = 0; i < size0; i++) {
		if ( i >= byte_shift && (i-byte_shift) < size1 ) {
			ASM_CODE2(ld1, REG_A, acceItem(ip1, i-byte_shift, 1, reg1));
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
		else if ( i >= byte_shift && sign1 ) {
			if ( (i-byte_shift) == size1 )
				call((char*)"_extendRegA");
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
		}
		else
			ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg0));
	}
	if ( bit_shift > 0 ) {
		int  lbl = pcoder->getLbl();
		if ( bit_shift > 1 ) {
			ASM_CODE2(_LD, REG_A, bit_shift, '#');
			ASM_LABL(lbl);
		}
		for (int i = byte_shift; i < size0; i++) {
			char *inst = (i==byte_shift)? _SLL: _RLC;
			ASM_CODE1(inst, acceItem(ip0, i, 1, reg0));
		}
		if ( bit_shift > 1 ) {
			ASM_CODE1(_DEC, REG_A);
			ASM_CODE_LBL(_JRNE, lbl);
		}
	}
}

void STM8S :: rightOpr(Item *ip0, Item *ip1, Item *ip2)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();
	
	if ( isDirectReg(ip0) )
	{
		if ( (ip1->type == TEMP_ITEM || ip1->type == ACC_ITEM) && size1 >= size0 )
		{
			rightAssign(ip1, ip2, ip1->acceSign());
			mov(ip0, ip1);
		}
		else
		{
			Item *acc = accItem(size0);
			rightOpr(acc, ip1, ip2);
			mov(ip0, acc);
			delete acc;
		}
		return;
	}
	if ( same(ip0, ip1) )
	{
		rightAssign(ip0, ip2, ip1->acceSign());
		return;
	}
	if ( ip2->type == CON_ITEM ) {
		rightOpr(ip0, ip1, ip2->val.i);
		return;
	}
	if ( ip1->type == ACC_ITEM || ip1->type == TEMP_ITEM ) {
		rightAssign(ip1, ip2, sign1);
		mov(ip0, ip1);
		return;
	}

	int   lbl1 = pcoder->getLbl();
	int   lbl2 = pcoder->getLbl();
	Item *acc  = NULL;
	attrib *attr0 = ip0->attr;
	delete pushStack(ip2, 1);			// push ip2 into stack (1 byte)

	if ( size0 < size1 || attr0->isVolatile )
		acc = storeToACC(ip1, size1);	// bigger than ip0, acc <- ip1
	else
		mov(ip0, ip1);					// ip0 <- ip1

	popStack(1);
	ASM_CODE1(_TNZ, REG_A);
	ASM_CODE_LBL(_JREQ, lbl2);
	ASM_LABL(lbl1);
	int size = acc? size1: size0;		// shift operand size
	char *reg0 = usePtr(ip0)? REG_X: NULL;
	for (int i = size; i--;) {
		char *inst = sign1? _SRA: _SRL;
		if ( i != (size-1) ) inst = _RRC;

		if ( acc )
			ASM_CODE1(inst, acceItem(acc, i, 1));
		else
			ASM_CODE1(inst, acceItem(ip0, i, 1, reg0));
	}
	ASM_CODE1(_DEC, REG_A);
	ASM_CODE_LBL(_JRNE, lbl1);
	ASM_LABL(lbl2);
	if ( acc ) { mov(ip0, acc); delete acc; }
}

void STM8S :: rightOpr(Item *ip0, Item *ip1, int n)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	bool sign1 = ip1->acceSign();
	attrib *attr0 = ip0->attr;

	n = (n > size1*8)? size1*8: n;
	int  byte_shift = n/8;
	int  bit_shift  = n%8;

	if ( attr0->isVolatile && bit_shift ) {
		Item *acc = storeToACC(ip1, size1);
		rightAssign(acc, n, sign1);
		mov(ip0, acc);
		delete acc;
		return;
	}
	if ( LOCAL_ADDR(ip1) ) {//|| size0 >= size1 ) {
		mov(ip0, ip1);
		rightAssign(ip0, n, sign1);
		return;
	}
	if ( (ip1->type == TEMP_ITEM || ip1->type == ACC_ITEM) &&
		  related(ip0, ip1) && (size1-byte_shift) > size0	)
	{
		rightAssign(ip1, n, ip1->acceSign());
		mov(ip0, ip1);
		return;
	}

	char *reg0 = setPtr(ip0, 0);
	char *reg1 = setPtr(ip1, reg0? 1: 0);
	int  limit = size0 + (bit_shift + 7)/8;
	bool full_copy = (size0+byte_shift) == size1;
	char *ld1  = farAcce(ip1)? _LDF: _LD;
	for (int i = 0; i < limit; i++)
	{
		if ( (i + byte_shift) < size1 )
			ASM_CODE2(ld1, REG_A, acceItem(ip1, i+byte_shift, 1, reg1));
		else if ( (i + byte_shift) == size1 && !full_copy )
		{
			if ( sign1 )
				call((char*)"_extendRegA");
			else
				ASM_CODE1(_CLR, REG_A);
		}

		if ( i < size0 )
			ASM_CODE2(_LD, acceItem(ip0, i, 1, reg0), REG_A);
	}

	if ( bit_shift ) {
		int  lbl = pcoder->getLbl();
		char *r16 = reg0? REG_Y: REG_X;
		full_copy = (size0+byte_shift) >= size1;
		if ( full_copy ) limit = size1 - byte_shift;
		if ( bit_shift > 1 )
		{
			if ( full_copy ) ASM_CODE2(_LD, REG_A, bit_shift, '#');
			else			 ASM_CODE2(_LDW, r16, bit_shift, '#');
			ASM_LABL(lbl);
		}
		for (int i = limit; i--;) {
			if ( (i + 1) == limit ) {	// highest byte ?
				if ( full_copy ) {
					char *inst = sign1? _SRA: _SRL;
					ASM_CODE1(inst, acceItem(ip0, i, 1, reg0));
				}
				else
					ASM_CODE1(_RRC, REG_A);
			}
			else
				ASM_CODE1(_RRC, acceItem(ip0, i, 1, reg0));
		}
		if ( bit_shift > 1 ) {
			if ( full_copy ) ASM_CODE1(_DEC, REG_A);
			else			 ASM_CODE1(_DECW, r16);
			ASM_CODE_LBL(_JRNE, lbl);
		}
	}
}