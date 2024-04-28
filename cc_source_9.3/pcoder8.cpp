#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "nlist.h"
#include "item.h"
#include "pnode.h"
#include "pcoder.h"
#include "dlink.h"
#include "flink.h"
#include "sizer.h"
#include "pcoder.h"
#include "./p16e/pic16e_inst.h"

void Pcoder :: run8(oprNode_t *op)
{
	int depth = DEPTH();
	int code  = OPR_TYPE(op);
	node  *np;
	Pnode *pp, *pp0 = NULL, *pp1 = NULL;
	Fnode *fp = NULL;
	int   true_lbl, false_lbl, end_lbl;
	Item  *ip0, *ip1;

	switch ( code )
	{
		case CALL:
			np = op->op[0];
			if ( np->type == NODE_ID && strcmp(np->id.name, "asm") == 0 )
			{	// it's an in-line assembly code...
				if ( op->nops > 1 )
				{
					np = op->op[1];	// fetch parameter list
					if ( np->type == NODE_LIST && LIST_LENGTH(np) == 1 )
					{
						np = LIST_NODE(np, 0);
						if ( np->type == NODE_STR )
						{
							pp = new Pnode(AASM);
							addPnode (&mainPcode, pp);
							pp->items[0] = strItem(np->str.str);
							return;
						}
					}
				}
				errPrint("asm(...) format error!");
				return;
			}

			if ( np->type == NODE_ID && buildInAsm((node*)op) )
				return;

			run(np);
			if ( DEPTH() == (depth+1) )
			{
				Item *ip = POP();
				node *par_list= (op->nops > 1 && op->op[1])? op->op[1]: NULL;
				int   par_cnt = (op->nops > 1 && op->op[1])? LIST_LENGTH(par_list): 0;
				int   f_par_count;
				attrib *r_attr = NULL;
				node *parnp = NULL;
				bool par_elipsis;

				switch ( ip->type )
				{
					case ID_ITEM:	// it's function pointer
					case TEMP_ITEM:
					case ACC_ITEM:
					case DIR_ITEM:
					case PID_ITEM:
					case INDIR_ITEM:
						if ( !(ip->attr && ip->attr->isFptr) )
						{
							delete ip;
							errPrint("invalid function pointer!");
							return;
						}
						parnp = (node*)ip->attr->parList;
						f_par_count = LIST_LENGTH(parnp);
						par_elipsis = (parnp && parnp->list.elipsis);

						r_attr = cloneAttr(ip->attr);// function return attr.
						r_attr->isFptr = 0;
						if ( ip->type == INDIR_ITEM || ip->type == DIR_ITEM || ip->type == PID_ITEM )
							reducePtr(r_attr);
						break;

					case LBL_ITEM:	// it's a regular function call
						fp = (Fnode*)ip->home;
						if ( fp == NULL )
						{
							delete ip;
							errPrint("invalid function name!");
							return;
						}
						f_par_count = fp->parCount();
						par_elipsis = fp->elipsis? true: false;

						r_attr = cloneAttr(fp->attr);
						break;

					default:
						delete ip;
						errPrint("invalid function name!");
						return;
				}

				if ( par_cnt < f_par_count ||
					(par_cnt > f_par_count && !par_elipsis) )
				{
					delete ip;
					delAttr(r_attr);
					errPrint("parameter amount error!");
					return;
				}
				// push parameter(s) into stack...
				for (int i = 0; i < par_cnt; i++)
				{
					attrib *par_attr = fp? fp->parAttr(i): LIST_NODE(parnp, i)->id.attr;
					passParameter(par_attr, LIST_NODE(par_list, i), i, f_par_count);
				}

				pp = fp? new Pnode(CALL): new Pnode(P_CALL);
				pp->items[0] = ip;
				addPnode(&mainPcode, pp);

				// clear up extra parameters' space ...
				if ( par_cnt > f_par_count )
				{
					pp = new Pnode(P_ARG_CLEAR);
					addPnode (&mainPcode, pp);
					pp->items[0] = intItem((par_cnt - f_par_count)*4);
				}
				// return value stored in ACC
				if ( r_attr && (r_attr->type != VOID || r_attr->ptrVect) )
				{
					pp = new Pnode('=');
					addPnode (&mainPcode, pp);
					pp->items[0] = makeTemp(r_attr);
					pp->items[1] = accItem(r_attr);
					PUSH(pp->items[0]->clone());
				}
				else
					delAttr(r_attr);
			}
			break;

		case '?':
			true_lbl  = getLbl();
			false_lbl = getLbl();
			end_lbl   = getLbl();
			logicBranch(op->op[0], true_lbl, false_lbl, true_lbl);
			ip0 = ip1 = NULL;

			PUT_LBL(true_lbl);
			run(op->op[1]);
			if ( DEPTH() == (depth+1) )
			{
				pp0 = new Pnode(P_MOV);
				addPnode(&mainPcode, pp0);
				pp0->items[1] = ip0 = POP();

				pp = new Pnode(GOTO);
				addPnode(&mainPcode, pp);
				pp->items[0] = lblItem(end_lbl);
			}

			PUT_LBL(false_lbl);
			run(op->op[2]);
			if ( DEPTH() == (depth+1) )
			{
				pp1 = new Pnode(P_MOV);
				addPnode(&mainPcode, pp1);
				pp1->items[1] = ip1 = POP();
			}
			PUT_LBL(end_lbl);

			if ( ip0 && ip1 && ip0->isOperable() && ip1->isOperable() )
			{
				int size0 = ip0->acceSize();
				int	size1 = ip1->acceSize();
				if ( size0 < size1 ) size0 = size1;
				size0 = (size0 > 0)? size0-1: 0;
				Item *ip = makeTemp();
				ip->attr = newAttr(size0 + CHAR);
				ip->attr->isUnsigned = (ip0->acceSign() && ip1->acceSign())? 0: 1;

				pp0->items[0] = ip->clone();
				pp1->items[0] = ip->clone();
				PUSH(ip);
			}
			else // something wrong, dump both pp0 and pp1
			{
				if ( pp0 ) delete pp0;
				if ( pp1 ) delete pp1;
			}
			break;

		case ',':
			run(op->op[0]); if ( DEPTH() > depth ) DEL(DEPTH() - depth);
			run(op->op[1]);
			break;

		case RETURN:
			fp = curFnode;
			if ( op->nops == 0 ) // return without value
			{
				if ( !ISR_FUNC(fp) && fp->attr && (fp->attr->type != VOID || fp->attr->ptrVect) )
					errPrint("missing return value!");
				else
				{	// jump to the end of function
					pp = new Pnode(GOTO);
					addPnode(&mainPcode, pp);
					pp->items[0] = lblItem(fp->endLbl);
				}
			}
			else				// return with a value
			{
				if ( !ISR_FUNC(fp) && fp->attr && (fp->attr->type != VOID || fp->attr->ptrVect) )
				{
					run(op->op[0]);
					if ( DEPTH() == (depth+1) )
					{
						ip0 = POP();
						if ( ip0->isOperable() )
						{	// put the returning value into ACC
							pp = new Pnode('=');
							addPnode (&mainPcode, pp);
							pp->items[0] = accItem(cloneAttr(fp->attr));
							pp->items[1] = ip0;
							// jump to the end of function
							pp = new Pnode(GOTO);
							addPnode (&mainPcode, pp);
							pp->items[0] = lblItem(fp->endLbl);
						}
						else
						{
							delete ip0;
							errPrint("invalid return value!");
						}
					}
				}
				else
				{
					errPrint("invalid 'return'!");
				}
			}
			break;

		case AASM:
			pp = new Pnode(AASM);
			addPnode(&mainPcode, pp);
			pp->items[0] = strItem(op->op[0]->str.str);
			break;

		case PRAGMA:
			pp = new Pnode(PRAGMA);
			addPnode(&mainPcode, pp);
			pp->items[0] = idItem(op->op[0]->id.name);
			if ( op->nops > 1 )
			{
				run(op->op[1]);
				if ( DEPTH() == (depth+1) )
					pp->items[1] = POP();
				else
					errPrint("unknown item in pragma!");
			}
			break;

		default:
			printf("unknown oper type - %d!\n", code);
	}
}

