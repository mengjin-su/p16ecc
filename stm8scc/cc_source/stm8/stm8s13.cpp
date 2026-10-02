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
#include "../option.h"
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../const.h"
#include "../nlist.h"
#include "stm8s.h"
#include "stm8s_reg.h"
#include "stm8s_inst.h"
#include "stm8s_asm.h"

void STM8S :: outputInit(Pnode *plist)
{
	if ( plist == NULL ) return;

	char *buf = STRBUF();	// string buffer
	ASM_OUTP('\n');
	ASM_CODE1(_SEGMENT, "CODEi (REL)");
	for(; plist; plist = plist->next)
	{
		Item *ip0 = plist->items[0];
		Item *ip1 = plist->items[1];
		Item *ip2 = plist->items[2];
		int   lbl;
		char *ld1 = EXT_MODE? _LDF: _LD;

		switch ( plist->type )
		{
			case P_COPY:
				if ( !(ip0 && ip1 && ip2) )	break;	// should never happen...

				if ( ip2->val.i <= 4 || (!EXT_MODE && ip2->val.i <= 8) ) {
					for (int i = 0; i < ip2->val.i; i++) {
						char *src = STRBUF(); sprintf(src, "%s+%d", ip1->val.s, i);
						char *dst = STRBUF(); sprintf(dst, "%s+%d", ip0->val.s, i);

						if ( EXT_MODE || (i+1) >= ip2->val.i ) {
							ASM_CODE2(ld1, REG_A, src);
							ASM_CODE2(_LD, dst, REG_A);
						}
						else {
							ASM_CODE2(_LDW, REG_X, src);
							ASM_CODE2(_LDW, dst, REG_X);
							i++;
						}
					}
					break;
				}

				if ( !EXT_MODE ) {
					ASM_CODE2(_LDW, REG_X, ip1->val.s, '#');
					ASM_CODE2(_LDW, 2, REG_X);
				}
				else {
					ASM_CODE2(_LDW, REG_X, ip1->val.s, '#');
					ASM_CODE2(_LDW, 3, REG_X);
					sprintf(buf, "#(%s)>>16", ip1->val.s);
					ASM_CODE2(_MOV, 2, buf);
				}

				ASM_CODE2(_LDW, REG_X, ip0->val.s, '#');
				ASM_CODE2(_LDW, 0, REG_X);

				ASM_CODE1(_CLRW, REG_X);
				ASM_CODE2(_LDW, REG_Y, ip2->val.i, '#');

				lbl = pcoder->getLbl();
				ASM_LABL(lbl);
				ASM_CODE2(ld1, REG_A, (char*)"([2], %X)");
				ASM_CODE2(_LD, (char*)"([0], %X)", REG_A);
				ASM_CODE1(_INCW, REG_X);
				ASM_CODE1(_DECW, REG_Y);
				ASM_CODE_LBL(_JRNE, lbl);
				ASM_OUTP('\n');
				break;

			case '=':
				mov(ip0, ip1);
				ASM_OUTP('\n');
				break;
		}
	}
}

void STM8S :: outputConst(Pnode *pcode)
{
	for (; pcode; pcode = pcode->next)
	{
		Item *ip0 = pcode->items[0];
		Item *ip1 = pcode->items[1];
		char *buf = STRBUF();
		bool is_public;
		switch ( pcode->type )
		{
			case P_SEGMENT:
				is_public = !(strchr(ip0->val.s, '$') || ip0->attr->isStatic);
				ASM_OUTP('\n');
				if ( ip1 && ip1->val.i > 0 )
					sprintf(buf, "CONST0 (ABS =0x%04X)", ip1->val.i);
				else
					sprintf(buf, "CONSTi (REL)");

				if ( is_public && ip0->attr->dimVect )
					sprintf(&buf[strlen(buf)], " %s", ip0->val.s);
				ASM_CODE1(_SEGMENT, buf);
				sprintf(buf, "%s:", ip0->val.s);
				if ( is_public ) strcat(buf, ":");
				ASM_OUTP_LN(buf);
				break;

			case P_FILL:
				switch ( ip1->val.i ) {
					case 1:	 ASM_I_CODE(_DB); 	break;
					case 2:	 ASM_I_CODE(_DW); 	break;
					case 3:	 ASM_I_CODE(_DT); 	break;
					case 4:	 ASM_I_CODE(_DD); 	break;
					default: return;
				}
				if ( ip0->type == CON_ITEM )
					ASM_OUTP(ip0->val.i);
				else
					ASM_OUTP(ip0->val.s);
				ASM_OUTP_LN();
				break;
		}
	}
}

void STM8S :: outputString(void)
{
	Const_t *p = pcoder->constGroup->list;
	if ( !p ) return;

	ASM_OUTP('\n');
	for(; p; p = p->next)
	{
		ASM_CODE1(_SEGMENT, "CONSTi (REL)");
		ASM_LABL_LN(p->strName(), true);
		for (int i = 0; i < (int)(strlen(p->str)+1); i++)
		{
			char *s = STRBUF();
			if ( (i & 3) == 0 )
				sprintf(s, "\t%s\t%d", _DB, p->str[i] & 0xff);
			else
				sprintf(s, ", %d", p->str[i] & 0xff);

			ASM_OUTP(s);
			if ( (i & 3) == 3 ) st8asm->output("\n");
		}
		ASM_OUTP('\n');
	}
}

