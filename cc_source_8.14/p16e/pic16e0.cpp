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

char *PIC16E :: acceItem (Item *ip, int offset, char *indf)
{
	if ( indf ) return indf;
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

	if ( regFSR0->load(fsr, ip, bias) ) return indf;

	switch ( ip->type )
	{
		case ID_ITEM:
			sprintf(s, "(%s)", ip->val.s);
			asm16e->code(_MOVLW, s);
			asm16e->code(_MOVWF, fsrl);
			strcat(s, ">>8");
			asm16e->code(_MOVLW, s);
			asm16e->code(_MOVWF, fsrh);
			break;

		case PID_ITEM:
			if ( ip->attr->dataBank == CONST )
			{
				far_bias = (bias < -32*4 || bias > 31*4);

				sprintf(s, "%s", ip->val.s); call(s);
				asm16e->code(_MOVWF, fsrl);
				strcat(s, "+1");			 call(s);
				asm16e->code(_MOVWF, fsrh);
				if ( far_bias ) {
					asm16e->code(_MOVLW, bias & 0xff);
					asm16e->code(_ADDWF, fsrl, _F_);
					asm16e->code(_MOVLW, bias >> 8);
					asm16e->code(_ADDWFC,fsrh, _F_);
				}
				break;
			}
			// fall through
		case INDIR_ITEM:
			if ( far_bias )
			{
				asm16e->code(_MOVLW, bias & 0xff);
				asm16e->code(_ADDWF, acceItem(ip, 0), _W_);
				asm16e->code(_MOVWF, fsrl);
				asm16e->code(_MOVLW, bias >> 8);
				asm16e->code(_ADDWFC,acceItem(ip, 1), _W_);
				asm16e->code(_MOVWF, fsrh);
			}
			else
			{
				asm16e->code(_MOVF,  acceItem(ip, 0), _W_);
				asm16e->code(_MOVWF, fsrl);
				asm16e->code(_MOVF,  acceItem(ip, 1), _W_);
				asm16e->code(_MOVWF, fsrh);
			}
			break;

		case DIR_ITEM:
			if ( ip->attr->ptrVect[1] == CONST )
			{
				asm16e->code(_MOVLW, ip->val.i);
				asm16e->code(_MOVWF, fsrl);
				asm16e->code(_MOVLW, (ip->val.i >> 8) | 0x80);
				asm16e->code(_MOVWF, fsrh);
			}
			else
			{
				asm16e->code(_MOVLW, ip->val.i);
				asm16e->code(_MOVWF, fsrl);
				asm16e->code(_MOVLW, ip->val.i >> 8);
				asm16e->code(_MOVWF, fsrh);
			}
			break;

		case ACC_ITEM:
			regWREG->load(0x70);
			asm16e->code(_MOVWF, fsrl);
			asm16e->code(_CLRF, fsrh);
			return indf;

		case TEMP_ITEM:
			sprintf(s, "%s", itemStr(ip, 0));
			asm16e->code(_MOVLW, s);
			asm16e->code(_MOVWF, fsrl);
			sprintf(s, "%s>>8", itemStr(ip, 0));
			asm16e->code(_MOVLW, s);
			asm16e->code(_MOVWF, fsrh);
			break;

		default:	// should never happen
			return NULL;//(char*)"!!";
	}

	while ( bias && !far_bias )
	{
		int n;
		if ( bias < 0 ) n = (bias < -32)? -32: bias;
		else			n = (bias >  31)?  31: bias;
		asm16e->code(_ADDFSR, fsr, n);
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
		asm16e->code(_MOVWF, 0x70+i);
	}

	if ( size0 < size )
	{
		if ( sign0 )
		{
			asm16e->code(_MOVLW, 0x00);
			asm16e->code(_BTFSC, 0x70+size0-1, 7);
			asm16e->code(_MOVLW, 0xff);
			regWREG->reset();
		}
		for(int i = size0; i < size; i++)
		{
			if ( sign0 )
				asm16e->code(_MOVWF, 0x70+i);
			else
				asm16e->code(_CLRF, 0x70+i);
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
		asm16e->code(_MOVIW, s);
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
		asm16e->code(_MOVWI, s);
	}
	else
		asm16e->code(_MOVWF, acceItem(ip, offset));
}

bool PIC16E :: isWREG(Item *ip)
{
	return (ip->type == DIR_ITEM &&
		    ip->val.i == aWREG   &&
			ip->acceSize() == 1  &&
		    ip->attr->ptrVect[1] == 0);
}