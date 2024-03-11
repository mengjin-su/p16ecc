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

	if ( (ip0->attr&& ip0->attr->type == SBIT) || (ip1->attr && ip1->attr->type == SBIT) )
	{
		jeqjneBits(code, ip0, ip1, ip2);
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

		if ( indf1 && (i+1) < size1 )
			regFSR0->inc(1, true);
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

		if ( indf && (i+1) < size1 && (i+1) < size0 )
			regFSR0->inc(1, true);	//ASM_CODE(_ADDFSR, indf, 1);
	}
	if ( size0 < size1 && sign0 )
		ASM_CODE(_ADDFSR, INDF1, 1);

	regPCLATH->load(ip2->val.s);
	ASM_CODE(inst, aSTATUS, 2);
	ASM_CODE(_GOTO, ip2->val.s);
}

void PIC16E :: jeqjneBits(int code, Item *ip0, Item *ip1, Item *ip2)
{
	if ( ip0->attr && ip0->attr->type != SBIT )
	{
		jeqjneBits(code, ip1, ip0, ip2);
		return;
	}

	if ( ip1->type == CON_ITEM && (ip1->val.i & ~1) == 0 )
	{
		code = (code == P_JEQ)? (ip1->val.i? P_JNZ: P_JZ):
							    (ip1->val.i? P_JZ: P_JNZ);
		jzjnz(code, ip0, ip2);
	}
	else if ( ip1->attr && ip1->attr->type == SBIT )
	{
		char *inst = (code == P_JEQ)? _BTFSS: _BTFSC;
		regBSR->load(ip0->val.i >> 3);
		ASM_CODE(_MOVF, (ip0->val.i >> 3) & 0x7f, _W_);
		regBSR->load(ip1->val.i >> 3);
		ASM_CODE(_BTFSC, (ip1->val.i >> 3) & 0x7f, ip1->val.i & 7);
		ASM_CODE(_XORLW,  0xff);
		regPCLATH->load(ip2->val.s);
		ASM_CODE(inst, aWREG, ip0->val.i & 7);
		ASM_CODE(_GOTO, ip2->val.s);
		regWREG->reset();
	}
	else
		errPrint("illegal SBIT compare!");
}