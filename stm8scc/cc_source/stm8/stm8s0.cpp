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
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../const.h"
#include "../nlist.h"
#include "../option.h"
#include "stm8s.h"
#include "stm8s_reg.h"
#include "stm8s_inst.h"
#include "stm8s_asm.h"

int STM8S :: localOffset(Item *ip)
{
	char *buf = STRBUF(), *p = NULL;
	LocalData *ldata = NULL;
	switch ( ip->type )
	{
		case TEMP_ITEM:
		case INDIR_ITEM:
			sprintf(buf, "%s_$%d", curFunc->name, ip->val.i+1);
			ldata = getLocalData(localDataList, buf);
			break;

		case ID_ITEM:
			if ( isdigit(ip->val.s[0]) )
				return atoi(ip->val.s);
		case PID_ITEM:
		case IMMD_ITEM:
		case LBL_ITEM:
			p = ip->val.s;
			ldata = getLocalData(localDataList, p);
			break;

		default:
			break;
	}

	int offset = 0;
	if ( ldata )
	{
		offset = ldata->offset + frameOffset;
		while ( p && (p = strchr(p, '+')) != NULL )
		{	// add offset for struct/union member
			p++;
			offset += atoi(p);
		}
	}
	return offset;
}

////////////////////////////////////////////////////////////
char *STM8S :: itemStr(Item *ip, int offset, int acce_size, char *reg)
{
	char *s = STRBUF();
	int n, size = ip->acceSize();
	int f_offset = size - (offset + acce_size) + ip->bias;

	if ( reg )
	{
		if ( farAcce(ip) )
		{
			STM8S_reg *indx = (strcmp(reg, REG_X) == 0)? Reg_X: Reg_Y;
			switch ( f_offset - indx->bias )
			{
				case  0: 	break;
				case  2:	ASM_CODE1(_INCW, reg);
				case  1: 	ASM_CODE1(_INCW, reg);	break;
				case -2:	ASM_CODE1(_DECW, reg);
				case -1:	ASM_CODE1(_DECW, reg);	break;
				default:	ASM_CODE2(_ADDW, reg, f_offset - indx->bias, '#');
			}
			indx->bias = f_offset;

			sprintf(s, "([4], %s)", reg);
			if ( ip->type == PID_ITEM && !isLocal(ip) && ip->attr->dataBank != CONST )
				sprintf(s, "([%s], %s)", ip->val.s, reg);
		}
		else if ( f_offset > 0 )
			sprintf(s, "(%d, %s)", f_offset, reg);
		else
			sprintf(s, "(%s)", reg);

		return s;
	}

	if ( localOffset(ip) > 0 )	// local variables
	{
		if ( ip->type == PID_ITEM || ip->type == INDIR_ITEM )
			f_offset = ip->storSize() - (offset + acce_size);

		sprintf(s, "(%d, %s)", f_offset + localOffset(ip), REG_SP);
		return s;
	}

	switch ( ip->type )
	{
		case CON_ITEM:
			n  = (ip->val.i >> (offset*8));
			n &= (1 << (acce_size*8)) - 1;
			sprintf(s, "%d", n);
			return s;

		case PID_ITEM:
			if ( !isLocal(ip) )
			{
				if ( ip->attr->dataBank == BANK0 )
					sprintf(s, "[*%s]", ip->val.s);
				else
					sprintf(s, "[%s]", ip->val.s);
				return s;
			}
		case ID_ITEM:
			f_offset = ip->storSize() - (offset + acce_size);
			if ( ip->attr->dataBank == BANK0 )
				sprintf(s, "*%s", ip->val.s);
			else
				sprintf(s, "%s", ip->val.s);

			if ( f_offset > 0 )
				sprintf(&s[strlen(s)], "+%d", f_offset);
			return s;

		case STR_ITEM:
			break;

		case IMMD_ITEM:
		case LBL_ITEM:
			sprintf(s, "(%s)", ip->val.s);
			if ( offset > 0 )
				sprintf(&s[strlen(s)], ">>%d", offset*8);
			return s;

		case DIR_ITEM:
			sprintf(s, "%d", ip->val.i + f_offset);
			return s;

		case ACC_ITEM:
			sprintf(s, "%d", f_offset);
			return s;

		default:
			break;
	}
	return (char*)"";
}

