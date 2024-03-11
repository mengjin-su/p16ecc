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

void Pcoder :: run5(oprNode_t *op)
{
	int depth = DEPTH();
	Pnode *pnp;
	Item *ip0, *ip1;
	int code = OPR_TYPE(op);

	switch ( code )
	{
		case ADD_ASSIGN:
		case SUB_ASSIGN:
			run(OPR_NODE(op, 0));
			run(OPR_NODE(op, 1));
			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();
				ip0 = POP();
				int size = ip0->stepSize();// incremental size

				if ( size > 0 && ip0->isWritable() && ip0->attr->type != SBIT && 
					 ip0->isOperable() && ip1->isOperable() && ip1->attr->type != SBIT )
				{
					if ( ip0->isAccePtr() && ip1->isMonoVal() )	// modify a pointer?
					{
						if ( ip1->type == CON_ITEM )
							ip1->val.i *= size;
						else if ( size > 1 )
						{
							pnp = new Pnode('*');
							addPnode(&mainPcode, pnp);
							pnp->items[2] = intItem(size);
							pnp->items[1] = ip1;
							pnp->items[0] = makeTemp();
							pnp->items[0]->attr = newAttr(INT);
							ip1 = pnp->items[0]->clone();
						}
					}

					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[1] = ip1;
					pnp->items[0] = ip0;
					PUSH(pnp->items[0]->clone());
					break;
				}
				delete ip0;
				delete ip1;
				errPrint("invalid +=/-= operation!");
			}
			break;

		case '+':
		case '-':
			run(OPR_NODE(op, 0));
			run(OPR_NODE(op, 1));
			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();
				ip0 = POP();
				if ( mergeConst(code, ip0, ip1) )
					return;

				if ( !(ip0->isOperable() && ip1->isOperable()) ||
					 ip0->attr->type == SBIT || ip1->attr->type == SBIT )
				{
					delete ip0;
					delete ip1;
					errPrint("invalid operand(s)!");
					return;
				}
				if ( (ip0->isAccePtr() || ip0->type == IMMD_ITEM) && ip1->isMonoVal() )
				{
					ptrBiasing(code, ip0, ip1);
					return;
				}
				if ( (ip1->isAccePtr() || ip1->type == IMMD_ITEM) && ip0->isMonoVal() && code == '+' )
				{
					ptrBiasing(code, ip1, ip0);
					return;
				}

				pnp = new Pnode(code);
				addPnode(&mainPcode, pnp);
				pnp->items[1] = ip0;
				pnp->items[2] = ip1;
				Item *tmp = makeTemp();
				tmp->attr = (ip0->isMonoVal() && ip1->isMonoVal())?	maxMonoAttr(ip0, ip1, code):
																	maxSizeAttr(ip0, ip1);
				pnp->items[0] = tmp;
				PUSH(pnp->items[0]->clone());
				return;
			}
			break;

		case '|': case '&': case '^':
			run(OPR_NODE(op, 0));
			run(OPR_NODE(op, 1));
			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();
				ip0 = POP();
				if ( mergeConst(code, ip0, ip1) )
					return;
				if ( ip0->isOperable() && ip1->isOperable() &&
					 ip0->attr->type != SBIT && ip0->attr->type != SBIT )
				{
					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[1] = ip0;
					pnp->items[2] = ip1;

					Item *tmp = makeTemp();
					tmp->attr = maxSizeAttr(ip0, ip1);
					tmp->attr->isUnsigned = 1;
					pnp->items[0] = tmp;
					PUSH(pnp->items[0]->clone());
					return;
				}
				delete ip0;
				delete ip1;
				errPrint("invalid operation type!");
				return;
			}
			break;

		case AND_ASSIGN:	case OR_ASSIGN:		case XOR_ASSIGN:
		case MUL_ASSIGN:	case DIV_ASSIGN:	case MOD_ASSIGN:
		case LEFT_ASSIGN:	case RIGHT_ASSIGN:
			run(OPR_NODE(op, 0));
			run(OPR_NODE(op, 1));
			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();
				ip0 = POP();
				if ( ip0->isOperable() && ip1->isOperable() && ip0->isWritable() && 
					 ip0->attr->type != SBIT && ip1->attr->type != SBIT 		  )
				{
					if ( code == AND_ASSIGN || code == OR_ASSIGN || code == XOR_ASSIGN )
					{
						pnp = new Pnode(code);
						addPnode(&mainPcode, pnp);
						pnp->items[0] = ip0;
						pnp->items[1] = ip1;
						PUSH(pnp->items[0]->clone());
						return;
					}
					if ( ip0->isMonoVal() && ip1->isMonoVal() )		// *=, /=, %=, >>=, <<=
					{
						pnp = new Pnode(code);
						addPnode(&mainPcode, pnp);
						pnp->items[0] = ip0;
						pnp->items[1] = ip1;
						PUSH(pnp->items[0]->clone());
						return;						
					}
				}
				delete ip0;
				delete ip1;
				errPrint("invalid operation type!");
				return;
			}
			break;

		default:
			run6(op);
			break;
	}
}

void Pcoder :: ptrBiasing(int code, Item *ip0, Item *ip1)
{
	Pnode *pnp;
	int scale = ip0->stepSize();

	if ( ip1->type == CON_ITEM )
		ip1->val.i *= scale;
	else if ( scale > 1 )
	{
		pnp = new Pnode('*');
		pnp->items[2] = intItem(scale);
		pnp->items[1] = ip1;
		pnp->items[0] = makeTemp();
		pnp->items[0]->attr = newAttr(INT);
		pnp->items[0]->attr->isUnsigned = ip1->attr->isUnsigned;
		addPnode(&mainPcode, pnp);

		ip1 = pnp->items[0]->clone();
	}

	pnp = new Pnode(code);
	addPnode(&mainPcode, pnp);
	pnp->items[2] = ip1;
	pnp->items[1] = ip0;
	pnp->items[0] = makeTemp(ip0->attr);
	PUSH(pnp->items[0]->clone());
}
