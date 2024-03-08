#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "asm.h"
}
#include "symbol.h"
#include "p16asm.h"
#include "fwriter.h"

typedef struct {
	char *regName;
	int	  regAddr;
} CoreReg_t;

const CoreReg_t pic16_reg[] = {
    {(char*)"INDF0", 	0x00}, {(char*)"INDF1",		0x01},
    {(char*)"PCL", 	  	0x02}, {(char*)"STATUS",	0x03},
    {(char*)"FSR0L", 	0x04}, {(char*)"FSR0H",		0x05},
    {(char*)"FSR1L", 	0x06}, {(char*)"FSR1H",		0x07},
    {(char*)"BSR",	  	0x08}, {(char*)"WREG",		0x09},
	{(char*)"PCLATH",	0x0a}, {(char*)"INTCON",	0x0b},
	{(char*)"FSR0",		0x00}, {(char*)"FSR1", 		0x01},
    {NULL, 0}
};

P16E_asm :: P16E_asm(char *fname, const char *ext)
{
	memset(this, 0, sizeof(P16E_asm));

	int len = strlen(fname);
	if ( len > 4 && strcasecmp(&fname[len-4], ".asm") == 0 )
	{
		len -= 4;
		lstFile = new char[len+5];
		memcpy(lstFile, fname, len); strcpy(&lstFile[len], ".lst");
		objFile = new char[len+5];
		memcpy(objFile, fname, len); strcpy(&objFile[len], ext);

		FILE *objFout = fopen(objFile, "w");
		FILE *lstFout = fopen(lstFile, "w");
		if ( objFout == NULL || lstFout == NULL )
		{
			errorCount++;
			printf("open file error!\n");
		}
		else
		{
			objWriter = new ObjWriter(objFout);
			lstWriter = new LstWriter(lstFout);
		}
		
		// add core registers's definitions...
		for(const CoreReg_t *rp = pic16_reg; rp->regName; rp++)
		{
			Symbol *sp = addSymbol(&symbolList, new Symbol(rp->regName));
			sp->item   = valItem(rp->regAddr);
			sp->type   = EQU;
		}
	}
	else
	{
		errorCount++;
		printf("file name error!\n");
	}
}

P16E_asm :: ~P16E_asm()
{
	while ( symbolList ) {
		Symbol *sp = symbolList->next;
		delete symbolList;
		symbolList = sp;
	}
	delete objWriter;
	delete lstWriter;
	delete objFile;
	delete lstFile;
}

void P16E_asm :: run(line_t *lp, int pass)
{
	for (; lp; lp = lp->next)
	{
		if ( pass == ASM_PASS1 && lp->inst == EQU )
		{
			if ( lp->label && lp->oprds )
			{
				lp->oprds   = symbolReplace(symbolList, lp->oprds);
				Symbol *sym = addSymbol(&symbolList, new Symbol(lp->label));
				sym->type   = EQU;
				sym->item 	= cloneItem(lp->oprds);
				sym->global = lp->globalLbl? true: false;
			}
		}

		if ( pass == ASM_PASS2 && lp->inst != EQU )
		{
			regulateInst(lp);
			if ( lp->oprds )
			{
				lp->oprds = symbolReplace(symbolList, lp->oprds);
				for (item_t *ip = lp->oprds; ip->next; ip = ip->next)
					ip->next = symbolReplace(symbolList, ip->next);
			}
		}
	}
}

item_t *P16E_asm :: symbolReplace(Symbol *slist, item_t *ip)
{
	if ( ip == NULL || slist == NULL ) return ip;

	switch ( ip->type )
	{
		case TYPE_SYMBOL:
		{
			Symbol *sp = searchSymbol(slist, ip->data.str);
			if ( sp )
			{
				item_t *next = ip->next;
				freeItem(ip);
				ip = cloneItem(sp->item);
				ip->next = next;
			}
			break;
		}
		case TYPE_STRING:
		case TYPE_VALUE:
			break;

		default:
			ip->left  = symbolReplace(slist, ip->left);
			ip->right = symbolReplace(slist, ip->right);
			break;
	}
	return ip;
}

void P16E_asm :: regulateInst(line_t *lp)
{
	int item_cnt = itemCount(lp);
	bool operand_err = false;

	switch ( lp->inst )
	{
		case MOVIW:
		case MOVWI:
			switch ( lp->oprds->type )
			{
				case FSR_PRE_INC:	// MOVIW/MOVWI	++INDF0
				case FSR_PRE_DEC:	// MOVIW/MOVWI	--INDF0
				case FSR_POST_INC:	// MOVIW/MOVWI	INDF0++
				case FSR_POST_DEC:	// MOVIW/MOVWI	INDF0--
					lp->inst = (lp->inst == MOVIW)? MOVIW_OP: MOVWI_OP;
					break;

				case FSR_OFFSET:	// MOVIW/MOVWI	n[INDF0]
					lp->inst = (lp->inst == MOVIW)? MOVIW_OFF: MOVWI_OFF;
					break;

				default:
					operand_err = true;
					break;
			}
			break;

		case MOVF:		case ADDWF:		case SUBWF:
		case ANDWF:		case IORWF:		case XORWF:
		case ADDWFC:	case SUBWFB:	case SWAPF:
		case RLF:		case RRF:		case COMF:
		case INCF:		case DECF:		case INCFSZ:	case DECFSZ:
		case LSLF:		case LSRF:		case ASRF:
			operand_err = (item_cnt > 2);
			if ( item_cnt > 1 )
			{
				item_t *ip = lp->oprds->next;
				switch ( ip->type )
				{
					case TYPE_VALUE:
						lp->desRegF = ip->data.val;
						break;

					case TYPE_SYMBOL:
						if ( !strcasecmp(ip->data.str, "F") )
							lp->desRegF = 1;
						else if ( strcasecmp(ip->data.str, "W") )
							operand_err = true;
				}
				freeItem(ip);
				lp->oprds->next = NULL;
			}
			break;
	}
	if ( operand_err )
	{
		errorCount++;
		printf("#%d - operand(s) error!\n", lp->lineno);
	}
}

item_t *P16E_asm :: mergeItems(int op, item_t *ip1, item_t *ip2)
{
	if ( ip1 && ip1->type == TYPE_VALUE && ip2 && ip2->type == TYPE_VALUE )
	{
		int v1 = ip1->data.val;
		int v2 = ip2->data.val;
		switch ( op )
		{
			case '+':		return valItem(v1 + v2);
			case '-':		return valItem(v1 - v2);
			case '*':		return valItem(v1 * v2);
			case '/':		return valItem(v1 / v2);
			case '%':		return valItem(v1 % v2);
			case '&':		return valItem(v1 & v2);
			case '|':		return valItem(v1 | v2);
			case '^':		return valItem(v1 ^ v2);
			case LSHIFT:	return valItem(v1 << v2);
			case RSHIFT:	return valItem(v1 >> v2);
		}
	}

	if ( ip1 && ip1->type == TYPE_VALUE )
	{
		int v1 = ip1->data.val;
		switch ( op )
		{
			case UMINUS:	return valItem(-v1);
			case INVERSE:	return valItem(~v1);
		}
	}
	return NULL;
}