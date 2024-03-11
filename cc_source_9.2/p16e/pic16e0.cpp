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

////////////////////////////////////////////////////////////
char *PIC16E :: itemStr(Item *ip, int offset)
{
	char *s = STRBUF();
	int n;

	switch ( ip->type )
	{
		case CON_ITEM:
			sprintf(s, "%d", (ip->val.i >> (offset*8)) & 0xff);
			return s;

		case ID_ITEM:
		case PID_ITEM:
			sprintf(s, "%s", ip->val.s);

			if ( offset > 0 )
			{
				int len = strlen(s);
				sprintf(&s[len], "+%d", offset);
			}
			return s;

		case STR_ITEM:
			break;

		case IMMD_ITEM:
		case LBL_ITEM:
			sprintf(s, "(%s)", ip->val.s);
			if ( offset > 0 )
			{
				int len = strlen(s);
				sprintf(&s[len], ">>%d", offset*8);
			}
			return s;

		case TEMP_ITEM:
		case INDIR_ITEM:
			sprintf(s, "%s_$%d", curFunc->name, ip->val.i+1);
			if ( offset > 0 )
			{
				int len = strlen(s);
				sprintf(&s[len], "+%d", offset);
			}
			return s;

		case DIR_ITEM:
			n = ip->val.i + offset;
			if ( ip->attr->ptrVect[1] == CONST )
				sprintf(s, "%d", n | 0x8000);
			else
				sprintf(s, "%d", n & 0x7f);
			return s;

		case ACC_ITEM:
			sprintf(s, "%d", 0x70+offset);
			return s;

		default:
			break;
	}
	return (char*)"";
}

char *PIC16E :: acceItem(Item *ip, int offset, char *indf)
{
	if ( indf ) {
		for (int current_offset = regFSR0->getVarOffset();
			 current_offset != offset;)
		{
			int n = offset - current_offset;
			if ( n > 31 ) n = 31;
			else if ( n < -32 ) n = -32;
			ASM_CODE(_ADDFSR, 0, n);
			current_offset += n;
			regFSR0->inc(n);
		}
		return indf;
	}
	char *s = STRBUF();
	Dnode *dnp = (Dnode*)ip->home;
	int n;

	switch ( ip->type )
	{
		case ID_ITEM:
		case PID_ITEM:
			if ( dnp->func && !ip->attr->isStatic )
			{
				sprintf(s, "%s_$data$", curFunc->name);
				regBSR->load(s);
			}
			else if ( /*ip->attr->dataBank == LINEAR &&*/ offset )
			{
				sprintf(s, "%s+%d", ip->val.s, offset);
				regBSR->load(s);
			}
			else
				regBSR->load(ip->val.s);

			return itemStr(ip, offset);

		case DIR_ITEM:
			if ( ip->attr->type == SBIT )
			{
				n = ip->val.i;
				regBSR->load(n >> 3);
				sprintf(s, "%d, %d", (n >> 3) & 0x7f, n & 7);
				return s;
			}
			n = ip->val.i + offset;
			if ( n >= 0x2000 )
				regBSR->load((n/80) << 7);
			else if ( (n & 0x7f) >= 0x0c && (n & 0x7f) < 0x70 )
				regBSR->load(n);

			return itemStr(ip, offset);

		case TEMP_ITEM:
		case INDIR_ITEM:
			sprintf(s, "%s_$data$", curFunc->name);
			regBSR->load(s);
			return itemStr(ip, offset);

		case IMMD_ITEM:
		case CON_ITEM:
		case ACC_ITEM:
		case STR_ITEM:
		case LBL_ITEM:
			return itemStr(ip, offset);

		default:
			break;
	}
	return (char*)"!";
}

bool PIC16E :: useFSR(Item *ip)
{
	return (ip->type == INDIR_ITEM || ip->type == PID_ITEM);
}
bool PIC16E :: useFSR(Item *ip0, Item *ip1)
{
	return useFSR(ip0) && useFSR(ip1);
}
bool PIC16E :: useFSR(Item *ip0, Item *ip1, Item *ip2)
{
	return useFSR(ip0, ip1) && useFSR(ip2);
}