char *STM8S :: acceItem(Item *ip, int offset, int acce_size, char *reg)
{
	char *s, *str = STRBUF();
	switch ( ip->type )
	{
		case CON_ITEM:
			s = itemStr(ip, offset, acce_size, reg);
			sprintf(str, "#%s", s);
			return str;

		case IMMD_ITEM:
		case LBL_ITEM:
			s = itemStr(ip, offset, acce_size, reg);
			sprintf(str, "#%s", s);
			return str;

		default:
			return itemStr(ip, offset, acce_size, reg);
	}
}

char *STM8S :: itemStrI(Item *ip, int offset, int acce_size)
{
	char *s = STRBUF();
	sprintf(s, "#%s", itemStr(ip, offset, acce_size, NULL));
	return s;
}

void STM8S :: loadLocalAddr(Item *ip, char *reg)
{
	int offset = localOffset(ip);
	ASM_CODE2(_LDW, reg, REG_SP);

	if ( offset == 1 )
		ASM_CODE1(_INCW, reg);
	else
		ASM_CODE2(_ADDW, reg, offset, '#');
}

bool STM8S :: usePtr(Item *ip)
{
	int flags;
	if ( isIndirect(ip, &flags) )
	{
		if ( flags == 0 )
			return (ip->bias || ip->acceSize() > 1);
		
		return true;
	}

	return (ip->type == INDIR_ITEM || ip->type == PID_ITEM);
}

bool STM8S :: usePtr(Item *ip0, Item *ip1)
{
	return usePtr(ip0) && usePtr(ip1);
}

bool STM8S :: usePtr(Item *ip0, Item *ip1, Item *ip2)
{
	return usePtr(ip0, ip1) && usePtr(ip2);
}

bool STM8S :: farAcce(Item *ip)
{
	switch ( ip->type )
	{
		case PID_ITEM:
		case INDIR_ITEM:
			return (EXT_MODE && ip->attr->ptrVect[1] == CONST);

		case ID_ITEM:
			return (EXT_MODE && !isLocal(ip) && ip->attr->dataBank == CONST);

		case DIR_ITEM:
			return (ip->val.i & 0xffff0000)? true: false;

		default:
			return false;
	}
}

bool STM8S :: isBank0(Item *ip)
{
	switch ( ip->type )
	{
		case ACC_ITEM:
			return true;

		case ID_ITEM:
			return (!isLocal(ip) && ip->attr->dataBank == BANK0);

		case DIR_ITEM:
			return (ip->val.i & 0xffffff00)? false: true;

		default:
			return false;
	}
}

bool STM8S :: isLocal(Item *ip)
{
	if ( ip->type == ID_ITEM && isdigit(ip->val.s[0]) )
		return true;

	return (localOffset(ip) > 0);
}

bool STM8S :: isDirect(Item *ip)
{
	switch ( ip->type )
	{
		case ACC_ITEM:	return true;
		case DIR_ITEM:	return (ip->val.i & ~0xffff)? false: true;

		case ID_ITEM:	if ( isLocal(ip) ) return false;
						return !(ip->attr->dataBank == CONST && EXT_MODE);

		default:		return false;
	}
}

bool STM8S :: isGeneric(Item *ip)
{
	return ip->type == CON_ITEM || ip->type == ACC_ITEM ||
		   (ip->type == ID_ITEM && !farAcce(ip)) ||
		   isDirect(ip) || ip->type == TEMP_ITEM ||
		   (ip->type == IMMD_ITEM && !isLocal(ip));
}

bool STM8S :: isIndirect(Item *ip, int *flags)
{
	if ( ip->type == PID_ITEM && !isLocal(ip) )
	{
		*flags = 0;
		if ( !EXT_MODE ) return true;
		if ( ip->attr->dataBank == CONST )   *flags |= 1;
		if ( ip->attr->ptrVect[1] == CONST ) *flags |= 2;
		return true;
	}
	return false;
}