void STM8S :: mul(Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int size2 = ip2->acceSize();
	if ( same(ip0, ip1) )
	{
		mulAssign(ip0, ip2);
		return;
	}
	if ( same(ip0, ip2) )
	{
		mulAssign(ip0, ip1);
		return;
	}
	if ( ip1->type == CON_ITEM ) {
		mul(ip0, ip2, ip1);
		return;
	}

	if ( size0 == 1 ||
		 (size0 == 2 &&
		  size1 == 1 && !ip1->acceSign() &&
		  size2 == 1 && !ip2->acceSign()  ) )
	{
		mul8(ip0, ip1, ip2);
		return;
	}

	mov(ip0, ip1);
	mulAssign(ip0, ip2);
}

void STM8S :: mul8(Item *ip0, Item *ip1, Item *ip2)
{
	char *reg1 = setPtr(ip1, 0);
	char *reg2 = setPtr(ip2, reg1? 1: 0);
	char *r16  = REG_X, *ld;

	ld = farAcce(ip1)? _LDF: _LD;
	ASM_CODE2(ld, REG_A, acceItem(ip1, 0, 1, reg1));
	if ( reg1 || !reg2 )
	{
		ASM_CODE2(_LD, REG_XL, REG_A);
	}
	else
	{
		ASM_CODE2(_LD, REG_YL, REG_A);
		r16 = REG_Y;
	}

	ld = farAcce(ip2)? _LDF: _LD;
	ASM_CODE2(ld, REG_A, acceItem(ip2, 0, 1, reg2));
	ASM_CODE2(_MUL, r16, REG_A);

	if ( ip0->acceSize() == 1 )
	{
		ASM_CODE2(_LD, REG_A, strcmp(r16, REG_X)? REG_YL: REG_XL);
		char *reg0 = setPtr(ip0, 0);
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
	}
	else
	{
		char *reg0 = strcmp(r16, REG_X)? setPtr(ip0, 0): setPtr(ip0, 1);
		ASM_CODE2(_LDW, acceItem(ip0, 0, 2, reg0), r16);
	}
}

void STM8S :: casefetch(Item *ip0)
{
	int size0 = ip0->acceSize();
	char *reg0 = setPtr(ip0, 0);
	if ( LOCAL_ADDR(ip0) )
		loadLocalAddr(ip0, REG_X);
	else if ( farAcce(ip0) )
	{
		for (int i = 0; i < size0; i++) {
			ASM_CODE2(_LDF, REG_A, acceItem(ip0, i, 1, reg0));
			if ( size0 > 1 ) {
				ASM_CODE2(_RRWA, REG_Y, REG_A);
				if ( size0 == 4 && i >= 2 )
					ASM_CODE1(_PUSH, REG_A), frameOffset++;
			}
		}
		if ( size0 == 4 )
			ASM_CODE1(_POPW, REG_X), frameOffset -= 2;
		else if ( size0 >= 2 )
			ASM_CODE2(_LDW, REG_X, REG_Y);
	}
	else
		switch ( size0 )
		{
			case 1:
				ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
				break;

			case 2:
				ASM_CODE2(_LDW, REG_X, acceItem(ip0, 0, 2, reg0));
				break;

			case 3:
				ASM_CODE2(_LD,  REG_A, acceItem(ip0, 0, 1, reg0));
				ASM_CODE2(_LDW, REG_X, acceItem(ip0, 1, 2, reg0));
				break;

			case 4:
				if ( reg0 ) {
					ASM_CODE2(_LDW, REG_Y, reg0);
					ASM_CODE2(_LDW, REG_X, acceItem(ip0, 0, 2, reg0));
					ASM_CODE2(_LDW, REG_Y, acceItem(ip0, 2, 2, REG_Y));
				}
				else {
					ASM_CODE2(_LDW, REG_X, acceItem(ip0, 0, 2));
					ASM_CODE2(_LDW, REG_Y, acceItem(ip0, 2, 2));
				}
		}
}

void STM8S :: casejump(Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->val.i;
	unsigned int n = ip1->val.i;
	switch ( size0 ) {
		case 1:	if ( (n & 0x000000ff) ) ASM_CODE2(_CP,  REG_A, n & 0xff, '#');
				else					ASM_CODE1(_TNZ, REG_A);
				break;
		case 2:	if ( (n & 0x0000ffff) ) ASM_CODE2(_CPW, REG_X, n & 0xffff, '#');
				else					ASM_CODE1(_TNZW, REG_X);
				break;
		case 3:	if ( (n & 0x0000ffff) ) ASM_CODE2(_CPW, REG_X, n & 0xffff, '#');
				else					ASM_CODE1(_TNZW, REG_X);
				ASM_CODE1(_JRNE, (n & 0x00ff0000)? ".+2": ".+1");
				if ( (n & 0x00ff0000) ) ASM_CODE2(_CP, REG_A, (n>>16) & 0xff, '#');
				else				  	ASM_CODE1(_TNZ, REG_A);
				break;
		case 4:	if ( (n & 0x0000ffff) ) ASM_CODE2(_CPW, REG_X, n & 0xffff, '#');
				else 					ASM_CODE1(_TNZW, REG_X);
				ASM_CODE1(_JRNE, (n & 0xffff0000)? ".+4": ".+2");
				if ( (n & 0xffff0000) ) ASM_CODE2(_CPW, REG_Y, (n>>16) & 0xffff, '#');
				else					ASM_CODE1(_TNZW, REG_Y);
				break;
	}
	ASM_CODE2(__JP, J_CODE_EQ, ip2->val.s);
}