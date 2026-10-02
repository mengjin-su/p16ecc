#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
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
#include "display.h"

void Pcoder :: run8(oprNode_t *op)
{
	int depth = DEPTH();
	int code  = OPR_TYPE(op);
	node  *np;
	Pnode *pnp, *pnp0, *pnp1;
	Fnode *fp;
	int   true_lbl, false_lbl, end_lbl;
	Item  *ip0 = NULL, *ip1 = NULL;

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
							pnp = new Pnode(AASM);
							addPnode (&mainPcode, pnp);
							pnp->items[0] = strItem(np->str.str);
							return;
						}
					}
				}
				errPrint("asm(...) format error!");
				return;
			}

			run(np);
			if ( DEPTH() == (depth+1) )
			{
				Item *ip = POP();
				node *parnp;
				node *par_list= (op->nops > 1 && op->op[1])? op->op[1]: NULL;
				int   par_cnt = (op->nops > 1 && op->op[1])? LIST_LENGTH(par_list): 0;
				int   f_par_count;
				attrib *r_attr;
				int   *offset, size0;

				switch ( ip->type )
				{
					case ID_ITEM:		// it's function pointer
					case INDIR_ITEM:	// indirect function call
					case TEMP_ITEM:
					case ACC_ITEM:
					case DIR_ITEM:
						if ( !(ip->attr && ip->attr->isFptr) )
						{
							delete ip;
							errPrint("invalid function pointer!");
							return;
						}

						parnp = (node*)ip->attr->parList;
						f_par_count = LIST_LENGTH(parnp);
						if ( par_cnt < f_par_count ||
						    (par_cnt > f_par_count && parnp && !parnp->list.elipsis) )
						{
							delete ip;
							errPrint("parameter amount error!");
							return;
						}
						if ( f_par_count == 1 && par_cnt == 1 )
						{
							node *par0 = LIST_NODE(parnp, 0);
							size0 = passParameter(par0->id.attr, LIST_NODE(par_list, 0));
							offset = &size0;
						}
						else
						{
							pnp = new Pnode(P_DEC_STACK, intItem(0));
							addPnode(&mainPcode, pnp);
							offset = &pnp->items[0]->val.i;

							for (int i = 0; i < par_cnt; i++)
							{
								node *par_i = LIST_NODE(parnp, i);
								*offset += passParameter(par_i->id.attr,
														 LIST_NODE(par_list, i),
														 *offset,
														 i >= f_par_count);
							}
						}
						pnp = new Pnode(P_CALL);	// pointer function call
						addPnode(&mainPcode, pnp);
						pnp->items[0] = ip;

						r_attr = cloneAttr(ip->attr);// function return attr.
						r_attr->isFptr = 0;			// clear the Function flag!
						if ( ip->type == INDIR_ITEM || ip->type == DIR_ITEM )
							reducePtr(r_attr);

						addPnode(&mainPcode, new Pnode(P_INC_STACK, intItem(*offset)));
						break;

					case LBL_ITEM:	// regular function call
						fp = (Fnode*)ip->home;
						if ( fp == NULL )
						{
							delete ip;
							errPrint("invalid function name!");
							return;
						}
						f_par_count = fp->parCount();
						if ( par_cnt < f_par_count ||
							(par_cnt > f_par_count && !fp->elipsis) )
						{
							delete ip;
							errPrint("parameter amount error!");
							return;
						}

						if ( f_par_count == 1 && par_cnt == 1 )
						{
							size0 = passParameter(fp->parAttr(0), LIST_NODE(par_list, 0));
							offset = &size0;
						}
						else
						{
							pnp = new Pnode(P_DEC_STACK);
							addPnode(&mainPcode, pnp);
							pnp->items[0] = intItem(0);
							offset = &pnp->items[0]->val.i;

							for (int i = 0; i < par_cnt; i++)
								*offset += passParameter(fp->parAttr(i),
														 LIST_NODE(par_list, i),
														 *offset, i >= f_par_count);
						}

						pnp = new Pnode(CALL);
						addPnode (&mainPcode, pnp);
						pnp->items[0] = ip;

						r_attr = cloneAttr(fp->attr);	// function return attr.

						pnp = new Pnode(P_INC_STACK, intItem(*offset));
						addPnode(&mainPcode, pnp);
						break;

					default:
						errPrint("invalid function name!");
						return;
				}

				// return value stored in ACC
				if ( r_attr && (r_attr->type != VOID || r_attr->ptrVect) )
				{
					pnp = new Pnode('=');
					addPnode (&mainPcode, pnp);
					pnp->items[0] = makeTemp(r_attr);
					pnp->items[1] = accItem(cloneAttr(r_attr));
					PUSH(pnp->items[0]->clone());
				}
				delAttr(r_attr);
			}
			break;

		case '?':
			true_lbl  = getLbl();
			false_lbl = getLbl();
			end_lbl   = getLbl();
			logicBranch(op->op[0], true_lbl, false_lbl, true_lbl);

			PUT_LBL(true_lbl);
			run(op->op[1]);
			if ( DEPTH() == (depth+1) )
			{
				pnp0 = new Pnode(P_MOV);
				addPnode (&mainPcode, pnp0);
				pnp0->items[1] = ip0 = POP();

				pnp = new Pnode(GOTO);
				addPnode (&mainPcode, pnp);
				pnp->items[0] = lblItem(end_lbl);

				PUT_LBL(false_lbl);
				run(op->op[2]);
				if ( DEPTH() == (depth+1) )
				{
					pnp1 = new Pnode(P_MOV);
					addPnode (&mainPcode, pnp1);
					pnp1->items[1] = ip1 = POP();
				}
				PUT_LBL(end_lbl);
			}

			if ( ip0 && ip1 && ip0->isOperable() && ip1->isOperable() )
			{
				int size0 = ip0->acceSize();
				int size1 = ip1->acceSize();
				if ( size0 <= 4 && size1 <= 4 )
				{
					int max_size = (size0 >= size1)? size0: size1;
					if ( max_size > 0 ) max_size--;

					Item *ip = makeTemp();
					ip->attr = newAttr(max_size+CHAR);
					ip->attr->isUnsigned = (ip0->acceSign() && ip1->acceSign())? 0: 1;
					pnp0->items[0] = ip->clone();
					pnp1->items[0] = ip->clone();
					PUSH(ip);
					break;
				}
			}
			if ( pnp0 ) delete pnp0;
			if ( pnp1 ) delete pnp1;
			errPrint("invalid size/type for '?' operation!");
			break;

		case ',':
			run(op->op[0]);
			run(op->op[1]);
			break;

		case RETURN:
			fp = curFnode;
			if ( op->nops == 1 ) // return with a value
			{
				if ( fp->attr && (fp->attr->type != VOID || fp->attr->ptrVect) )
				{
					run(op->op[0]);
					if ( DEPTH() == (depth+1) )
					{
						ip0 = POP();
						if ( ip0->isOperable() )
						{	// put the returning value into ACC
							pnp = new Pnode('=');
							addPnode (&mainPcode, pnp);
							pnp->items[0] = accItem(cloneAttr(fp->attr));
							pnp->items[1] = ip0;
							// jump to the end of function
							pnp = new Pnode(GOTO);
							addPnode (&mainPcode, pnp);
							pnp->items[0] = lblItem(fp->endLbl);
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
			else				// return without value
			{
				if ( fp->attr && (fp->attr->type != VOID || fp->attr->ptrVect) )
					errPrint("missing return value!");
				else
				{	// jump to the end of function
					pnp = new Pnode(GOTO);
					addPnode(&mainPcode, pnp);
					pnp->items[0] = lblItem(fp->endLbl);
				}
			}
			break;

		default:
			run9(op);
	}
}

int Pcoder :: passParameter(attrib *attr, node *par, int offset, bool ext_par)
{
	if ( attr == NULL && !ext_par )
	{
		errPrint("parameter error!");
		return 0;
	}

	int depth = DEPTH();
	run(par);
	if ( DEPTH() == (depth+1) )
	{
		char buf[10];
		sprintf(buf, "%d", offset+1);
		Pnode *pnp = new Pnode(P_ARG_PASS);	addPnode (&mainPcode, pnp);
//		pnp->items[2] = intItem(par_count);
		pnp->items[1] = POP();
		pnp->items[0] = idItem(buf);
		pnp->items[0]->attr = ext_par? newAttr(LONG): cloneAttr(attr);
		if ( !moveMatch(pnp->items[0], pnp->items[1]) )
			errPrint("unmatched parameter!");
	}
	return ext_par? 4: sizer.size(attr, ATTR_SIZE);
}

int Pcoder :: passParameter(attrib *attr, node *par)
{
	if ( attr == NULL )
	{
		errPrint("parameter error!");
		return 0;
	}

	int depth = DEPTH();
	run(par);
	if ( DEPTH() == (depth+1) )
	{
		Pnode *pnp = new Pnode(P_ARG_PASS1);	addPnode (&mainPcode, pnp);
		pnp->items[1] = POP();
		pnp->items[0] = idItem((char*)"1");
		pnp->items[0]->attr = cloneAttr(attr);
		if ( !moveMatch(pnp->items[0], pnp->items[1]) )
			errPrint("unmatched parameter!");
	}
	return sizer.size(attr, ATTR_SIZE);
}
