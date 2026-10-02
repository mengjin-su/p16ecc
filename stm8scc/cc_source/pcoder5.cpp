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

				if ( ip0->isWritable() && size > 0 &&
					 ip0->isOperable() && ip1->isOperable() )
				{
					if ( ip0->isAccePtr() )	// modify a pointer?
					{
						if ( ip1->isMonoVal() )
						{
							if ( size > 1 && ip1->type != CON_ITEM )
							{
								pnp = new Pnode('*');
								addPnode(&mainPcode, pnp);
								pnp->items[2] = intItem(size);
								pnp->items[1] = ip1;
								pnp->items[0] = makeTemp();
								pnp->items[0]->attr = newAttr(INT);
								ip1 = pnp->items[0]->clone();
							}
							else if ( ip1->type == CON_ITEM )
							{
								ip1->val.i *= size;
								code = (code == ADD_ASSIGN)? P_INC: P_DEC;
							}

							pnp = new Pnode(code);
							addPnode(&mainPcode, pnp);
							pnp->items[1] = ip1;
							pnp->items[0] = ip0;
							PUSH(pnp->items[0]->clone());
							return;
						}
						delete ip0;
						delete ip1;
						errPrint("increment/decrement value error!");
						break;
					}

					if ( !ip0->isMonoVal() || !ip1->isMonoVal() )
						warningPrint("type not compatible!", NULL);

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
				attrib *ap0 = ip0->attr;
				attrib *ap1 = ip1->attr;

				if ( !ap0 || !ap1 )
				{
					delete ip0;
					delete ip1;
					errPrint("invalid operand(s)!");
					return;
				}
				if ( ip0->type == CON_ITEM && ip1->type == CON_ITEM )
				{
					int n = (code == '+')? ip0->val.i + ip1->val.i:
										   ip0->val.i - ip1->val.i;
					PUSH(intItem(n));
					if ( ap0->ptrVect && !ap1->ptrVect )
					{
						n = (code == '+')? ip0->val.i + ip1->val.i*ip0->stepSize():
										   ip0->val.i - ip1->val.i*ip0->stepSize();
						TOP()->val.i = n;
						TOP()->updatePtr(cloneDim(ap0->ptrVect));
					}
					return;
				}
				if ( ip0->isMonoVal() && ip1->isMonoVal() )
				{
					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[1] = ip0;
					pnp->items[2] = ip1;

					Item *tmp = makeTemp();
					tmp->attr = maxMonoAttr(ip0, ip1, code);
					pnp->items[0] = tmp;
					PUSH(pnp->items[0]->clone());
					return;
				}
				if ( ip0->type == IMMD_ITEM && ip1->isMonoVal() )
				{
					int size = ip0->stepSize();	//printf ("IMMD + N --- %d\n", size);
					if ( size > 1 && ip1->type != CON_ITEM )
					{
						pnp = new Pnode('*');
						addPnode(&mainPcode, pnp);
						pnp->items[2] = intItem(size);
						pnp->items[1] = ip1;
						pnp->items[0] = makeTemp();
						pnp->items[0]->attr = newAttr(INT);
						ip1 = pnp->items[0]->clone();
					}
					else if ( ip1->type == CON_ITEM )
					{
						ip1->val.i *= size;
					}
					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[2] = ip1;
					pnp->items[1] = ip0;
					pnp->items[0] = makeTemp(ip0->attr);
					insertPtr(pnp->items[0]->attr, ip0->attr->dataBank);
					clearDim(pnp->items[0]->attr);
					PUSH(pnp->items[0]->clone());
					return;
				}
				if ( ip0->isOperable() && ip1->isOperable() )
				{
					if ( ip0->isAccePtr() )			// ip0 is pointer's value?
					{
						if ( ip1->isMonoVal() )
						{
							ptrBiasing(code, ip0, ip1);
							return;
						}
					}
					else if ( ip1->isAccePtr() )	// ip1 is pointer's value?
					{
						if ( ip0->isMonoVal() && code == '+' )
						{
							ptrBiasing(code, ip1, ip0);
							return;
						}
					}

					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[1] = ip0;
					pnp->items[2] = ip1;

					Item *tmp = makeTemp();
					tmp->attr = maxSizeAttr(ip0, ip1);
					tmp->attr->isUnsigned = 1;
					pnp->items[0] = tmp;
					PUSH(pnp->items[0]->clone());
					warningPrint("improper data type!", NULL);
					return;
				}
				delete ip0;
				delete ip1;
				errPrint("invalid operation type!");
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
				attrib *ap0 = ip0->attr;
				attrib *ap1 = ip1->attr;
				if ( ip0->type == CON_ITEM && ip1->type == CON_ITEM )
				{
					int n = (code == '|')? ip0->val.i | ip1->val.i:
							(code == '&')? ip0->val.i & ip1->val.i:
										   ip0->val.i ^ ip1->val.i;
					PUSH(intItem(n));
					return;
				}
				if ( ap0 && ap1 && ip0->isOperable() && ip1->isOperable() )
				{
					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[1] = ip0;
					pnp->items[2] = ip1;

					Item *tmp = makeTemp();
//					tmp->attr = maxMonoAttr(ip0, ip1, code);
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
//				attrib *ap0 = ip0->attr;
				if ( ip0->isWritable() && ip0->isOperable() )
				{
					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[0] = ip0;
					pnp->items[1] = ip1;
					PUSH(pnp->items[0]->clone());
					return;
				}
				if ( code == AND_ASSIGN || code == OR_ASSIGN || code == XOR_ASSIGN )
				{
					if ( ip0->isWritable() && ip0->isBitVal() && ip1->isBitVal() )
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
