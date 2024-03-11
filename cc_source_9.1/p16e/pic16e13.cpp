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

void PIC16E :: outputInit(Pnode *plist)
{
	if ( plist == NULL ) return;

	ASM_OUTP("\n");
	ASM_CODE(_SEGMENT, "CODEi (REL)");
	regWREG->reset();
	regBSR->reset();
	for(; plist; plist = plist->next)
	{
		Item *ip0 = plist->items[0];
		Item *ip1 = plist->items[1];
		Item *ip2 = plist->items[2];
		int size;
		Item *lbl;
		
		switch ( plist->type )
		{
			case P_COPY:
				if ( !(ip0 && ip1 && ip2 && ip2->val.i > 0) ) break;
				ASM_CODE(_MOVLW, acceItem(ip1, 0));	ASM_CODE(_MOVWF, FSR1L);
				ASM_CODE(_MOVLW, acceItem(ip1, 1));	ASM_CODE(_MOVWF, FSR1H);
				ASM_CODE(_MOVLW, acceItem(ip0, 0));	ASM_CODE(_MOVWF, FSR0L);
				ASM_CODE(_MOVLW, acceItem(ip0, 1));	ASM_CODE(_MOVWF, FSR0H);
				
				size = -ip2->val.i;
				ASM_CODE(_MOVLW, size & 0xff);
				ASM_CODE(_MOVWF, ACC0);
				if ( ip2->val.i >= 256 )
				{
					ASM_CODE(_MOVLW, (size >> 8) & 0xff);
					ASM_CODE(_MOVWF, ACC1);						
				}
				lbl = lblItem(pcoder->getLbl());
				ASM_LABL(lbl->val.s);	
				ASM_CODE(_MOVIW, "INDF1++");
				ASM_CODE(_MOVWI, "INDF0++");
				
				ASM_CODE(_INCFSZ, ACC0, _F_);
				if ( ip2->val.i >= 256 )
				{
					ASM_CODE(_BRA, lbl->val.s);
					ASM_CODE(_INCFSZ, ACC1, _F_);
				}
				ASM_CODE(_BRA, lbl->val.s);
				delete lbl;
				break;

			case '=':
				mov(ip0, ip1);
				break;
		}
	}
}

void PIC16E :: outputConst(Pnode *pcode)
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
				ASM_OUTP("\n");
				if ( ip1 && ip1->val.i > 0 )
					sprintf(buf, "CONST0 (ABS =0x%04X)", ip1->val.i);
				else
					sprintf(buf, "CONSTi (REL)");

				if ( is_public && ip0->attr->dimVect )
					sprintf(&buf[strlen(buf)], " %s", ip0->val.s);
				
				ASM_CODE(_SEGMENT, buf);
				ASM_LABL(ip0->val.s, is_public);
				break;

			case P_FILL:
				for (int i = 0; i < ip1->val.i; i++)
					ASM_CODE(_RETLW, acceItem(ip0, i));
				break;
		}
	}
}

void PIC16E :: outputString(Const_t *list)
{
	for (; list; list = list->next)
	{
		ASM_OUTP("\n");
		ASM_CODE(_SEGMENT, "CONSTi (REL)");
		ASM_LABL(list->strName()); 
		for(int i = 0; i < (int)(strlen(list->str)+1); i++)
			ASM_CODE(_RETLW, list->str[i]);
	}
}
