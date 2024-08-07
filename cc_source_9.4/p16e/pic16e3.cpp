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

void PIC16E :: incValue(Item *ip, int value)
{
	int  size0 = ip->acceSize();
	char *indf = setFSR0(ip);
	bool started = false;
	if ( value == 1 &&
		 !(ip->type == INDIR_ITEM || ip->type == PID_ITEM || ip->attr->dataBank == LINEAR) )
   	{
		char *ds = acceItem(ip, 0, indf);
		ASM_CODE(_INCF, regWREG->reset(ds), _F_);
		for (int i = 1; i < size0; i++)
		{
			ds = acceItem(ip, i, indf);
			ASM_CODE(_BTFSC, aSTATUS, 2);
			ASM_CODE(_INCF, regWREG->reset(ds), _F_);
		}
		return;
	}
	for (int i = 0; i < size0; i++, value >>= 8)
	{
		int v = value & 0xff;
		if ( (v == 1 || v == 0xff) && !started && (i+1) == size0 )
		{
			if ( v == 1 )
				ASM_CODE(_INCF, acceItem(ip, i, indf), _F_);
			else
				ASM_CODE(_DECF, acceItem(ip, i, indf), _F_);
		}
		else if ( !(v == 0 && !started) )
		{
			regWREG->load(v);
			if ( started )
				ASM_CODE(_ADDWFC, acceItem(ip, i, indf), _F_);
			else
				ASM_CODE(_ADDWF,  acceItem(ip, i, indf), _F_);
			started = true;
		}
	}
}

void PIC16E :: compl2(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	char *indf0 = setFSR0(ip0);
	ASM_CODE(_BSF, aSTATUS, 0);
	for(int i = 0; i < size0; i++)
	{
		if ( i < size1 )
			fetch(ip1, i, NULL);
		else if ( i == size1 && SIGNED(ip1) )
			EXTEND_WREG;
		else
			regWREG->load(0);

		ASM_CODE(_COMF, acceItem(ip0, i, indf0), _F_);
		ASM_CODE(_ADDWFC, acceItem(ip0, i, indf0), _F_);
	}
}

void PIC16E :: compl2(Item *ip0)
{
	int  size0 = ip0->acceSize();
	char *indf = setFSR0(ip0);
	for (int i = 0; i < size0; i++)
	{
		char *ds = acceItem(ip0, i, indf);
		ASM_CODE(_COMF, regWREG->reset(ds), _F_);
		
		if ( size0 == 1 )
			ASM_CODE(_INCF, ds, _F_);
		else
		{		
			char *inst = i? _ADDWFC: _ADDWF;
			regWREG->load(i? 0: 1);
			ASM_CODE(inst, ds, _F_);
		}
	}
}

void PIC16E :: neg(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	char *indf0, *indf1;
	if ( same(ip0, ip1) )
	{
		compl2(ip0);
		return;
	}

	if ( size0 == 1 )
	{
		if ( CONST_ID_ITEM(ip1) )
		{
			indf0 = setFSR0(ip0);
			fetch(ip1, 0, NULL);
			ASM_CODE(_SUBLW, 0);
		}
		else if ( useFSR(ip0, ip1) )
		{
			indf1 = setFSR0(ip1);
			ASM_CODE(_COMF, acceItem(ip1, 0, indf1), _W_);
			ASM_CODE(_MOVWF, ACC0);
			indf0 = setFSR0(ip0);			
			ASM_CODE(_INCF, ACC0, _W_);
		}
		else
		{
			indf1 = setFSR0(ip1);
			indf0 = setFSR0(ip0);
			ASM_CODE(_COMF, acceItem(ip1, 0, indf1), _W_);
			ASM_CODE(_ADDLW, 1);
		}
		store(ip0, 0, indf0);
		regWREG->reset();
		return;
	}

	if ( useFSR(ip0, ip1) )
	{
		mov(ip0, ip1);
		compl2(ip0);
		return;
	}

	indf0 = setFSR0(ip0);
	indf1 = setFSR0(ip1);
	for(int i = 0; i < size0; i++)
	{
		if ( i < size1 )
		{
			fetch(ip1, i, indf1);
			if ( i == 0 )
				ASM_CODE(_SUBWF, ZERO_LOC, _W_);
			else
				ASM_CODE(_SUBWFB, ZERO_LOC, _W_);
		}
		else if ( i == size1 )
		{
			if ( SIGNED(ip1) )
				EXTEND_WREG;
			else
			{
				ASM_CODE(_MOVLW, 0);
				ASM_CODE(_SUBWFB, ZERO_LOC, _W_);
			}
		}
		store(ip0, i, indf0);
		regWREG->reset();
	}
}