char *STM8S :: setPtr2(Item *ip, int index)
{
	int flags;
	if ( isIndirect(ip, &flags) && flags == 0 && ip->bias == 0 )
		return NULL;

	return setPtr(ip, index);
}

char *STM8S :: setPtr(Item *ip, int index)
{
	char *reg  = (index == 0)? REG_X: REG_Y;
	char *regl = (index == 0)? REG_XL:REG_YL;
	char *regh = (index == 0)? REG_XH:REG_YH;
	int  flags;
	switch ( ip->type )
	{
		case PID_ITEM:
			if ( isIndirect(ip, &flags) )	// external PID_ITEM
			{
				char *s = STRBUF();
				switch ( flags )
				{
					case 0:	// pointer in RAM/access RAM or !FAR_MODE 
						if ( ip->bias || ip->acceSize() > 1 )
						{
							ASM_CODE2(_LDW, reg, ip->val.s);
							return reg;
						}
						return NULL;
					case 1:	// pointer in ROM/access RAM
						ASM_CODE2(_LDF, REG_A, ip->val.s);	ASM_CODE2(_LD, regh, REG_A);
						sprintf(s, "%s+1", ip->val.s);
						ASM_CODE2(_LDF, REG_A, s);			ASM_CODE2(_LD, regl, REG_A);
						return reg;
					case 2:	// pointer in RAM/access ROM
						ASM_CODE1(_CLRW, reg);
						return reg;
					case 3:	// pointer in ROM/access ROM
						ASM_CODE2(_LDF, REG_A, ip->val.s);	ASM_CODE2(_LD, 4, REG_A);
						sprintf(s, "%s+1", ip->val.s);		
						ASM_CODE2(_LDF, REG_A, s);			ASM_CODE2(_LD, 5, REG_A);
						sprintf(s, "%s+2", ip->val.s);		
						ASM_CODE2(_LDF, REG_A, s);			ASM_CODE2(_LD, 6, REG_A);
						ASM_CODE1(_CLRW, reg);
						return reg;
				}
			}
		case INDIR_ITEM:
			if ( ip->attr->ptrVect[1] == CONST && EXT_MODE )
			{
				ASM_CODE2(_LD,  REG_A, itemStr(ip, 2, 1));	ASM_CODE2(_LD,  4, REG_A);
				ASM_CODE2(_LDW, reg,   itemStr(ip, 0, 2));	ASM_CODE2(_LDW, 5, reg);
				ASM_CODE1(_CLRW, reg);
			}
			else
				ASM_CODE2(_LDW, reg, itemStr(ip, 0, 2));
			break;

		default:
			reg = NULL;
	}

	if ( reg )
	{
		STM8S_reg *indx = (strcmp(reg, REG_X) == 0)? Reg_X: Reg_Y;
		indx->bias = 0;
	}
	return reg;
}

