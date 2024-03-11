#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
extern "C" {
#include "../common.h"
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../item.h"
#include "pic16e.h"
#include "pic16e_reg.h"
#include "pic16e_inst.h"

#define AS_CODE		pic16e->code

//////////////////////////////////////////////////////////
P16E_FSR0 :: P16E_FSR0(P16E_ASM *p16e)
{
	pic16e = p16e;
	reset();
}

bool P16E_FSR0 :: loaded(Item *_ip)
{
	return (ip && same(ip, _ip));
}

bool P16E_FSR0 :: load(int fsr, Item *_ip, int _offset)
{
	if ( fsr ) return false;

	if ( ip && same(ip, _ip, true) && !_ip->attr->isVolatile )
	{
		int adjustment = _offset - offset;
		if ( adjustment >= -(32*4) && adjustment <= (31*4) )
		{
			while ( adjustment )
			{
				int n;
				if ( adjustment < 0 )
					n = (adjustment >= -32)? adjustment: -32;
				else
					n = (adjustment <=  31)? adjustment:  31;

				AS_CODE(_ADDFSR, INDF0, n);
				adjustment -= n;
			}
		}
		else
		{
			AS_CODE(_MOVLW, adjustment);
			AS_CODE(_ADDWF, FSR0L, _F_);
			AS_CODE(_MOVLW, adjustment >> 8);
			AS_CODE(_ADDWFC, FSR0H, _F_);
			regWREG->reset();
		}
		offset = _offset;
		return true;
	}
	switch ( _ip->type )
	{
		case ID_ITEM:
		case PID_ITEM:
		case DIR_ITEM:
			ip = _ip;
			offset = _offset;
			break;
		default:
			ip = NULL;
			break;
	}
	return false;
}

void P16E_FSR0 :: inc(int amount, bool asm_output)
{
	if ( asm_output ) AS_CODE(_ADDFSR, 0, amount);
	offset += amount;
}

void P16E_FSR0 :: reset(void)
{
	ip = NULL;
}

void P16E_FSR0 :: reset(Item *_ip)
{
	if ( ip && _ip && _ip->type == ID_ITEM && strcmp(ip->val.s, _ip->val.s) == 0 )
		ip = NULL;

	if ( _ip->type == DIR_ITEM && (_ip->val.i == aFSR0L || _ip->val.i == aFSR0H) )
		ip = NULL;
}

/////////////////////////////////////////////////////////////////////////////////
P16E_REG8 :: P16E_REG8(char *_name, P16E_ASM *p16e)
{
	name   = _name;
	pic16e = p16e;
	reset();
}

void P16E_REG8 :: reset(void)
{
	valStr  = "";
	instStr = "";
}

char*P16E_REG8 :: reset(char *val)
{
	if ( name == WREG )
	{
		if ( valStr == val && instStr != _MOVLW )
			reset();
	}
	return val;
}

void P16E_REG8 :: update(char *i, char *s)
{
	instStr = i;
	valStr  = s;
}

void P16E_REG8 :: load(char *inst, char *val)
{
	if ( name == WREG )
	{
		int length = strlen(val);
		char buf[1024]; strcpy(buf, val);
		while ( buf[0] == '(' && buf[length-1] == ')' )
		{
			length -= 2;
			memcpy(buf, &buf[1], length); buf[length] = 0;
		}

		bool volatile_val = strcmp(inst, _MOVLW) &&
							(strchr(val, '[') 	|| isdigit(buf[0]) ||
							 strstr(val, INDF0) || strstr(val, INDF1));

		if ( valStr == buf && instStr == inst && instStr != _MOVIW && !volatile_val )
			return;

		if ( strcmp(inst, _MOVF) == 0 )
			AS_CODE(inst, buf, _W_);
		else
			AS_CODE(inst, buf);

		if ( volatile_val ) reset();
		else update(inst, buf);
	}
}

void P16E_REG8 :: load(char *inst, int val)
{
	if ( name == WREG )
	{
		char buf[32];

		if ( strcmp(inst, _MOVLW) == 0 ) val &= 0xff;
		if ( strcmp(inst, _MOVF)  == 0 ) val &= 0x7f;

		sprintf(buf, "%d", val);
		load(inst, buf);
	}
}

void P16E_REG8 :: load(int val)
{
	if ( name == WREG )
	{
		load(_MOVLW, val);
	}
	else if ( name == BSR )
	{
		char buf[32]; sprintf(buf, "%d", val);
		char *c_str = (char*)valStr.c_str();

		if ( (val & 0x7f) < 0x0c || (val & 0x7f) >= 0x70 )
			return;

		if ( valStr == "" )
		{
			AS_CODE(_MOVLB, val >> 7);
			update(_MOVLB, buf);
		}
		else
		{
			AS_CODE(_BSEL, c_str, buf);
			update(_BSEL, buf);
		}
		valStr = buf;
	}
	else
	{
		val &= 0x7fff;
		char buf[32]; sprintf(buf, "%d", val);
		char *c_str = (char*)valStr.c_str();
		int cur_val = atoi(c_str);

		if ( valStr == "" )
		{
			AS_CODE(_MOVLP, val >> 8);
			update(_MOVLP, buf);
		}
		else if ( !isdigit(c_str[0]) )
		{
			AS_CODE(_PSEL, c_str, buf);
			update(_PSEL, buf);
		}
		else if ( (cur_val ^ val) & (0xf << 11) )
		{
			AS_CODE(_MOVLP, val >> 8);
			update(_MOVLP, buf);
		}
		valStr = buf;
	}
}

void P16E_REG8 :: load(char *val)
{
	if ( name == WREG )
		load(_MOVLW, val);
	else if ( name == BSR )
	{
		if ( valStr == "" )
			AS_CODE(_BSEL, val);
		else if ( valStr != val )
			AS_CODE(_BSEL, (char*)valStr.c_str(), val);

		update(_BSEL, val);
	}
	else if ( name == PCLATH )
	{
		if ( valStr == "" )
			AS_CODE(_PSEL, val);
		else if ( valStr != val )
			AS_CODE(_PSEL, (char*)valStr.c_str(), val);

		update(_PSEL, val);
	}
}

void P16E_REG8 :: set(int val)
{
	char buf[32]; sprintf(buf, "%d", val);

	if ( name == BSR || name == PCLATH )
		update((char*)"", buf);
	else
		update(_MOVLW, buf);
}

void P16E_REG8 :: set(char *val)
{
	if ( name == PCLATH )
		update((char*)"", val);
}