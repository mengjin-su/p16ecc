#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "asm.h"
}
#include "symbol.h"
#include "p16asm.h"
#include "p16inst.h"

#define isABS(lp)	(lp->attr && lp->attr->isABS)
#define isREL(lp)	(lp->attr && lp->attr->isREL)
#define isBEG(lp)	(lp->attr && lp->attr->isBEG)
#define isEND(lp)	(lp->attr && lp->attr->isEND)

void P16E_asm :: output(line_t *lp)
{
	unsigned int addr = 0;
	for (bool done = false; !done && lp; lp = lp->next)
	{
		if ( lp->label && lp->inst != EQU )
		{
			if ( lp->globalLbl )
				objWriter->output('G', lp->label);
			else
				objWriter->output('L', lp->label);

			objWriter->flush();
		}

		item_t *ip0 = lp->oprds;			// fisrt operand
		item_t *ip1 = ip0? ip0->next: NULL;	// second operand
		P16inst_t *p16inst;
		char buf[4096];
		int  v, mask;
		char type = 0;
		int  incAddr = 0;

		switch ( lp->inst )
		{
			case S_COMMENT:	// comments (C source)
			case DEVICE:	// device name, RAM & ROM size
			case INVOKE:	// invoke library file
			case CBLANK:	// code space blank
			case DBLANK:	// data space blank
            case FCALL:		// function call index
			case BSEL:		// .bsel
			case PSEL:		// .psel
			case RS:		// .rs
				if ( ip0 )
				{
					char t = (lp->inst == S_COMMENT)? 	';':
							 (lp->inst == DEVICE)? 		'P':
							 (lp->inst == INVOKE)? 		'I':
							 (lp->inst == CBLANK)?		'N':
							 (lp->inst == DBLANK)?		'M':
							 (lp->inst == FCALL)?		'F':
							 (lp->inst == BSEL)?		'K':
							 (lp->inst == PSEL)?		'J': 'R';
					objOutput(t, ip0);

					for (item_t *ip = ip1; ip; ip = ip->next)
						objOutput(ip);
					objWriter->flush();
				}
				if ( (lp->inst == BSEL || lp->inst == PSEL) && !ip1 )
				{
					int code = (lp->inst == PSEL)? 0x3180:
							   (useBsr6 == 0)? 	   0x0020: 0x0140;
					lstWriter->output(addr, code, '?');
					incAddr = 1, type = '?';
				}
				lstWriter->output(lp);
				break;

			case SEGMENT:
				addr = 0;
				objWriter->output('S', lp->attr->name);
				if ( isREL(lp) )
					objWriter->output("REL");
				else if ( isABS(lp) )
				{
					objWriter->output("ABS");
					if ( lp->attr->addr )
					{
						objOutput(lp->attr->addr);
						addr = lp->attr->addr->data.val;
					}
				}

				if ( isBEG(lp) )
					objWriter->output("BEG");
				else if ( isEND(lp) )
					objWriter->output("END");

				if ( I_TYPE(ip0, ':') )
				{
					objOutput('U', ip0->left);	// function name
					objOutput(ip0->right);		// function RAM cost
				}
				objWriter->flush();
				lstWriter->output(lp);
				break;

			case EQU:
				if ( lp->label && lp->globalLbl && ip0 )
				{
					objWriter->output('E', lp->label);
					objOutput(ip0);
					objWriter->flush();
				}
				lstWriter->output(lp);
				break;

			case DW:
				for(item_t *ip = lp->oprds; ip; ip = ip->next)
				{
					if ( parseItem(ip, buf, &v, DW) == VAL_EXP )
						objWriter->outputW(v & 0x3fff);
					else
						objWriter->outputW(buf, 0x3fff);

					lstWriter->output(addr++, v, type);
				}
				lstWriter->output(lp);
				break;

			case END:
				lstWriter->output(lp);
				done = true;
				break;

			default:	// all regular instructions ...
				p16inst = P16inst(lp->inst);
				if ( p16inst )
				{
					int code = p16inst->code;
					incAddr = 1;
					switch ( p16inst->form )
					{
						case P16_FD_I:		// ADDWF x, W
							if ( lp->desRegF ) code |= 0x0080;
						case P16_F_I:		// MOVWF x
							switch ( parseItem(ip0, buf, &v, lp->inst) )
							{
								case VAL_EXP:
									objWriter->outputW(code, &v, 0x7f);
									code |= v & 0x7f;
									break;

								case STR_EXP:
									objWriter->outputW(code, buf, 0x7f);
									type = '?';
									break;

								default:
									break;
							}
							break;

						case P16_LIT5_I:	// MOVLB
						case P16_LIT6_I:	// MOVLB
						case P16_LIT8_I:	// MOVLW
						case P16_LIT9_I:	// BRA
						case P16_LIT11_I:	// GOTO, CALL
							mask = (p16inst->form == P16_LIT5_I)? 0x01f:
								   (p16inst->form == P16_LIT6_I)? 0x03f:
								   (p16inst->form == P16_LIT8_I)? 0x0ff:
								   (p16inst->form == P16_LIT9_I)? 0x1ff: 0x7ff;

							switch ( parseItem(ip0, buf, &v, lp->inst) )
							{
								case VAL_EXP:
									code |= v & mask;
									objWriter->outputW(code);
									break;

								case STR_EXP:
									type = '?';
									objWriter->outputW(code, buf, mask);
									break;

								default:
									break;
							}
							break;

						case P16_BIT_I:		// BCF x, 1
							if ( parseItem(ip1, buf, &v, lp->inst) == VAL_EXP )
								code |= (v & 7) << 7;

							switch ( parseItem(ip0, buf, &v, lp->inst) )
							{
								case VAL_EXP:
									objWriter->outputW(code, &v, 0x7f);
									code |= v & 0x7f;
									break;

								case STR_EXP:
									objWriter->outputW(code, buf, 0x7f);
									type = '?';
									break;

								default:
									break;
							}
							break;

						case P16_FSR_OP_I:	// ADDFSR INDF0, 1
							if ( parseItem(ip0, buf, &v, lp->inst) == VAL_EXP )
								code |= (v&1) << 6;
							if ( parseItem(ip1, buf, &v, lp->inst) == VAL_EXP )
								code |= v & 0x3f;
							objWriter->outputW(code);
							break;

						case P16_MOV_OP_I:	// movwi ++FSR0
							switch ( ip0->type )
							{
								case FSR_PRE_INC:	// MOVIW/MOVWI	++INDF0
								case FSR_PRE_DEC:	// MOVIW/MOVWI	--INDF0
								case FSR_POST_INC:	// MOVIW/MOVWI	INDF0++
								case FSR_POST_DEC:	// MOVIW/MOVWI	INDF0--
									if ( parseItem(ip0->left, buf, &v, lp->inst) == VAL_EXP )
									{
										code |= (v&1) << 2;
										code |= (ip0->type == FSR_PRE_INC )? 0:
												(ip0->type == FSR_PRE_DEC )? 1:
												(ip0->type == FSR_POST_INC)? 2: 3;
										objWriter->outputW(code);
									}
									break;
							}
							break;

						case P16_MOV_OFF_I:	// movwi n[FSR0]
							if ( ip0->type == FSR_OFFSET )
							{
								if ( parseItem(ip0->left, buf, &v, lp->inst) == VAL_EXP )
									code |= v & 0x3f;	// offset

								if ( parseItem(ip0->right, buf, &v, lp->inst) == VAL_EXP )
									code |= (v&1) << 6;// FSR index

								objWriter->outputW(code);
							}
							break;

						default:	// NOP
							objWriter->outputW(code);
							break;
					}
					lstWriter->output(addr, code, type);
					lstWriter->output(lp);
				}
				else if ( lp->inst )
					printf("unknown instruction - %d\n", lp->inst);
				else
					lstWriter->output(lp);
				break;
		}
		addr += incAddr;
	}

	objWriter->flush();
	objWriter->close();
	lstWriter->close();
}

void P16E_asm :: objOutput(char t, item_t *ip)
{
	int val;
	char buf[4096];
	switch ( parseItem(ip, buf, &val, 0) )
	{
		case VAL_EXP:	objWriter->output(t, val);	break;
		case STR_EXP:	objWriter->output(t, buf);	break;
		default:		break;
	}
}

void P16E_asm :: objOutput(item_t *ip)
{
	int val;
	char buf[4096];
	switch ( parseItem(ip, buf, &val, 0) )
	{
		case VAL_EXP:	objWriter->output(val);	break;
		case STR_EXP:	objWriter->output(buf);	break;
		default:		break;
	}
}