void PIC16E :: compl1(Item *ip0)
{
	int size0  = ip0->acceSize();
	char *indf = setFSR0(ip0);
	for (int i = 0; i < size0; i++)
	{
		char *ds = acceItem(ip0, i, indf);
		ASM_CODE(_COMF, regWREG->reset(ds), _F_);
	}
}

void PIC16E :: compl1(Item *ip0, Item *ip1)
{
	if ( same(ip0, ip1) )
	{
		compl1(ip0);
		return;
	}

	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	char *indf0, *indf1;
	if ( size0 == 1 )
	{
		if ( CONST_ID_ITEM(ip1) )
		{
			indf0 = setFSR0(ip0);
			fetch(ip1, 0, NULL);
			ASM_CODE(_XORLW, 0xff);
		}
		else if ( useFSR(ip0, ip1) )
		{
			indf1 = setFSR0(ip1);
			ASM_CODE(_COMF, acceItem(ip1, 0, indf1), _W_);
			ASM_CODE(_MOVWF, ACC0);
			indf0 = setFSR0(ip0);
			ASM_CODE(_MOVF, ACC0, _W_);
		}
		else
		{
			indf1 = setFSR0(ip1);
			indf0 = setFSR0(ip0);
			ASM_CODE(_COMF, acceItem(ip1, 0, indf1), _W_);
		}
		store(ip0, 0, indf0);
		regWREG->reset();
		return;
	}

	if ( useFSR(ip0, ip1) )
	{
		mov(ip0, ip1);
		compl1(ip0);
		return;
	}

	indf0 = setFSR0(ip0);
	indf1 = setFSR0(ip1);
	for(int i = 0; i < size0; i++)
	{
		if ( i < size1 )
		{
			if ( CONST_ID_ITEM(ip1) )
			{
				fetch(ip1, i, NULL);
				ASM_CODE(_XORLW, 0xff);
			}
			else
				ASM_CODE(_COMF, acceItem(ip1, i, indf1), _W_);
		}
		else if ( i == size1 )
		{
			ASM_CODE(_MOVLW, 0xff);
		}
		store(ip0, i, indf0);
		regWREG->reset();
	}
}

void PIC16E :: notop(Item *ip0, Item *ip1)
{
	int size1 = ip1->acceSize();
	char *indf= setFSR0(ip1);

	if ( ip1->type == DIR_ITEM && ip1->attr->type == SBIT )
	{
		ASM_CODE(_CLRF, ACC0);
		ASM_CODE(_BTFSS, acceItem(ip1, 0));
		ASM_CODE(_INCF, ACC0, _F_);
	}
	else if ( CONST_ID_ITEM(ip1) )
	{
		ASM_CODE(_CLRF, ACC0);
		ASM_CODE(_CLRF, ACC1);
		for (int i = 0; i < size1; i++)
		{
			fetch(ip1, i, indf);
			ASM_CODE(_IORWF, ACC1, _F_);
		}
		ASM_CODE(_BTFSC, STATUS, 2);
		ASM_CODE(_INCF, ACC0, _F_);
	}
	else
	{
		fetch(ip1, 0, indf);
		for(int i = 1; i < size1; i++)
			ASM_CODE(_IORWF, acceItem(ip1, i, indf), _W_);

		ASM_CODE(_CLRF, ACC0);
		ASM_CODE(_ADDLW, 0);
		ASM_CODE(_BTFSC, STATUS, 2);
		ASM_CODE(_INCF, ACC0, _F_);
	}

	attrib *attr = newAttr(CHAR);
	attr->isUnsigned = 1;
	Item *acc = accItem(attr);
	mov(ip0, acc);
	delete acc;
	regWREG->reset();
}