void STM8S :: setPtr(int addr, Item *ip)
{
	int offset = ip->bias, flag;
	switch ( ip->type )
	{
		case PID_ITEM:
			if ( farAcce(ip) && !isLocal(ip) && ip->attr->dataBank == CONST )
			{
				for (int i = 0; i < 3; i++)
				{
					char *inst = (i == 0)? _ADD: _ADC;
					ASM_CODE2(_LDF, REG_A, STRBUF(ip->val.s, 2-i));
					if ( offset ) ASM_CODE2(inst, REG_A, offset>>(i*8), '#');
					ASM_CODE2(_LD, addr+2-i, REG_A);
				}
				return;
			}
			if ( farAcce(ip) && !isLocal(ip) )
			{
				ASM_CODE2(_LDW, REG_X, STRBUF(ip->val.s, 1));
				if ( offset ) ASM_CODE2(_ADDW, REG_X, offset, '#');
				ASM_CODE2(_LDW, addr+1, REG_X);
				ASM_CODE2(_LD, REG_A, STRBUF(ip->val.s, 0));
				if ( offset ) ASM_CODE2(_ADC, REG_A, offset>>16, '#');
				ASM_CODE2(_LD, addr, REG_A);
				return;
			}
			if ( !isLocal(ip) && ip->attr->dataBank == CONST && EXT_MODE )
			{
				ASM_CODE2(_LDF, REG_A, STRBUF(ip->val.s, 1));
				if ( offset ) ASM_CODE2(_ADD, REG_A, offset, '#');
				ASM_CODE2(_LD, addr+1, REG_A);
				ASM_CODE2(_LDF, REG_A, STRBUF(ip->val.s, 0));
				if ( offset ) ASM_CODE2(_ADC, REG_A, offset/256, '#');
				ASM_CODE2(_LD, addr, REG_A);
				return;
			}
			if ( isIndirect(ip, &flag) && flag == 0 )
			{
				ASM_CODE2(_LDW, REG_X, ip->val.s);
				if ( offset ) ASM_CODE2(_ADDW, REG_X, offset, '#');
				ASM_CODE2(_LDW, addr, REG_X);
				return;
			}
		case INDIR_ITEM:
			if ( ip->attr->ptrVect[1] != CONST || !EXT_MODE )
			{
				ASM_CODE2(_LDW, REG_X, itemStr(ip, 0, 2, NULL));
				if ( offset ) ASM_CODE2(_ADDW, REG_X, offset, '#');
				ASM_CODE2(_LDW, addr, REG_X);
			}
			else// if ( isLocal(ip) )
			{
				ASM_CODE2(_LDW, REG_X, itemStr(ip, 0, 2, NULL));
				if ( offset ) ASM_CODE2(_ADDW, REG_X, offset, '#');
				ASM_CODE2(_LDW, addr+1, REG_X);
				ASM_CODE2(_LD,  REG_A, itemStr(ip, 2, 1, NULL));
				if ( offset ) ASM_CODE2(_ADC, REG_A, offset >> 8, '#');
				ASM_CODE2(_LD,  addr, REG_A);
			}
			break;
		case ID_ITEM:
			if ( !isLocal(ip) )
			{
				if ( ip->attr->dataBank == CONST && EXT_MODE )
				{
					char *s = STRBUF();
					sprintf(s, "(%s)>>8", ip->val.s);
					ASM_CODE2(_LDW, REG_X, s, '#');
					ASM_CODE2(_LDW, addr, REG_X);
					ASM_CODE2(_MOV, addr+2, ip->val.s, '#');
				}
				else
				{
					ASM_CODE2(_LDW, REG_X, ip->val.s, '#');
					ASM_CODE2(_LDW, addr, REG_X);
				}
				return;
			}
		case TEMP_ITEM:
			offset = localOffset(ip);

			ASM_CODE2(_LDW, REG_X, REG_SP);
			if ( offset == 1 )
				ASM_CODE1(_INCW, REG_X);
			else
				ASM_CODE2(_ADDW, REG_X, offset, '#');
			ASM_CODE2(_LDW, addr, REG_X);
			break;

		case DIR_ITEM:
			if ( ip->val.i > 0xffff )
			{
				ASM_CODE2(_LDW, REG_X, ip->val.i >> 8, '#');
				ASM_CODE2(_LDW, addr, REG_X);
				ASM_CODE2(_LD, REG_A, ip->val.i & 0xff, '#');
				ASM_CODE2(_LD, addr+2, REG_A);
			}
			else
			{
				ASM_CODE2(_LDW, REG_X, ip->val.i, '#');
				ASM_CODE2(_LDW, addr, REG_X);
			}
			break;

		default:
			break;
	}
}

