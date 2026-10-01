#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}
#include "symbol.h"
#include "seg.h"
#include "parse.h"
#include "st8asm.h"
#include "exp.h"
#include "st8inst.h"
#include "asmout.h"


enum {
	OPRD_CNT_ERR = 1,
	OPRD_TYPE_ERR
};

#define VALID_F_VAL(s)	(s == TBD_VALUE || s == ABS_VALUE || s == LOC_VALUE || s == REL_VALUE)
#define CONST_VAL(s)	(s == ABS_VALUE)

//static int label;
static FILE *f_obj, *f_lst;
static int f_lst_col;

static Cout_obj obj_file;
static Cout_lst lst_file;

static int  createOutputFiles(Cseg *sp);
static void ST8asmOut(Cseg *sp, line_t *lp, const ST8_inst_t *inst, int cur_addr);
static const ST8_inst_t *ST8asmParse(Cseg *sp, line_t *lp);
static bool isRelBranch(const ST8_inst_t *inst);

/////////////////////////////////////////////////////////////////////
int ST8_asm(Cseg *sp, int pass)
{
	line_t *lp = sp->lines;
	int v, cur_addr = 0;

	if ( pass == ASM_PASS3 )
	{
		if ( createOutputFiles(sp) )
			return 1;
	}

	while ( lp != NULL )
	{
		int addr_inc = 0;
		const ST8_inst_t *st8inst = NULL;

		if ( pass == ASM_PASS2 && lp->label != NULL && lp->inst != EQU )
		{
			symbol_t *p = add_symbol(sp, lp, lp->label, valItem(cur_addr), LABEL);

			if ( p != NULL )
			{
				if ( sp->isABS() )
					p->isABS = 1;

				p->global = lp->global_lbl? 1: 0;
			}
		}

		switch ( lp->inst )
		{
			case DEVICE:
			case 0:
				break;

			case CBLANK:
			case DBLANK:
				if ( pass == ASM_PASS1 && itemCount(lp) != 2 )
				{
					my_yyerror(lp, "illegal instruction operand!");
					return 1;
				}
				break;

			case SEGMENT:
				cur_addr = 0;		// each segment starts address 0x0000 (relative address)

				if ( sp->isABS() )
				{
					if ( expValue (sp, lp, lp->area->item, &v, 0) != ABS_VALUE )
					{
						my_yyerror (lp, "illegal absolute expression!");
						return 1;
					}

					sp->addr = v;
					cur_addr = v;
				}
				break;

			case DB: addr_inc = itemCount(lp);		break;
			case DW: addr_inc = itemCount(lp)*2;	break;
			case DT: addr_inc = itemCount(lp)*3;	break;
			case DD: addr_inc = itemCount(lp)*4;	break;

			case RS:
                if ( pass == ASM_PASS1 && itemCount(lp) != 1 )
				{
                    my_yyerror(lp, "illegal RS statement!");
					break;
				}

                if ( expValue(sp, lp, lp->items, &v, cur_addr) == ABS_VALUE )
					addr_inc += v;
				else
				{
                    my_yyerror(lp, "illegal expression!");
					return 1;
				}
				break;

			case EQU:
				if ( lp->label != NULL )
				{
					if ( pass == ASM_PASS1 )
					{
                        symbol_t *p = add_symbol(sp, lp, lp->label, cloneItem(lp->items), lp->inst);

						if ( p != NULL )
							p->type = lp->inst;
					}
				}
				break;

			case VECT:	// vector ...
			case ADD:	case ADDW: 	case ADC:	case AND: 	case BCCM: 	case BCP: 	case BCPL:
			case BRES: 	case BSET: 	case BTJF: 	case BTJT: 	case CALL: 	case CALLF: case CALLR:
			case CCF: 	case CLR: 	case CLRW: 	case CP: 	case CPW: 	case CPL: 	case CPLW:
			case DEC: 	case DECW: 	case DIV: 	case DIVW: 	case EXG: 	case EXGW: 	case HALT:
			case INC: 	case INCW: 	case IRET: 	case JP: 	case JPF: 	case JRA: 	case JRC:
			case JREQ: 	case JRF: 	case JRH: 	case JRIH: 	case JRIL: 	case JRM: 	case JRMI:
			case JRNC: 	case JRNE: 	case JRNH: 	case JRNM: 	case JRNV: 	case JRPL: 	case JRSGE:
			case JRSGT: case JRSLE:	case JRSLT: case JRT: 	case JRUGE: case JRUGT: case JRULE:
			case JRULT: case JRV: 	case LD: 	case LDF: 	case LDW: 	case MOV: 	case MUL:
			case NEG: 	case NEGW: 	case NOP: 	case OR: 	case POP: 	case POPW: 	case PUSH:
			case PUSHW: case RCF: 	case RET: 	case RETF: 	case RIM: 	case RLC: 	case RLCW:
			case RLWA: 	case RRC: 	case RRCW: 	case RRWA: 	case RVF: 	case SBC: 	case SCF:
			case SIM: 	case SLL: 	case SLLW: 	case SRA: 	case SRAW: 	case SRL: 	case SRLW:
			case SUB: 	case SUBW: 	case SWAP: 	case SWAPW: case TNZ: 	case TNZW: 	case TRAP:
			case WFE: 	case WFI: 	case XOR:
				if ( pass != ASM_PASS1 )
				{
					st8inst = ST8asmParse(sp, lp);

					if ( st8inst == NULL )
						my_yyerror(lp, "operand(s) error!");
					else
						addr_inc = st8inst->inst_size;
				}
				break;

            case S_COMMENT:
			case END:
            case INVOKE:
            	break;

			case _JP:
				addr_inc = 2;
				break;

			case _JBZ: case _JBNZ:
				addr_inc = 5;
				break;

			default:
                printf ("%d: ", lp->inst);
				my_yyerror(lp, "unknown instruction!");
				break;
		}

		if ( pass == ASM_PASS3 )
			ST8asmOut(sp, lp, st8inst, cur_addr);

		if ( pass == ASM_PASS3 && lp->next == NULL && sp->next == NULL )	// end of file
		{
			obj_file.flush();
			lst_file.flush();

			fclose(f_obj);
			fclose(f_lst);
			return 0;
		}

		cur_addr += addr_inc;
		lp = lp->next;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////
static int createOutputFiles (Cseg *sp)
{
	if ( f_obj == NULL )
	{
		int  length = strlen (sp->lines->fname);
		char *fname = new char[length + 4];

		strcpy(fname, sp->lines->fname);

		while (	--length > 0 )
		{
		 	if ( fname[length] == '.' ||
				 fname[length] == '/' ||
				 fname[length] == '\\' ) break;
		}

		if ( fname[length] != '.' )
		{
			yyerror("can't create output file!");
			return 1;
		}

		strcpy(&fname[length], ".obj");	// create object file
		f_obj = fopen(fname, "w");

		strcpy(&fname[length], ".lst");	// create list file
		f_lst = fopen(fname, "w");
		delete(fname);

		if ( f_obj == NULL || f_lst == NULL )
		{
			yyerror("can't create output file!");
			return 1;
		}

		obj_file.set_output(f_obj);
		lst_file.set_output(f_lst);

		for (symbol_t *sym_p = symbol_list; sym_p != NULL; sym_p = sym_p->next)
		{
			// first of all, output those global EQU items
			if ( sym_p->attr == EQU && sym_p->global )
			{
				char *str = expStr(sp, NULL, sym_p->item);
				obj_file.output('E', sym_p->name);
				obj_file.output('E', str);
				obj_file.flush();
			}
		}

		f_lst_col = 0;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
static void ST8asmOut(Cseg *sp, line_t *lp, const ST8_inst_t *inst, int cur_addr)
{
	if ( lp->label != NULL && lp->inst != EQU )
	{
		if ( lp->global_lbl )
			obj_file.output('G', lp->label);
		else
			obj_file.output('L', lp->label);

		obj_file.flush();
		lst_file.output(cur_addr, lp);
	}

	int seg_type = sp->type();

	if ( inst == NULL )
	{
		int  v1, i;
		char buf[1024];
		switch ( lp->inst )
		{
			case DEVICE:
				obj_file.output('P', device);
				for (i = 1; i < itemCount(lp); i++)
				{
					item_t *ip = itemPtr(lp, i);
					switch ( ip->type )
					{
						case TYPE_VALUE:	obj_file.output (ip->data.val);	break;
						case TYPE_SYMBOL:	obj_file.output (ip->data.str);	break;
					}
				}

				obj_file.flush();
				lst_file.output(lp);
				break;

			case INVOKE:	// invoke library file
				if ( lp->items != NULL &&
					 (lp->items->type == TYPE_STRING || lp->items->type == TYPE_SYMBOL) )
				{
					obj_file.output('I', lp->items->data.str);
					obj_file.flush();
					lst_file.output(lp);
				}
				break;

			case CBLANK:
			case DBLANK:
				expValue(sp, lp, lp->items, &v1, cur_addr);
				obj_file.output((lp->inst == CBLANK)? 'N': 'M', v1);

				expValue(sp, lp, lp->items->next, &v1, cur_addr);
				obj_file.output((lp->inst == CBLANK)? 'N': 'M', v1);

				obj_file.flush();
				lst_file.output(lp);
				break;

			case RS:
				lst_file.output(cur_addr, lp);
				expValue(sp, lp, lp->items, &v1, cur_addr);
				obj_file.output('R', v1);
				lst_file.output(v1, lp, RS);
				break;

			case SEGMENT:
				obj_file.output('S', lp->area->name);

				if ( sp->isOVR() )
					obj_file.output((char*)"OVR");

				if ( sp->isREL() )
					obj_file.output((char*)"REL");

				if ( sp->isABS() )
				{
					obj_file.output((char*)"ABS");
					if ( lp->area->item != NULL )
					{
						expValue(sp, lp, lp->area->item, &v1, 0);
						obj_file.output(v1);
					}
				}
				else if ( sp->isBEG() )
					obj_file.output((char*)"BEG");
				else if ( sp->isEND() )
					obj_file.output((char*)"END");

				obj_file.flush();
				lst_file.output(lp);

				if ( lp->mem_item && lp->mem_item->type == ':' &&
					 lp->mem_item->left->type == TYPE_SYMBOL 	)
				{
					char buf[128];
					expValue(sp, lp, lp->mem_item->right, &v1, 0);

					if ( v1 )
						sprintf(buf, "%s %d", lp->mem_item->left->data.str, v1);
					else
						sprintf(buf, "%s 0", lp->mem_item->left->data.str);

					obj_file.output('U', buf);
					obj_file.flush();
				}
				break;

            case DW:
			case DB:
			case DD:
			case DT:
				i = 0;
				buf[0] = 0;
            	for (item_t *ip = lp->items; ip; ip = ip->next)
            	{
					char cmd = (lp->inst == DW)? 'W':
							   (lp->inst == DB)? 'B':
							   (lp->inst == DD)? 'D': 'T';

                    if ( expValue(sp, lp, ip, &v1, 0) == ABS_VALUE )
                    {
						obj_file.output(cmd, v1);

						if ( i < 10 )
						{
							switch ( lp->inst )
							{
								case DB:	sprintf(&buf[i], " %02X", v1 & 0x000000ff);	break;
								case DW:	sprintf(&buf[i], " %04X", v1 & 0x0000ffff);	break;
								case DT:	sprintf(&buf[i], " %06X", v1 & 0x00ffffff);	break;
								case DD:	sprintf(&buf[i], " %08X", v1);				break;
							}
							i = strlen(buf);
						}
					}
					else
                    {
						char *str = expStr(sp, lp, ip);
						obj_file.output(cmd, str);

						if ( i < 10 )
						{
							switch ( lp->inst )
							{
								case DB:	strcat (buf, " ??");		break;
								case DW:	strcat (buf, " ????");		break;
								case DT:	strcat (buf, " ??????");	break;
								case DD:	strcat (buf, " ????????");	break;
							}
							i = strlen (buf);
						}
					}
				}
				lst_file.output(cur_addr, buf, lp);
            	break;

			case _JP:	// variable branch instruction
				obj_file.flush();
				if ( itemCount(lp) == 1 )
				{
					item_t *ip0 = itemPtr(lp, 0);
					obj_file.output('J', expStr(sp, lp, ip0));
				}
				else
				{
					item_t *ip0 = itemPtr(lp, 0);
					item_t *ip1 = itemPtr(lp, 1);
					obj_file.output('J', expStr(sp, lp, ip0), expStr(sp, lp, ip1));
				}
				lst_file.output(cur_addr, (char*)" ?? ??", lp);
				cur_addr += 2;	// temp length value
				break;

			case _JBZ:	case _JBNZ:	// variable bit test instructions
				obj_file.flush();
				if ( itemCount(lp) == 3 )
				{
					obj_file.flush();
					char type = (lp->inst == _JBZ)? 'U': 'V';
					char buf[256];
					item_t *ip0 = itemPtr(lp, 0);
					item_t *ip1 = itemPtr(lp, 1);
					item_t *ip2 = itemPtr(lp, 2);
					sprintf(buf, "%s %s", expStr(sp, lp, ip0), expStr(sp, lp, ip1));
					obj_file.output(type, buf, expStr(sp, lp, ip2));
					lst_file.output(cur_addr, (char*)" ?? ?? ?? ?? ??", lp);
					cur_addr += 5;
				}
				break;

			case S_COMMENT:
				obj_file.output(';', lp->items->data.str);
			default:
				lst_file.output(lp);
		}
		return;
	}

	int v1, v2, v3, size;
	char *str, buf[4096];
	EXP_STATUS s1 = expValue(sp, lp, itemPtr(lp, 0), &v1, cur_addr);
	EXP_STATUS s2 = expValue(sp, lp, itemPtr(lp, 1), &v2, cur_addr);
	EXP_STATUS s3 = expValue(sp, lp, itemPtr(lp, 2), &v3, cur_addr);

	int operand_bytes = inst->inst_size - inst->code_size;
	int operand_cnt = operCnt(inst);

	if ( operand_bytes <= 0 )
	{
		obj_file.output((inst->code_size > 1)? 'W': 'B', inst->code);
		lst_file.output(lp, cur_addr, inst->code, inst->code_size);
	}
	else if ( operand_cnt == 1 || (operand_cnt == 2 && operandIsReg(inst,1)) )
	{
		obj_file.output((inst->code_size > 1)? 'W': 'B', inst->code);
		lst_file.output(lp, cur_addr, inst->code, inst->code_size);

		if ( operand_cnt == 1 && operand_bytes == 1 && pickOpr(inst, 0) == ST8_EXP )
		{
			if ( s1 == ABS_VALUE || s1 == OFS_VALUE )
			{
				obj_file.output('B', v1);
				lst_file.output(v1, operand_bytes);
			}
			else if ( isRelBranch(inst) )
			{
				str = expStr(sp, lp, lp->items);
				sprintf (buf, "(%s-.)", str);
				obj_file.output('B', buf);
				lst_file.output((char*)NULL, operand_bytes);
			}
			else
			{
				str = expStr(sp, lp, lp->items);
				obj_file.output('B', str);
				lst_file.output((char*)NULL, operand_bytes);
			}
		}
		else
		{
			if ( s1 == ABS_VALUE )
			{
				obj_file.output((operand_bytes > 2)? 'T':
								(operand_bytes > 1)? 'W': 'B', v1);
				lst_file.output(v1, operand_bytes);
			}
			else
			{
				str = expStr(sp, lp, lp->items);
				obj_file.output((operand_bytes > 2)? 'T':
								(operand_bytes > 1)? 'W': 'B', str);
				lst_file.output((char*)NULL, operand_bytes);
			}
		}
	}
	else if ( operand_cnt == 2 && operandIsReg(inst, 0) )
	{
		obj_file.output((inst->code_size > 1)? 'W': 'B', inst->code);
		lst_file.output(lp, cur_addr, inst->code, inst->code_size);

		if ( s2 == ABS_VALUE )
		{
			obj_file.output((operand_bytes > 2)? 'T':
							(operand_bytes > 1)? 'W': 'B', v2);
			lst_file.output(v2, operand_bytes);
		}
		else
		{
			str = expStr(sp, lp, itemPtr(lp, 1));
			obj_file.output((operand_bytes > 2)? 'T':
			                (operand_bytes > 1)? 'W': 'B', str);
			lst_file.output((char*)NULL, operand_bytes);
		}
	}
	else if ( operand_bytes == 2 && inst->operands == ((ST8_EXP0 << 8) | ST8_EXP0) )
	{
		obj_file.output((inst->code_size > 1)? 'W': 'B', inst->code);
		lst_file.output(lp, cur_addr, inst->code, inst->code_size);

		if ( s2 == ABS_VALUE )
		{
			obj_file.output('B', v2);
			lst_file.output(v2, 1);
		}
		else
		{
			str = expStr(sp, lp, itemPtr (lp, 1));
			obj_file.output('B', str);
			lst_file.output((char*)NULL, 1);
		}

		if ( s1 == ABS_VALUE )
		{
			obj_file.output('B', v1);
			lst_file.output(v1, 1);
		}
		else
		{
			str = expStr(sp, lp, lp->items);
			obj_file.output('B', str);
			lst_file.output((char*)NULL, 1);
		}
	}
	else if ( operand_bytes == 4 && inst->operands == ((ST8_EXP << 8) | ST8_EXP) )
	{
		obj_file.output((inst->code_size > 1)? 'W': 'B', inst->code);
		lst_file.output(lp, cur_addr, inst->code, inst->code_size);

		if ( s2 == ABS_VALUE )
		{
			obj_file.output('W', v2);
			lst_file.output(v2, 2);
		}
		else
		{
			str = expStr(sp, lp, itemPtr(lp, 1));
			obj_file.output('W', str);
			lst_file.output((char*)NULL, 2);
		}

		if ( s1 == ABS_VALUE )
		{
			obj_file.output('W', v1);
			lst_file.output(v1, 2);
		}
		else
		{
			str = expStr(sp, lp, lp->items);
			obj_file.output('W', str);
			lst_file.output((char*)NULL, 2);
		}
	}
	else if ( operand_bytes == 3 && inst->operands == ((ST8_EXP << 8) | ST8_IMMD) )
	{
		obj_file.output((inst->code_size > 1)? 'W': 'B', inst->code);
		lst_file.output(lp, cur_addr, inst->code, inst->code_size);

		if ( s2 == ABS_VALUE )
		{
			obj_file.output('B', v2);
			lst_file.output(v2, 1);
		}
		else
		{
			str = expStr(sp, lp, itemPtr(lp, 1));
			obj_file.output('B', str);
			lst_file.output((char*)NULL, 1);
		}

		if ( s1 == ABS_VALUE )
		{
			obj_file.output('W', v1);
			lst_file.output(v1, 2);
		}
		else
		{
			str = expStr(sp, lp, lp->items);
			obj_file.output('W', str);
			lst_file.output((char*)NULL, 2);
		}
	}
	else if ( operand_bytes == 2 && inst->operands == ((ST8_EXP << 8) | ST8_IMMD) )
	{
		if ( s2 == ABS_VALUE )
		{
			int v = inst->code | ((v2 & 7) << 1);
			obj_file.output((inst->code_size > 1)? 'W': 'B', v);
			lst_file.output(lp, cur_addr, v, inst->code_size);

			if ( s1 == ABS_VALUE )
			{
				obj_file.output('W', v1);
				lst_file.output(v1, operand_bytes);
			}
			else
			{
				str = expStr(sp, lp, lp->items);
				obj_file.output('W', str);
				lst_file.output((char*)NULL, operand_bytes);
			}
		}
	}
	else if ( inst->code_size == 2 && operand_bytes == 3 &&
			  inst->operands == ((ST8_EXP << 16) | (ST8_IMMD << 8) | ST8_EXP) )
	{
		int v = inst->code | ((v2 & 7) << 1);

		if ( s2 == ABS_VALUE )
		{
			obj_file.output('W', v);
			lst_file.output(lp, cur_addr, v, 2);
		}
		else
		{
			str = expStr(sp, lp, itemPtr(lp, 1));
			sprintf (buf, "[0x%04X|((%s&7)<<1)]", inst->code, str);

			obj_file.output('W', buf);
			lst_file.output(lp, cur_addr, v >> 8, (char*)NULL, 1);
		}

		if ( s1 == ABS_VALUE )
		{
			obj_file.output('W', v1);
			lst_file.output(v1, 2);
		}
		else
		{
			str = expStr (sp, lp, lp->items);
			obj_file.output('W', str);
			lst_file.output((char*)NULL, 2);
		}

		if ( s3 == ABS_VALUE || s3 == OFS_VALUE )
		{
			obj_file.output('B', v3);
			lst_file.output(v3, 1);
		}
		else if ( isRelBranch(inst) )
		{
			str = expStr(sp, lp, itemPtr(lp, 2));
			sprintf(buf, "(%s-.)", str);
			obj_file.output('B', buf);
			lst_file.output((char*)NULL, 1);
		}
		else
		{
			str = expStr(sp, lp, itemPtr(lp, 2));
			obj_file.output('B', str);
			lst_file.output((char*)NULL, 1);
		}
	}
	else
	{
		printf ("opr = 0x%X\n", inst->operands);
	}
}

/////////////////////////////////////////////////////////////////////////////////
static const ST8_inst_t *ST8asmParse(Cseg *sp, line_t *lp)
{
	for (const ST8_inst_t *p = ST8_instCodeTbl; p && p->symbl; p++)
	{
		// match opcode & num_of_operands
		if ( p->symbl != lp->inst || itemCount(lp) != operCnt(p) )
			continue;

		EXP_STATUS s;
		bool oper_matched = 1;
		int i = 0, v;

		for (item_t *ip = lp->items; ip && oper_matched; ip = ip->next)
		{
			int opr = pickOpr(p, i++);

			switch ( opr )
			{
				case ST8_REGA:
				case ST8_REGX:
				case ST8_REGY:
				case ST8_REGSP:
				case ST8_REGXH:
				case ST8_REGXL:
				case ST8_REGYH:
				case ST8_REGYL:
				case ST8_REGCC:
					if ( ip->type != TYPE_REG )
						oper_matched = 0;
					else if ( !((opr == ST8_REGA  && ip->data.val == REG_A ) ||
								(opr == ST8_REGX  && ip->data.val == REG_X ) ||
								(opr == ST8_REGY  && ip->data.val == REG_Y ) ||
								(opr == ST8_REGSP && ip->data.val == REG_SP) ||
								(opr == ST8_REGXH && ip->data.val == REG_XH) ||
								(opr == ST8_REGXL && ip->data.val == REG_XL) ||
								(opr == ST8_REGYH && ip->data.val == REG_YH) ||
								(opr == ST8_REGYL && ip->data.val == REG_YL) ||
								(opr == ST8_REGCC && ip->data.val == REG_CC) ) )
					{
						oper_matched = 0;
					}
					break;

				case ST8_IMMD:
					if ( ip->type != '#' )
					{
						oper_matched = 0;
					}
					break;

				case ST8_EXP0:
				case ST8_EXP:
					if ( ip->type == TYPE_INDIR		  ||
						 ip->type == '#'			  ||
						 ip->type == TYPE_INDEX  	  ||
						 ip->type == TYPE_INDIR_INDEX ||
						 ip->type == TYPE_REG     		)
					{
						oper_matched = 0;
					}
					else if ( opr == ST8_EXP0 )
					{
						s = expValue (sp, lp, ip, &v, 0);
						if ( ip->bank0 || (s == ABS_VALUE && (v & ~255) == 0) )
							break;

						oper_matched = 0;
					}
					break;

				case ST8_INDIR0:
				case ST8_INDIR:
					if ( ip->type != TYPE_INDIR	)
					{
						oper_matched = 0;
					}
					else if ( opr == ST8_INDIR0 )
					{
						s = expValue(sp, lp, ip->left, &v, 0);
						if ( ip->left->bank0 || (s == ABS_VALUE && (v & ~255) == 0) )
							break;

						oper_matched = 0;
					}
					break;

				case ST8_INDEX_X:	// (X)
				case ST8_INDEX_Y:	// (Y)
					if ( ip->type != TYPE_INDEX	|| ip->left )
					{
						oper_matched = 0;
					}
					else if ( !((opr == ST8_INDEX_X && ip->right->data.val == REG_X) ||
							    (opr == ST8_INDEX_Y && ip->right->data.val == REG_Y)) )
					{
						oper_matched = 0;
					}
					break;

				case ST8_INDIR0_X:	// ([*mem], X)
				case ST8_INDIR0_Y:	// ([*mem], Y)
				case ST8_INDIR_X:	// ([mem], X)
				case ST8_INDIR_Y:	// ([mem], Y)
					if ( ip->type != TYPE_INDIR_INDEX )
					{
						oper_matched = 0;
						break;
					}

					if ( ((opr == ST8_INDIR0_X || opr == ST8_INDIR_X) && ip->right->data.val != REG_X) ||
						 ((opr == ST8_INDIR0_Y || opr == ST8_INDIR_Y) && ip->right->data.val != REG_Y) )
					{
						oper_matched = 0;
					}
					else if ( opr == ST8_INDIR0_X || opr == ST8_INDIR0_Y )
					{
						s = expValue(sp, lp, ip->left, &v, 0);

						if ( ip->left->bank0 || (s == ABS_VALUE && (v & ~255) == 0) )
							break;

						oper_matched = 0;
					}
					break;

				case ST8_OFFSET0_X:		// (*n, X)
				case ST8_OFFSET0_Y:		// (*n, Y)
				case ST8_OFFSET0_SP:	// (*n, SP)
				case ST8_OFFSET_X:		// (n, X)
				case ST8_OFFSET_Y:		// (n, Y)
					if ( ip->type != TYPE_INDEX	|| ip->left == NULL )
					{
						oper_matched = 0;
					}
					else if ( !((opr == ST8_OFFSET0_X  && ip->right->data.val == REG_X) ||
							    (opr == ST8_OFFSET0_Y  && ip->right->data.val == REG_Y) ||
							    (opr == ST8_OFFSET0_SP && ip->right->data.val == REG_SP)||
							    (opr == ST8_OFFSET_X   && ip->right->data.val == REG_X) ||
							    (opr == ST8_OFFSET_Y   && ip->right->data.val == REG_Y) ) )
					{
						oper_matched = 0;
					}
					else if ( opr == ST8_OFFSET0_X || opr == ST8_OFFSET0_X || opr == ST8_OFFSET0_SP )
					{
						s = expValue(sp, lp, ip->left, &v, 0);

						if ( ip->left->bank0 || (s == ABS_VALUE && (v & ~255) == 0) )
							break;

						oper_matched = 0;
					}
					break;

				default:
					oper_matched = 0;
			}
		}

		if ( oper_matched )
			return p;
	}

	return NULL;
}

static bool isRelBranch(const ST8_inst_t *inst)
{
	return (inst->symbl == BTJF ||
			inst->symbl == BTJT ||
			inst->symbl == CALLR||
			inst->symbl == JRA	||
			inst->symbl == JRC	||
			inst->symbl == JREQ	||
			inst->symbl == JRF	||
			inst->symbl == JRH	||
			inst->symbl == JRIH	||
			inst->symbl == JRIL	||
			inst->symbl == JRM	||
			inst->symbl == JRMI	||
			inst->symbl == JRNC	||
			inst->symbl == JRNE	||
			inst->symbl == JRNH	||
			inst->symbl == JRNM	||
			inst->symbl == JRNV	||
			inst->symbl == JRPL	||
			inst->symbl == JRSGE||
			inst->symbl == JRSGT||
			inst->symbl == JRSLE||
			inst->symbl == JRSLT||
			inst->symbl == JRT	||
			inst->symbl == JRUGE||
			inst->symbl == JRUGT||
			inst->symbl == JRULE||
			inst->symbl == JRULT||
			inst->symbl == JRV);
}
