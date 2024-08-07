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
				call((char*)"_signedCmp");
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSS, aWREG, 0);
				ASM_CODE(_GOTO, ip2->val.s);
				break;
			case P_JLT:
				ASM_CODE(_MOVIW, "INDF1++");	// WREG[7] <- Sx1 ^ Sx2
				ASM_CODE(_BTFSC, aSTATUS, 0);
				ASM_CODE(_XORLW, 0x80);			// WREG[7] ^= 1 if Carry = 1
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSS, aWREG, 7);
				ASM_CODE(_GOTO, ip2->val.s);	// jump if (Sx1^Sx2^Sign) = 0
				break;
			case P_JGE:
				ASM_CODE(_MOVIW, "INDF1++");	// WREG[7] <- Sx1 ^ Sx2
				ASM_CODE(_BTFSC, aSTATUS, 0);
				ASM_CODE(_XORLW, 0x80);			// WREG[7] ^= 1 if Carry = 1
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSC, aWREG, 7);
				ASM_CODE(_GOTO, ip2->val.s);	// jump if (Sx1^Sx2^Sign) = 1
				break;
			case P_JGT:
				call((char*)"_signedCmp");
				regPCLATH->load(ip2->val.s);
				ASM_CODE(_BTFSC, aWREG, 0);
				ASM_CODE(_GOTO, ip2->val.s);
				break;
		}
		regWREG->reset();
	}
}