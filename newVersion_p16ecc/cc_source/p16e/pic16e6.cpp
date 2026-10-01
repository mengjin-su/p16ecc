#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "../common.h"
extern "C" {
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "pic16e.h"
#include "pic16e_asm.h"
#include "pic16e_reg.h"
#include "pic16e_inst.h"

void PIC16E :: jeqjne(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int sign1 = ip1->acceSign();
	int size  = (size0 > size1)? size0: size1;
	char *inst= (code == P_JEQ)? _BTFSC: _BTFSS;

	if ( ip0->isBF() )
	{
		if ( ip1->type != CON_ITEM )
			errPrint("SORRY - not support this compare!");
		else
		{
			size0     = BF_SIZE(ip0->attr);
			int shift = BF_OFFSET(ip0->attr);
			uint8_t mask = ((1 << size0) - 1) << shift;
			int n = (ip1->val.i << shift) & mask;

			if ( ip1->val.i >= (1 << size0) )	// out of range
			{
				if ( code == P_JNE )
				{
					regPCLATH->load(ip2->val.s);
					ASM_CODE(_GOTO, ip2->val.s);
				}
				return;
			}

			if ( size0 == 1 )					// 1-bit compare
			{
				if ( code == P_JEQ )
					jzjnz(n? P_JNZ: P_JZ, ip0, ip2);
				else
					jzjnz(n? P_JZ: P_JNZ, ip0, ip2);
			}
			else
			{
				getBF(ip0, shift, size0);
				ASM_CODE(_XORLW, n);
				regWREG->reset();
				
				regPCLATH->load(ip2->val.s);
				ASM_CODE(inst, STATUS, 2);
				ASM_CODE(_GOTO, ip2->val.s);
			}
		}
		return;
	}
	if ( ip1->isBF() )
	{
		jeqjne(code, ip1, ip0, ip2);
		return;
	}
	if ( useFSR(ip0, ip1) )
	{
		Item *acc = storeToACC(ip1, size1);
		jeqjne(code, ip0, acc, ip2);
		delete acc;
		return;
	}
	if ( CONST_ITEM(ip1) )
	{
		jeqjneImmd(code, ip0, ip1, ip2);
		return;
	}
	if ( CONST_ITEM(ip0) )
	{
		jeqjneImmd(code, ip1, ip0, ip2);
		return;
	}
	if ( size0 < size1 )
	{
		jeqjne(code, ip1, ip0, ip2);
		return;
	}
	if ( CONST_ID_ITEM(ip1) )
	{
		if ( CONST_ID_ITEM(ip0) )
		{
			Item *acc = storeToACC(ip1, size0);
			jeqjne(code, ip0, acc, ip2);
			delete acc;
		}
		else if ( size0 > size1 )
			jeqjneRomId(code, ip1, ip0, ip2);
		else
			jeqjne(code, ip1, ip0, ip2);
		return;
	}

	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	for(int i = 0; i < size0; i++)
	{
		fetch(ip0, i, indf0);
		if ( i < size1 )
		{
			ASM_CODE(_XORWF, acceItem(ip1, i, indf1), _W_);
			regWREG->reset();
		}
		else if ( sign1 )
		{
			ASM_CODE(_BTFSC, acceItem(ip1, size1-1, indf1), 7);
			ASM_CODE(_XORLW, 255);
			regWREG->reset();
		}

		if ( i == 0 && size > 1 )
			ASM_CODE(_MOVWF, ACC0);
		else if ( i > 0 )
			ASM_CODE(_IORWF, ACC0, _F_);
	}

	regPCLATH->load(ip2->val.s);
	ASM_CODE(inst, aSTATUS, 2);
	ASM_CODE(_GOTO, ip2->val.s);
}

void PIC16E :: jeqjneImmd(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int size  = (size0 >= size1)? size0: size1;
	char *inst= (code == P_JEQ)? _BTFSC: _BTFSS;
	if ( size1 > size0 )
	{
		if ( code == P_JNE )
		{
			regPCLATH->load(ip2->val.s);
			ASM_CODE(_GOTO, ip2->val.s);
		}
		return;
	}
	if ( CONST_ITEM(ip0) )
	{
		char *buf = STRBUF(); buf[0] = 0;
		for(int i = 0; i < size; i++)
		{
			char *s = STRBUF();
			sprintf(s, "((%s)^(%s))", acceItem(ip0, i), acceItem(ip1, i));
			if ( i ) strcat(buf, "|");
			strcat(buf, s);
		}
		ASM_CODE(_MOVLW, buf);
		ASM_CODE(_ANDLW, 0xff);
		regWREG->reset();
	}
	else
	{
		char *indf = setFSR0(ip0);
		char *result = (ip0->type == ACC_ITEM)? FSR0L: ACC0;
		int  value1 = ip1->val.i;
		for (int i = 0; i < size0; i++, value1 >>= 8)
		{
			int num = value1 & 0xff;
			fetch(ip0, i, indf);
			if ( num ) {
				ASM_CODE(_XORLW, num);
				regWREG->reset();
			}
			if ( size0 > 1 ) {
				if ( i == 0 )
					ASM_CODE(_MOVWF, result);
				else
					ASM_CODE(_IORWF, result, _F_);
			}
		}
		if ( ip0->type == ACC_ITEM ) regFSR0->reset();
	}
	regPCLATH->load(ip2->val.s);
	ASM_CODE(inst, aSTATUS, 2);
	ASM_CODE(_GOTO, ip2->val.s);
}

void PIC16E :: jeqjneRomId(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int sign0 = ip0->acceSign();
	int sign1 = ip1->acceSign();

	if ( CONST_ID_ITEM(ip1) )
	{
		Item *acc = storeToACC(ip1, size1);
		jeqjneRomId(code, ip0, acc, ip2);
		delete acc;
		return;
	}
	char *indf = setFSR0(ip1);
	int size = (size0 >= size1)? size0: size1;
	char *inst= (code == P_JEQ)? _BTFSC: _BTFSS;
	for(int i = 0; i < size; i++)
	{
		if ( i < size0 )
		{
			fetch(ip0, i, NULL);
			if ( size0 < size1 && (i+1) == size0 && sign0 )
				ASM_CODE(_MOVWI, "--INDF1");
		}
		else
		{
			if ( indf )
				fetch(ip1, i-size0+1, indf);
			else
				fetch(ip1, i, indf);
			if ( sign0 )
			{
				ASM_CODE(_BTFSC, INDF1, 7);
				ASM_CODE(_XORLW, 0xff);
			}
			ASM_CODE(_IORWF, ACC0, _F_);
			regWREG->reset();
			continue;
		}

		if ( i < size1 )
		{
			ASM_CODE(_XORWF, acceItem(ip1, i, indf), _W_);
			regWREG->reset();
		}
		else if ( sign1 )
		{
			ASM_CODE(_BTFSC, acceItem(ip1, size1-1, indf), 7);
			ASM_CODE(_XORLW, 0xff);
			regWREG->reset();
		}

		if ( size > 1 )
		{
			if ( i == 0 )
				ASM_CODE(_MOVWF, ACC0);
			else
				ASM_CODE(_IORWF, ACC0, _F_);
		}
	}
	if ( size0 < size1 && sign0 )
		ASM_CODE(_ADDFSR, INDF1, 1);

	regPCLATH->load(ip2->val.s);
	ASM_CODE(inst, aSTATUS, 2);
	ASM_CODE(_GOTO, ip2->val.s);
}

////////////////////////////////////////////////////////////////////////////////////////////
static int convertCompare(int code);
static bool signedConst(int size, Item *ip);

void PIC16E :: cmpJump(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int code2 = convertCompare(code);

	if ( CONST_ITEM(ip1) )
	{
		cmpJumpConst(code, ip0, ip1, ip2);
		return;
	}
	if ( CONST_ITEM(ip0) )
	{
		cmpJumpConst(code2, ip1, ip0, ip2);
		return;
	}
	if ( size0 < size1 )
	{
		cmpJump(code2, ip1, ip0, ip2);
		return;
	}
	if ( (CONST_ID_ITEM(ip0) && CONST_ID_ITEM(ip1)) || useFSR(ip0, ip1) )
	{
		Item *acc = storeToACC(ip0, size0);
		cmpJump(code, acc, ip1, ip2);
		delete acc;
		return;
	}
	if ( CONST_ID_ITEM(ip0) && size0 <= size1 )
	{
		cmpJump(code2, ip1, ip0, ip2);
		return;
	}
	if ( CONST_ID_ITEM(ip0) && ip1->type != ACC_ITEM )
	{
		Item *acc = storeToACC(ip0, size0);
		cmpJump(code, acc, ip1, ip2);
		delete acc;
		return;
	}

	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	bool sign0  = ip0->acceSign();
	bool sign1  = ip1->acceSign();
	bool signedCmp = sign0 && sign1;
	char *inst  = _SUBWF;

	if ( indf0 == NULL && CONST_ID_ITEM(ip0) )
		indf0 = setFSR(ip0, 0);

	for (int i = 0; i < size0; i++, inst = _SUBWFB)
	{
		if ( i < size1 )
			fetch(ip1, i, indf1);
		else if ( !sign1 )
			regWREG->load(0);
		else
		{
			regWREG->load(0);
			ASM_CODE(_BTFSC, acceItem(ip1, size1-1, indf1), 7);
			ASM_CODE(_MOVLW, 255);
		}
		// get sign bit of ip0 ^ ip1 ...
		if ( (i+1) == size0 && signedCmp )	// MSB of ip0
		{
			ASM_CODE(_XORWF, acceItem(ip0, i, indf0), _W_);
			ASM_CODE(_MOVWI, "--INDF1");
			ASM_CODE(_XORWF, acceItem(ip0, i, indf0), _W_);
		}

		ASM_CODE(inst, acceItem(ip0, i, indf0), _W_);
		regWREG->reset();

		if ( code != P_JLT && code != P_JGE && size0 > 1 )
		{
			if ( i ) ASM_CODE(_IORWF, ACC0, _F_);
			else	 ASM_CODE(_MOVWF, ACC0);
		}
	}
	cmpJump(signedCmp, code, ip2);
}

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

static bool signedConst(int size, Item *ip)
{
	if ( ip->acceSign() )
		return true;

	unsigned int n = ip->val.i;
	unsigned int m = (1 << (size*8 - 1)) - 1;
	return (n <= m);
}

void PIC16E :: cmpJumpConst(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	bool sign0 = ip0->acceSign();
	bool sign1 = ip1->acceSign();
	bool signedCmp = ip0->acceSign() && ip1->acceSign();

	if ( size0 < size1 && ip1->type == CON_ITEM )
	{	// eliminate unnecessary comparison
		bool must_jump = NULL;
		switch ( code )
		{
			case P_JLT:
			case P_JLE:
				must_jump = !sign1 || ip1->val.i > 0;
				break;

			case P_JGT:
			case P_JGE:
				must_jump =  sign1 && ip1->val.i < 0;
				break;
		}
		if ( must_jump )
		{
			regPCLATH->load(ip2->val.s);
			ASM_CODE(_GOTO, ip2->val.s);
		}
		return;
	}

	if ( ip1->type == CON_ITEM )
		signedCmp = sign0 && signedConst(size0, ip1);

	if ( CONST_ID_ITEM(ip0) )
	{
		if ( size0 > 1 || size1 > 1 )
		{
			Item *acc = storeToACC(ip0, size0);
			cmpJumpConst(code, acc, ip1, ip2);
			delete acc;
			return;
		}

		// 1-byte comparison: ip1 - ip0
		fetch(ip0, 0, NULL);
		if ( signedCmp )
		{
			ASM_CODE(_XORLW, acceItem(ip1, 0));
			regWREG->reset();
			ASM_CODE(_MOVWI, "--INDF1");
			fetch(ip0, 0, NULL);
		}
		ASM_CODE(_SUBLW, acceItem(ip1, 0));
		cmpJump(signedCmp, convertCompare(code), ip2);
		return;
	}
	char *indf = setFSR0(ip0);
	char *inst = _SUBWF;
	for (int i = 0; i < size0; i++)
	{
		fetch(ip1, i, NULL);
		if ( signedCmp && (i+1) == size0 )	// last byte of signed compare
		{
			ASM_CODE(_XORWF, acceItem(ip0, i, indf), _W_);
			regWREG->reset();
			ASM_CODE(_MOVWI, "--INDF1");
			ASM_CODE(_XORWF, acceItem(ip0, i, indf), _W_);
		}
		ASM_CODE(inst, acceItem(ip0, i, indf), _W_);
		if ( code != P_JLT && code != P_JGE )
		{
			if ( i )
				ASM_CODE(_IORWF, ACC0, _F_);
			else if ( size0 > 1 )
				ASM_CODE(_MOVWF, ACC0);
		}
		regWREG->reset();
		inst = _SUBWFB;
	}
	cmpJump(signedCmp, code, ip2);
}

void PIC16E :: cmpJump(bool signedCmp, int code, Item *ip2)
{
	if ( !signedCmp )
	{
		regPCLATH->load(ip2->val.s);
		switch ( code )
		{
			case P_JLE:	// jump on (x <= y) : Z = 1 || C = 0
				ASM_CODE(_BTFSC, aSTATUS, 2);
				ASM_CODE(_BCF, aSTATUS, 0);
			case P_JLT:	// jump on (x < y) : C = 0
				ASM_CODE(_BTFSS, aSTATUS, 0);
				ASM_CODE(_GOTO, ip2->val.s);
				break;

			case P_JGT:	// jump on (x > y) : Z = 0 && C = 1
				ASM_CODE(_BTFSC, aSTATUS, 2);
				ASM_CODE(_BCF, aSTATUS, 0);
			case P_JGE:	// jump on (x >= y) : C = 1
				ASM_CODE(_BTFSC, aSTATUS, 0);
				ASM_CODE(_GOTO, ip2->val.s);
				break;
		}
	}
	else
	{
		switch ( code )
		{
			case P_JLE:
				call((char*)"_signedCmpAndZ");	// WREG[0] = ~Z & (Sx ^ Sy ^ C)
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSS, aWREG, 0);
				ASM_CODE(_GOTO, ip2->val.s);	// jump if WREG[0] = 0
				break;
			case P_JGT:
				call((char*)"_signedCmpAndZ");	// WREG[0] = ~Z & (Sx ^ Sy ^ C)
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSC, aWREG, 0);
				ASM_CODE(_GOTO, ip2->val.s);	// jump if WREG[0] = 1
				break;				
			case P_JLT:
				call((char*)"_signedCmpOrZ");	// WREG[0] = Z | (Sx ^ Sy ^ C)
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSS, aWREG, 0);
				ASM_CODE(_GOTO, ip2->val.s);	// jump if WREG[0] = 0
				break;
			case P_JGE:
				call((char*)"_signedCmpOrZ");	// WREG[0] = Z | (Sx ^ Sy ^ C)
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSC, aWREG, 0);
				ASM_CODE(_GOTO, ip2->val.s);	// jump if WREG[0] = 1
				break;
		}
		regWREG->reset();
	}
}