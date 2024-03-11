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

void PIC16E :: jzjnz(int code, Item *ip0, Item *ip1)
{
	if ( ip0->attr && ip0->attr->type == SBIT )
	{
		char *inst = (code == P_JZ)? _BTFSS: _BTFSC;
		regPCLATH->load(ip1->val.s);
		ASM_CODE(inst, acceItem(ip0, 0));
		ASM_CODE(_GOTO, ip1->val.s);
		return;
	}

	int size0 = ip0->acceSize();
	char *indf0 = setFSR0(ip0);
	char *inst = (code == P_JZ)? _BTFSC: _BTFSS;
	for (int i = 0; i < size0; i++)
	{
		if ( i == 0 )
		{
			fetch(ip0, i, indf0);
			if ( CONST_ID_ITEM(ip0) && size0 > 1 )
				ASM_CODE(_MOVWF, ACC0);
		}
		else if ( CONST_ID_ITEM(ip0) )
		{
			fetch(ip0, i, indf0);
			ASM_CODE(_IORWF, ACC0, _F_);
		}
		else
		{
			ASM_CODE(_IORWF, acceItem(ip0, i, indf0), _W_);
			regWREG->reset();
		}

		if ( indf0 && (i+1) < size0 )
			regFSR0->inc(1, true);
	}

	if ( CONST_ID_ITEM(ip0) && size0 == 1 )
		ASM_CODE(_IORLW, 0);	// trigger Z flag

	regPCLATH->load(ip1->val.s);
	ASM_CODE(inst, aSTATUS, 2);
	ASM_CODE(_GOTO, ip1->val.s);
}

void PIC16E :: jbzjbnz(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	char *indf0 = setFSR0(ip0);
	char *inst = (code == P_JBZ)? _BTFSS: _BTFSC;
	int  offset = ip1->val.i/8;
	char *des;

	if ( ip1->val.i >= (size0*8) ) return;

	if ( CONST_ID_ITEM(ip0) )
	{
		fetch(ip0, offset, NULL);
		des = WREG;
	}
	else if ( indf0 && offset > 0 )
	{
		regFSR0->inc(offset, true);
		des = acceItem(ip0, 0, indf0);
	}
	else
		des = acceItem(ip0, offset, indf0);

	regPCLATH->load(ip2->val.s);
	ASM_CODE(inst, des, ip1->val.i%8);
	ASM_CODE(_GOTO, ip2->val.s);
}

void PIC16E :: passarg(Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();	// arg size
	int size1 = ip1->acceSize();	// data size
	char *indf1 = setFSR0(ip1);
	for (int i = 0; i < size0; i++)
	{
		if ( i < size1 )
			fetch(ip1, i, indf1);
		else if ( i == size1 )
		{
			if ( ip1->acceSign() )
				EXTEND_WREG;
			else
				regWREG->load(0);
		}
		ASM_CODE(_MOVWI, "--INDF1");
	}
}

void PIC16E :: pcall(Item *ip)
{
	char *indf = setFSR0(ip);
	fetch(ip, 0, indf);	// Lo byte of pointer -> stack
	ASM_CODE(_MOVWI, "--INDF1");
	fetch(ip, 1, indf);	// Hi byte of pointer -> WREG
	// system lib function call...
	call((char*)"_pcall");
}

void PIC16E :: jzjnz_incdec(int code, Item *ip0, Item *ip1, Item *ip2)
{
	char *indf = setFSR0(ip0);
	int  size0 = ip0->acceSize();
	int  n = (code == P_JZ_INC || code == P_JNZ_INC)? ip1->val.i: -ip1->val.i;
	char *inst = (code == P_JZ_INC || code == P_JZ_DEC)? _BTFSC: _BTFSS;

	if ( size0 == 1 && abs(n) == 1 )
	{
		ASM_CODE((n>0)? _INCF: _DECF, acceItem(ip0, 0, indf), _F_);
		ASM_CODE((n<0)? _INCF: _DECF, acceItem(ip0, 0, indf), _W_);
	}
	else
	{
		fetch(ip0, 0, indf);
		for (int i = 1; i < size0; i++)
		{
			ASM_CODE(_IORWF, acceItem(ip0, i, indf), _W_);
		}
		ASM_CODE(_MOVWF, ACC0);
		for (int i = 0; i < size0; i++)
		{
			regWREG->load(n>>(i*8));
			ASM_CODE(i? _ADDWFC: _ADDWF, acceItem(ip0, i, indf), _F_);
		}
		ASM_CODE(_MOVF, ACC0, _W_);
	}
	regWREG->reset();
	regPCLATH->load(ip2->val.s);
	ASM_CODE(inst, aSTATUS, 2);
	ASM_CODE(_GOTO, ip2->val.s);
}

