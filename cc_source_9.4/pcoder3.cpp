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
#include "display.h"

void Pcoder :: run3(oprNode_t *op)
{
	int depth = DEPTH();
	Pnode *pnp;
	Item *ip0;
	switch ( OPR_TYPE(op) )
	{
		case '.':	// struct/union direct addressing
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				node *np = OPR_NODE(op, 1);
				if ( np->type != NODE_ID )
				{
					DEL();
					errPrint("illegal member name!");
					return;
				}
				ip0 = POP();
				attrib *ap, *attr = cloneAttr(ip0->attr);
				int data_bank = 0, offset, i0_type;
				const char *error_msg = NULL;
				switch ( i0_type = ip0->type )
				{
					case ID_ITEM:	case DIR_ITEM:	case INDIR_ITEM:	case PID_ITEM:
						if ( !(attr->type == STRUCT || attr->type == UNION) )
							error_msg = "not a struct/union data1!";
						else if ( attr->dimVect )
							error_msg = "no array allowed struct/union!";
						else if ( i0_type != ID_ITEM  )
						{
							if ( ptrWeight(attr) != 1 )
								error_msg = "improper multi-pointer!";
						}
						else if ( ptrWeight(attr) != 0 )
							error_msg = "improper pointer!";

						if ( error_msg )
						{
							errPrint(error_msg);
							delAttr(attr);	delete ip0;
							return;
						}

						data_bank = (i0_type == ID_ITEM)? attr->dataBank:
														  reducePtr(attr);

						ap = sizer.memberAttrClone(attr, np->id.name);	// member's attr.
						if ( ap == NULL )
						{
							errPrint("struct/union member not found!", np->id.name);
							delAttr(attr);	delete ip0;
							return;
						}
						offset = sizer.memberOffset(attr, np->id.name);
						delAttr(attr);
						insertPtr(ap, data_bank);

						pnp = new Pnode('+');
						addPnode(&mainPcode, pnp);
						if ( i0_type == DIR_ITEM   ) ip0->type = CON_ITEM;
						if ( i0_type == ID_ITEM    ) ip0->type = IMMD_ITEM;
						if ( i0_type == INDIR_ITEM ) ip0->type = TEMP_ITEM;
						if ( i0_type == PID_ITEM   ) ip0->type = ID_ITEM;
						pnp->items[2] = intItem(offset);
						pnp->items[1] = ip0;
						pnp->items[0] = makeTemp();
						pnp->items[0]->attr = ap;

						PUSH(pnp->items[0]->clone());
						if ( !ap->dimVect ) TOP()->type = INDIR_ITEM;
						break;

					default:
						errPrint("unsupported 'X.Y' operation!");
						delAttr(attr);	delete ip0;
						break;
				}
			}
			break;

		case PTR_OP:	// struct/union indirect addressing
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				node *np = OPR_NODE(op, 1);
				if ( np->type != NODE_ID )
				{
					DEL();
					errPrint("illegal member name!");
					return;
				}
				ip0 = POP();
				attrib *ap, *attr = cloneAttr(ip0->attr);
				int data_bank = 0, offset, i0_type;
				const char *error_msg = NULL;
				switch ( i0_type = ip0->type )
				{
					case IMMD_ITEM:	 case ID_ITEM:	case TEMP_ITEM:	case ACC_ITEM:	case CON_ITEM:
					case INDIR_ITEM: case PID_ITEM:	case DIR_ITEM:
						if ( !(attr->type == STRUCT || attr->type == UNION) )
						{
							error_msg = "not a struct/union data2!";
						}
						else if ( i0_type == INDIR_ITEM || i0_type == PID_ITEM || i0_type == DIR_ITEM )
						{
							if ( ptrWeight(attr) != 2 ) error_msg = "improper multi-pointer!";
							reducePtr(attr);
						}
						else if ( i0_type == IMMD_ITEM )
						{
							if ( ptrWeight(attr) != 0 ) error_msg = "improper address!";
						}
						else
						{
							if ( ptrWeight(attr) != 1 )	error_msg = "improper pointer!";
						}

						if ( !error_msg && attr->dimVect )
							error_msg = "no array allowed struct/union!";

						if ( error_msg )
						{
							errPrint(error_msg);
							delAttr(attr);	delete ip0;
							return;
						}

						data_bank = attr->dataBank;
						if ( i0_type != IMMD_ITEM )
							data_bank = reducePtr(attr);

						ap = sizer.memberAttrClone(attr, np->id.name);	// member's attr.
						offset = sizer.memberOffset(attr, np->id.name);
						delAttr(attr);

						if ( ap == NULL )
						{
							errPrint("struct/union member not found!", np->id.name);
							delete ip0;
							return;
						}

						insertPtr(ap, data_bank);
						pnp = new Pnode('+');
						addPnode(&mainPcode, pnp);
						pnp->items[2] = intItem(offset);
						pnp->items[1] = ip0;
						pnp->items[0] = makeTemp();
						pnp->items[0]->attr = ap;

						PUSH(pnp->items[0]->clone());
						if ( !ap->dimVect ) TOP()->type = INDIR_ITEM;
						break;

					default:
						errPrint("unsupported 'X->Y' operation!");
						delAttr(attr);	delete ip0;
						break;
				}
			}
			break;

		case PRE_INC:
		case PRE_DEC:
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = POP();
				attrib *attr = ip0->attr;
				int code = (op->oper == PRE_INC)? INC_OP: DEC_OP;
				int size = ip0->stepSize();
				if ( attr->dimVect || !ip0->isWritable() || size == 0 ||
					 ip0->type == TEMP_ITEM || ip0->type == ACC_ITEM )
				{
					delete ip0;
					errPrint("inc/dec data type error!");
					return;
				}
				pnp = new Pnode(code);
				addPnode(&mainPcode, pnp);
				pnp->items[0] = ip0;
				pnp->items[1] = intItem(size);

				PUSH(pnp->items[0]->clone());
				return;
			}
			break;

		case POST_INC:
		case POST_DEC:
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = POP();
				attrib *attr = ip0->attr;
				int code = (op->oper == POST_INC)? INC_OP: DEC_OP;
				int size = ip0->stepSize();

				if ( attr->dimVect || !ip0->isWritable() || size == 0 ||
					 ip0->type == TEMP_ITEM || ip0->type == ACC_ITEM )
				{
					delete ip0;
					errPrint("inc/dec data type error!");
					return;
				}

				pnp = new Pnode('=');
				addPnode(&mainPcode, pnp);
				pnp->items[1] = ip0;

				if ( ip0->type == DIR_ITEM || ip0->type == PID_ITEM || ip0->type == INDIR_ITEM )
				{
					attrib *ap = cloneAttr(attr);
					reducePtr(ap);
					pnp->items[0] = makeTemp(ap);
					delAttr(ap);
				}
				else
					pnp->items[0] = makeTemp(attr);

				PUSH(pnp->items[0]->clone());

				pnp = new Pnode(code);
				addPnode(&mainPcode, pnp);
				pnp->items[0] = ip0->clone();
				pnp->items[1] = intItem(size);
			}
			break;

		default:
			run4(op);
			break;
	}
}