void STM8S :: setPtr(Item *ip, char *reg)
{
	switch ( ip->type )
	{
		case ACC_ITEM:
			ASM_CODE1(_CLRW, reg);
			break;

		case INDIR_ITEM:
		case PID_ITEM:
			setPtr(ip, (strcmp(reg, REG_X) == 0)? 0: 1);
			break;

		case ID_ITEM:
			if ( !isLocal(ip) ) {
				ASM_CODE2(_LDW, reg, ip->val.s, '#');
				break;
			}
		case TEMP_ITEM:
			loadLocalAddr(ip, reg);
			break;

		case DIR_ITEM:
			ASM_CODE2(_LD, reg, ip->val.i, '#');
			break;

		default:	// should never happen.
			break;
	}
}

Item *STM8S :: pushStack(Item *ip, int req_size)
{
	int  size = ip->acceSize();
	bool sign = ip->acceSign();
	char *reg = setPtr(ip, 0);
	char *r16 = reg? REG_Y: REG_X;
	int  i = 0;

	if ( LOCAL_ADDR(ip) )
	{
		ASM_CODE2(_LDW, REG_X, REG_SP);
		ASM_CODE2(_ADDW, REG_X, localOffset(ip), '#');
		if ( req_size >= 2 ) {
			ASM_CODE1(_PUSHW, REG_X);
			frameOffset += 2;
		}
		else {
			ASM_CODE2(_LD, REG_A, REG_XL);
			ASM_CODE1(_PUSH, REG_A);
			frameOffset++;
		}
		i = (req_size >= 2)? 2: 1;
	}
	for (; i < req_size; i++)
	{
		if ( ip->type == CON_ITEM ) {
			int n = (ip->val.i >> (i*8));
			if ( (req_size-i) >= 2 && !(n & 0xffff) ) {
				ASM_CODE1(_CLRW,  REG_X);
				ASM_CODE1(_PUSHW, REG_X);
				i++; frameOffset++;
			}
			else
				ASM_CODE1(_PUSH, n & 255, '#');
			frameOffset++;
			continue;
		}
		if ( i >= size )
		{
			if ( sign ) {
				if ( i == size )
					call((char*)"_extendSign");

				ASM_CODE1(_PUSH, REG_A);
			}
			else
				ASM_CODE1(_PUSH, "#0");
			frameOffset++;
			continue;
		}
		if ( farAcce(ip) )
		{
			ASM_CODE2(_LDF, REG_A, acceItem(ip, i, 1, reg));
			ASM_CODE1(_PUSH, REG_A);
			frameOffset++;
		}
		else if ( (size-i) > 1 && (req_size-i) > 1 )
		{
			if ( reg && strcmp(reg, r16) && (size-i) > 2 ) {
				ASM_CODE2(_LDW, r16, reg);
				ASM_CODE2(_LDW, r16, acceItem(ip, i++, 2, r16));
			}
			else {
				if ( reg )	r16 = reg;
				ASM_CODE2(_LDW, r16, acceItem(ip, i++, 2, reg));
			}
			ASM_CODE1(_PUSHW, r16);
			frameOffset += 2;
		}
		else if ( isDirect(ip) )
		{
			ASM_CODE1(_PUSH, acceItem(ip, i, 1, NULL));
			frameOffset++;
		}
		else
		{
			ASM_CODE2(_LD, REG_A, acceItem(ip, i, 1, reg));
			ASM_CODE1(_PUSH, REG_A);
			frameOffset++;
		}
	}
	Item *iptr = idItem((char*)"1");
	iptr->attr = newAttr((req_size == 1)? CHAR:
						 (req_size == 2)? INT:
						 (req_size == 3)? SHORT: LONG);
	iptr->attr->isUnsigned = ip->attr? ip->attr->isUnsigned: 1;
	return iptr;
}

void STM8S :: popStack(int size)
{
	switch ( size )
	{
		case 0: return;
		case 1:	ASM_CODE1(_POP, REG_A);		break;
		case 2:	ASM_CODE1(_POPW, REG_X);	break;
		default:ASM_CODE2(_ADD, REG_SP, size, '#');
	}
	frameOffset -= size;
}

Item *STM8S :: storeToACC(Item *ip, int size)
{
	Item *acc = accItem(size, ip->acceSign());
	mov(acc, ip);
	return acc;
}