void PIC16E :: addsub(int code, Item *ip0, Item *ip1, Item *ip2)
{
	if ( useFSR(ip0, ip1) || useFSR(ip0, ip2) || useFSR(ip1, ip2) ) {
		call((char*)"_saveFSR1");
		regWREG->reset();
	}

	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	int  size2 = ip2->acceSize();
	char *indf0 = setFSR0(ip0);
	char *indf1 = useFSR(ip1)? setFSR(ip1, indf0? 		   1: 0): NULL;
	char *indf2 = useFSR(ip2)? setFSR(ip2, (indf0||indf1)? 1: 0): NULL;
	bool started = false;
	bool ip1_const = CONST_ITEM(ip1);
	char *inst = (code == '+')? _ADDWF: _SUBWF;

	for (int i = 0; i < size0; i++)
	{
		if ( ip2->type == CON_ITEM )
		{
			int n = ip2->val.i >> (i*8);
			if ( (n & 0xff) == 0 && !started )
			{
				if ( indf1 )
					ASM_CODE(_MOVIW, INDF_PP(indf1));
				else
					fetch(ip1, i, NULL);

				store(ip0, i, indf0);
				continue;
			}
		}

		if ( ip1_const && started ) {
			fetch(ip1, i, NULL);
			store(ip0, i, indf0);
			fetch(ip2, i, indf2);
			ASM_CODE(inst, acceItem(ip0, i, indf0), _F_);
		}
		else {
			if ( i >= size2 )
				ASM_CODE(_CLRW);
			else
				fetch(ip2, i, indf2);
			if ( ip1_const ) {
				inst = (code == '+')? _ADDLW: _SUBLW;
				ASM_CODE(inst, acceItem(ip1, i, indf1));
			}
			else {
				if ( i >= size1 )
					ASM_CODE(inst, ZERO_LOC, _W_);
				else
					ASM_CODE(inst, acceItem(ip1, i, indf1), _W_);
			}
			store(ip0, i, indf0);
		}
		started = true;
		regWREG->reset();

		inst = (code == '+')? _ADDWFC: _SUBWFB;
		if ( indf1 && (i+1) < size0 ) ASM_CODE(_ADDFSR, indf1, 1);
	}
	if ( useFSR(ip0, ip1) || useFSR(ip0, ip2) || useFSR(ip1, ip2) ) {
		call((char*)"_restoreFSR1");
		regWREG->reset();
	}
}

void PIC16E :: sub(Item *ip0, Item *ip1, Item *ip2)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	int  size2 = ip2->acceSize();

	if ( same(ip0, ip1) )
	{
		addsub(SUB_ASSIGN, ip0, ip2);
		return;
	}
	if ( same(ip0, ip2) )
	{
		neg(ip0, ip2);
		addsub(ADD_ASSIGN, ip0, ip1);
		return;
	}
	if ( useFSR(ip0, ip1, ip2) )
	{
		Item *acc = storeToACC(ip1, size0);
		addsub(SUB_ASSIGN, acc, ip2);
		mov(ip0, acc);
		delete acc;
		return;
	}
	if ( CONST_ID_ITEM(ip1) )
	{
		mov(ip0, ip1);
		addsub(SUB_ASSIGN, ip0, ip2);
		return;
	}
	if ( (size2 < size0 && !CONST_ITEM(ip2) && ip2->acceSign()) ||
		 (size1 < size0 && !CONST_ITEM(ip1) && ip1->acceSign()) )
	{
		mov(ip0, ip1);
		addsub(SUB_ASSIGN, ip0, ip2);
		return;
	}
	addsub('-', ip0, ip1, ip2);
	regFSR0->reset(ip0);
}
void PIC16E :: add(Item *ip0, Item *ip1, Item *ip2)
{
	int  size0 = ip0->acceSize();
	int  size1 = ip1->acceSize();
	int  size2 = ip2->acceSize();

	if ( overlap(ip0, ip1) )
	{
		addsub(ADD_ASSIGN, ip0, ip2);
		return;
	}
	if ( overlap(ip0, ip2) )
	{
		addsub(ADD_ASSIGN, ip0, ip1);
		return;
	}
	if ( CONST_ITEM(ip1) && !CONST_ITEM(ip2) )
	{
		add(ip0, ip2, ip1);
		return;
	}
	if ( useFSR(ip0, ip1, ip2) )
	{
		Item *acc = storeToACC(ip1, size0);
		addsub(ADD_ASSIGN, acc, ip2);
		mov(ip0, acc);
		delete acc;
		return;
	}
	if ( CONST_ID_ITEM(ip1) )
	{
		mov(ip0, ip1);
		addsub(ADD_ASSIGN, ip0, ip2);
		return;
	}
	if ( (size2 < size0 && !CONST_ITEM(ip2) && ip2->acceSign()) ||
		 (size1 < size0 && !CONST_ITEM(ip1) && ip1->acceSign())  )
	{
		mov(ip0, 				related(ip0, ip2)? ip2: ip1);
		addsub(ADD_ASSIGN, ip0, related(ip0, ip2)? ip1: ip2);
		return;
	}
	addsub('+', ip0, ip1, ip2);
	regFSR0->reset(ip0);
}