char *PIC16E :: setFSR(Item *ip, int fsr)
{
	char *fsrl = fsr? FSR1L: FSR0L;
	char *fsrh = fsr? FSR1H: FSR0H;
	char *indf = fsr? INDF1: INDF0;
	char *s = STRBUF();
	int  bias = ip->bias;
	bool far_bias = (bias < -32*2 || bias > 31*2);

	if ( regFSR0->load(fsr, ip, bias) )
		return indf;

	switch ( ip->type )
	{
		case ID_ITEM:
			sprintf(s, "(%s)", ip->val.s);
			ASM_CODE(_MOVLW, s);
			ASM_CODE(_MOVWF, fsrl);
			strcat(s, ">>8");
			ASM_CODE(_MOVLW, s);
			ASM_CODE(_MOVWF, fsrh);
			break;

		case PID_ITEM:
			if ( ip->attr->dataBank == CONST )
			{
				far_bias = (bias < -32*4 || bias > 31*4);

				sprintf(s, "%s", ip->val.s); call(s);
				ASM_CODE(_MOVWF, fsrl);
				strcat(s, "+1");			 call(s);
				ASM_CODE(_MOVWF, fsrh);
				if ( far_bias ) 
				{
					ASM_CODE(_MOVLW, bias & 0xff);
					ASM_CODE(_ADDWF, fsrl, _F_);
					ASM_CODE(_MOVLW, bias >> 8);
					ASM_CODE(_ADDWFC,fsrh, _F_);
				}
				break;
			}
			// fall through
		case INDIR_ITEM:
			if ( far_bias )
			{
				ASM_CODE(_MOVLW, bias & 0xff);
				ASM_CODE(_ADDWF, acceItem(ip, 0), _W_);
				ASM_CODE(_MOVWF, fsrl);
				ASM_CODE(_MOVLW, bias >> 8);
				ASM_CODE(_ADDWFC,acceItem(ip, 1), _W_);
				ASM_CODE(_MOVWF, fsrh);
			}
			else
			{
				ASM_CODE(_MOVF,  acceItem(ip, 0), _W_);
				ASM_CODE(_MOVWF, fsrl);
				ASM_CODE(_MOVF,  acceItem(ip, 1), _W_);
				ASM_CODE(_MOVWF, fsrh);
			}
			break;

		case DIR_ITEM:
			if ( ip->attr->ptrVect[1] == CONST )
			{
				ASM_CODE(_MOVLW, ip->val.i);
				ASM_CODE(_MOVWF, fsrl);
				ASM_CODE(_MOVLW, (ip->val.i >> 8) | 0x80);
				ASM_CODE(_MOVWF, fsrh);
			}
			else
			{
				ASM_CODE(_MOVLW, ip->val.i);
				ASM_CODE(_MOVWF, fsrl);
				ASM_CODE(_MOVLW, ip->val.i >> 8);
				ASM_CODE(_MOVWF, fsrh);
			}
			break;

		case ACC_ITEM:
			regWREG->load(0x70);
			ASM_CODE(_MOVWF, fsrl);
			ASM_CODE(_CLRF, fsrh);
			return indf;

		case TEMP_ITEM:
			sprintf(s, "%s", itemStr(ip, 0));
			ASM_CODE(_MOVLW, s);
			ASM_CODE(_MOVWF, fsrl);
			sprintf(s, "%s>>8", itemStr(ip, 0));
			ASM_CODE(_MOVLW, s);
			ASM_CODE(_MOVWF, fsrh);
			break;

		default:	// should never happen
			return NULL;//(char*)"!!";
	}
	
	if ( fsr == 0 )
	{	
		regFSR0->setPtrOffset(bias);
		regFSR0->setVarOffset(0);
	}
	
	while ( !far_bias && bias ) 
	{
		int n = bias;
		if ( n < -32 ) n = -32;
		else if ( n > 31 ) n = 31;
		ASM_CODE(_ADDFSR, fsr, n);
		bias -= n;
	}

	regWREG->reset();
	return indf;
}

char *PIC16E :: setFSR0(Item *ip)
{
	return useFSR(ip)? setFSR(ip, 0): NULL;
}

char *PIC16E :: setFSR1(Item *ip)
{
	return setFSR(ip, 1);
}

Item *PIC16E :: storeToACC(Item *ip, int size)
{
	int  size0 = ip->acceSize();
	bool sign0 = ip->acceSign();
	attrib *attr = newAttr((size == 1)? CHAR:
						   (size == 2)? INT:
						   (size == 3)? SHORT:
										LONG);
	attr->isUnsigned = sign0? 0: 1;
	Item *acc = accItem(attr);
	char *indf0 = setFSR0(ip);
	for(int i = 0; i < size0 && i < size && ip->type != ACC_ITEM; i++)
	{
		fetch(ip, i, indf0);
		ASM_CODE(_MOVWF, 0x70+i);
	}

	if ( size0 < size )
	{
		if ( sign0 )
		{
			ASM_CODE(_MOVLW, 0x00);
			ASM_CODE(_BTFSC, 0x70+size0-1, 7);
			ASM_CODE(_MOVLW, 0xff);
			regWREG->reset();
		}
		for(int i = size0; i < size; i++)
		{
			if ( sign0 )
				ASM_CODE(_MOVWF, 0x70+i);
			else
				ASM_CODE(_CLRF, 0x70+i);
		}
	}
	return acc;
}

void PIC16E :: fetch(Item *ip, int offset, char *indf)
{
	if ( indf )
	{
		char *s = STRBUF();
		sprintf(s, "%d[%s]", offset, indf);
		ASM_CODE(_MOVIW, s);
//		regWREG->loaded(ip, offset);
	}
	else
	{
		switch ( ip->type )
		{
			case CON_ITEM:
				regWREG->load(ip->val.i >> (offset*8));
				return;

			case IMMD_ITEM:
			case LBL_ITEM:
				if ( offset < 2 )
				{
					regWREG->load(acceItem(ip, offset));
//					regWREG->loaded(ip, offset);
				}
				else
					regWREG->load(0);
				return;

			case ID_ITEM:
				if ( ip->attr->dataBank == CONST )
				{
					char *s = STRBUF();
					if ( offset == 0 )
						sprintf(s, "%s", ip->val.s);
					else
						sprintf(s, "%s+%d", ip->val.s, offset);

					call(s);
					break;
				}
			default:
				if ( isWREG(ip) )
					return;
				regWREG->load(_MOVF, acceItem(ip, offset));
		}
//		regWREG->loaded(ip, offset);
	}
}

void PIC16E :: store(Item *ip, int offset, char *indf)
{
	if ( indf )
	{
		char *s = STRBUF();
		sprintf(s, "%d[%s]", offset, indf);
		ASM_CODE(_MOVWI, s);
	}
	else
		ASM_CODE(_MOVWF, acceItem(ip, offset));
}

bool PIC16E :: isWREG(Item *ip)
{
	return (ip->type == DIR_ITEM &&
		    ip->val.i == aWREG   &&
			ip->acceSize() == 1  &&
		    ip->attr->ptrVect[1] == 0);
}