void Pcoder :: passParameter(attrib *attr, node *par, int par_index, int par_count)
{
	if ( attr == NULL && par_index < par_count )
	{
		errPrint("parameter error!");
		return;
	}

	int depth = DEPTH();
	run(par);
	if ( DEPTH() == (depth+1) )
	{
		char buf[10];
		sprintf(buf, "%d", par_index);
		Pnode *pnp = new Pnode(P_ARG_PASS);	addPnode (&mainPcode, pnp);
		pnp->items[2] = intItem(par_count);
		pnp->items[1] = POP();
		pnp->items[0] = idItem(buf);
		pnp->items[0]->attr = (par_index >= par_count)?	newAttr(LONG):
														cloneAttr(attr);
		if ( !moveMatch(pnp->items[0], pnp->items[1]) )
			errPrint("unmatched parameter!");
	}
}

/////////////////////////////////////////////////////////////////////////
const asmCode P16InstCode[] = {
	{"_MOVF_W", 	_MOVF, 	1},	{"_MOVF_F", 	_MOVF,	1},
	{"_RLF_W", 		_RLF, 	1},	{"_RLF_F", 		_RLF, 	1},
	{"_LSLF_W",		_LSLF, 	1},	{"_LSLF_F", 	_LSLF,	1},
    {"_LSRF_W", 	_LSRF, 	1}, {"_LSRF_F", 	_LSRF, 	1},
    {"_ASRF_W", 	_ASRF, 	1}, {"_ASRF_F", 	_ASRF, 	1},
	{"_RRF_W", 		_RRF, 	1},	{"_RRF_F", 		_RRF, 	1},
	{"_MOVWF", 		_MOVWF, 1},
	{"_ADDWF_W", 	_ADDWF, 1},	{"_ADDWF_F", 	_ADDWF, 1},
	{"_ADDWFC_W", 	_ADDWFC,1},	{"_ADDWFC_F", 	_ADDWFC,1},
	{"_SUBWF_W", 	_SUBWF, 1},	{"_SUBWF_F", 	_SUBWF, 1},
	{"_SUBWFB_W", 	_SUBWFB,1},	{"_SUBWFB_F", 	_SUBWFB,1},
	{"_IORWF_W", 	_IORWF, 1},	{"_IORWF_F", 	_IORWF, 1},
	{"_XORWF_W", 	_XORWF, 1},	{"_XORWF_F", 	_XORWF, 1},
	{"_ANDWF_W", 	_ANDWF, 1},	{"_ANDWF_F", 	_ANDWF, 1},
	{"_INCFSZ", 	_INCFSZ,1},
	{"_DECFSZ", 	_DECFSZ,1},
	{"_INCF_W", 	_INCF, 	1},	{"_INCF_F", 	_INCF, 	1},
	{"_DECF_W", 	_DECF, 	1},	{"_DECF_F", 	_DECF, 	1},
	{"_COMF_F", 	_COMF, 	1},	{"_COMF_W", 	_COMF, 	1},
	{"_CLRF", 		_CLRF, 	1},
	{"_BTFSC", 		_BTFSC, 2},	{"_BTFSS", 		_BTFSS, 2},
    {"_BCF", 		_BCF, 	2}, {"_BSF", 		_BSF, 	2},
	{NULL, 			NULL,   0}
};

bool Pcoder :: buildInAsm(node *np)
{
    idNode_t *ip = (idNode_t *)np->opr.op[0];
    int pars = (np->opr.nops > 1)? LIST_LENGTH(np->opr.op[1]): 0;

	for (const asmCode *afp = P16InstCode; afp->name; afp++)
	{
		if ( strcmp(afp->name, ip->name) == 0 && afp->pars == pars )
		{
			Pnode *pnp = new Pnode(P_ASMFUNC, ptrItem ((void*)afp));
			for (int i = 0; i < pars; i++)
			{
				int depth = DEPTH();
				run(LIST_NODE(np->opr.op[1], i));
				if ( DEPTH() == (depth+1) )
					pnp->items[i+1] = POP();
				else
				{
					errPrint("asm code error!");
					delete pnp;
					return true;
				}
			}
			addPnode(&mainPcode, pnp);
	        return true;
		}
	}
	return false;
}