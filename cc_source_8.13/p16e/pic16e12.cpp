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

static int logicAssign(int code)
{
	switch ( code )
	{
		case '&':	return AND_ASSIGN;
		case '|':	return OR_ASSIGN;
		case '^':	return XOR_ASSIGN;
	}
	return 0;
}

void PIC16E :: andorxor(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int size2 = ip2->acceSize();
	
	if ( same(ip1, ip2) )
	{
		mov(ip0, ip1);
		return;
	}
	if ( same(ip0, ip1) )
	{
		andorxor(logicAssign(code), ip0, ip2);
		return;
	}
	if ( same(ip0, ip2) )
	{
		andorxor(logicAssign(code), ip0, ip1);
		return;
	}
	bool dir0 = (size0 == 1 && ip0->type == DIR_ITEM && ip0->attr->isVolatile);
	if ( dir0 && (useFSR(ip1, ip2) || (CONST_ID_ITEM(ip1) && CONST_ID_ITEM(ip2))) )
	{
		Item *acc = storeToACC(ip1, 1);
		andorxor(code, ip0, ip2, acc);
		delete acc;
		return;
	}
	if ( useFSR(ip0, ip2) )
	{
		mov(ip0, ip2);
		andorxor(logicAssign(code), ip0, ip1);
		return;
	}
	if ( useFSR(ip0, ip1) || useFSR(ip1, ip2) )
	{
		mov(ip0, ip1);
		andorxor(logicAssign(code), ip0, ip2);
		return;
	}
	if ( ip1->type == CON_ITEM )
	{
		andorxor(code, ip0, ip2, ip1->val.i);
		return;
	}
	if ( ip2->type == CON_ITEM )
	{
		andorxor(code, ip0, ip1, ip2->val.i);
		return;
	}
	if ( CONST_ID_ITEM(ip1) && CONST_ID_ITEM(ip2) )
	{
		mov(ip0, ip1);
		andorxor(logicAssign(code), ip0, ip2);
		return;
	}
	if ( CONST_ID_ITEM(ip2) )
	{
		andorxor(code, ip0, ip2, ip1);
		return;
	}

	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);
	char *indf2 = setFSR0(ip2);
	char *inst  = (code == '&')? _ANDWF:
				  (code == '|')? _IORWF: _XORWF;
	char *inst2 = (code == '&')? _ANDLW:
				  (code == '|')? _IORLW: _XORLW;

	for(int i = 0; i < size0; i++)
	{
		bool zero_fill = false;
		switch ( code )
		{
			case '|':	case '^':
				if ( i >= size1 && i >= size2 ) zero_fill = true;
				break;

			case '&':
				if ( i >= size1 || i >= size2 )	zero_fill = true;
				break;
		}

		if ( zero_fill )
		{
			if ( !indf0 )
			{
				ASM_CODE(_CLRF, acceItem(ip0, i));
				continue;
			}
			regWREG->load(0);
		}
		else
		{
			if ( i < size1 )
			{
				fetch(ip1, i, indf1);
				if ( i < size2 )
				{
					if ( CONST_ITEM(ip2) )
						ASM_CODE(inst2, acceItem(ip2, i), _W_);
					else
						ASM_CODE(inst, acceItem(ip2, i, indf2), _W_);
					regWREG->reset();
				}				
			}
			else
				asm16e->code(_MOVF, acceItem(ip2, i, indf2), _W_);
		}

		store(ip0, i, indf0);
		if ( indf2 && (i+1) < size2 && (i+1) < size0 && !zero_fill )
			ASM_CODE(_ADDFSR, indf2, 1);
	}
}

void PIC16E :: andorxor(int code, Item *ip0, Item *ip1, int n)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	char *indf0 = setFSR0(ip0);
	char *indf1 = setFSR0(ip1);

	for (int i = 0; i < size0; i++, n >>= 8)
	{
		int  num = n & 0xff;
		bool load_ip0_num = false;
		switch ( code )
		{
			case '|':
				if ( i < size1 && num != 0xff )
				{
					fetch(ip1, i, indf1);
					if ( num )
					{
						ASM_CODE(_IORLW, num);
						regWREG->reset();
					}
				}
				else
					load_ip0_num = true;
				break;

			case '^':
				if ( i < size1 )
				{
					if ( num == 0xff && !(indf1 || CONST_ITEM(ip1)) )
					{
						ASM_CODE(_COMF, acceItem(ip1, i), _W_);
						regWREG->reset();
					}
					else
					{
						fetch(ip1, i, indf1);
						if ( num )
						{
							ASM_CODE(_XORLW, num);
							regWREG->reset();
						}
					}
				}
				else
					load_ip0_num = true;
				break;

			case '&':
				if ( i < size1 && num )
				{
					fetch(ip1, i, indf1);
					if ( num != 0xff )
					{
						ASM_CODE(_ANDLW, num);
						regWREG->reset();
					}
				}
				else 
				{
					num = 0;
					load_ip0_num = true;
				}
				break;
		}
		
		if ( load_ip0_num )
		{
			if ( !indf0 && num == 0 )
			{
				ASM_CODE(_CLRF, regWREG->reset(acceItem(ip0, i)));
				continue;
			}
			regWREG->load(num);
		}
		store(ip0, i, indf0);
	}
}
