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

void Pcoder :: run2(oprNode_t *op)
{
	int depth = DEPTH();
	Pnode *pnp;
	Item *ip0, *ip1;
	int code = OPR_TYPE(op);

	switch ( code )
	{
		case '=':	// x := y
			run(OPR_NODE(op, 0));	// scan 'x'
			run(OPR_NODE(op, 1));	// scan 'y'

			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();		// get 'y'
				ip0 = POP();		// get 'x'

				if ( ip0->isWritable() && moveMatch(ip0, ip1) )
				{
					pnp = new Pnode('=');
					pnp->items[0] = ip0;
					pnp->items[1] = ip1;
					addPnode(&mainPcode, pnp);

					PUSH((ip1->type == CON_ITEM)? ip1->clone():
												  ip0->clone());
				}
				else
				{
					delete ip0,	delete ip1;
					errPrint("assignment error!");
				}
			}
			else
				DEL(DEPTH() - depth);
			break;

		case ADDR_OF:
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = TOP();
				attrib *attr = ip0->attr;
				switch ( ip0->type )
				{
					case ID_ITEM:	ip0->type = IMMD_ITEM;	return;
					case INDIR_ITEM:ip0->type = TEMP_ITEM;	return;
					case PID_ITEM:	ip0->type = ID_ITEM;	return;
					case DIR_ITEM:	ip0->type = CON_ITEM;	return;
						
					case IMMD_ITEM:
						if ( attr && attr->dimVect )
						{
//							warningPrint("improper address specifying!", NULL);
							free(attr->dimVect),
							attr->dimVect = NULL;
							return;
						}
						break;

					case TEMP_ITEM:
					case ACC_ITEM:
						if ( ptrWeight(attr) > 0 )
						{
							if ( attr->dimVect )
								decDim(attr);
							else
								reducePtr(attr);
							return;
						}
					default:
						break;
				}
				DEL();
				errPrint("can't locate address!");
			}
			break;

		case POS_OF:	// indirect access using pointer - *p
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = TOP();
				attrib *attr = ip0->attr;
				switch ( ip0->type )
				{
					case CON_ITEM:
					case TEMP_ITEM:					
					case ID_ITEM:
					case ACC_ITEM:
						if ( ptrWeight(attr) > 0 )
						{
							switch ( ip0->type )
							{
								case CON_ITEM:	ip0->type = DIR_ITEM;	break;
								case TEMP_ITEM:	ip0->type = INDIR_ITEM;	break;
								case ID_ITEM:	ip0->type = PID_ITEM;	break;
								default:		pnp = new Pnode('=');
												addPnode(&mainPcode, pnp);
												pnp->items[1] = POP();
												pnp->items[0] = makeTemp(attr);
												PUSH(pnp->items[0]->clone());
												TOP()->type = INDIR_ITEM;
							}
							return;
						}
						break;
					case IMMD_ITEM:
						if ( dimDepth(attr) == 0 )
						{
							ip0->type = ID_ITEM;
							return;
						}
						break;
					case INDIR_ITEM:
					case PID_ITEM:
					case DIR_ITEM:
						if ( ptrWeight(attr) > 1 )
						{
							pnp = new Pnode('=');
							addPnode(&mainPcode, pnp);
							pnp->items[1] = POP();
							ip0 = makeTemp(attr);
							reducePtr(ip0->attr);
							pnp->items[0] = ip0;

							PUSH(ip0->clone());
							TOP()->type = INDIR_ITEM;
							return;
						}
					default:
						break;
				}
				DEL();
				errPrint("illegal indirect addressing!");
			}
			break;

		case '[':
			run(OPR_NODE(op, 0));
			run(OPR_NODE(op, 1));
			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();
				ip0 = POP();
				attrib *attr = ip0->attr;
				if ( !ip1->isMonoVal() || !attr )
				{
					delete ip0; delete ip1;
					errPrint("illegal index value!");
					return;
				}

				switch ( ip0->type )
				{
					case ID_ITEM:
					case TEMP_ITEM:
					case CON_ITEM:
					case ACC_ITEM:					
						if ( ptrWeight(attr) > 0 )
						{
							attrib *ap = cloneAttr(attr);
							int mem_type = -1;
							if ( dimDepth(ap) > 0 )
							{
								if ( ip0->type == TEMP_ITEM && ptrWeight(ap) )
									mem_type = reducePtr(ap);

								ip1 = makeOffset(ip1, sizer(ap, SUBDIM_SIZE));
								decDim(ap);
							}
							else // input_dim <= 0
							{
								mem_type = reducePtr(ap);
								ip1 = makeOffset(ip1, sizer(ap, ATTR_SIZE));
							}

							if ( mem_type != -1 )
								insertPtr(ap, mem_type);

							pnp = new Pnode('+');
							addPnode(&mainPcode, pnp);
							pnp->items[2] = ip1;
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp();
							pnp->items[0]->attr = ap;
							PUSH(pnp->items[0]->clone());

							if ( !ap->dimVect )	TOP()->type = INDIR_ITEM;
							return;
						}
						break;
					case IMMD_ITEM:
						if ( dimDepth(attr) > 0 )
						{
							attrib *ap = cloneAttr(attr);
							ip1 = makeOffset(ip1, sizer(ap, SUBDIM_SIZE));
							decDim(ap);
							insertPtr(ap, attr->dataBank);

							pnp = new Pnode('+');
							addPnode(&mainPcode, pnp);
							pnp->items[2] = ip1;
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp();
							pnp->items[0]->attr = ap;
							PUSH(pnp->items[0]->clone());

							if ( !ap->dimVect )	TOP()->type = INDIR_ITEM;
							return;
						}
						break;
					case INDIR_ITEM:
					case PID_ITEM:
					case DIR_ITEM:					
						if ( dimDepth(attr) > 0 )
						{
							attrib *ap = cloneAttr(attr);
							int mem_type = reducePtr(ap);
							ip1 = makeOffset(ip1, sizer(ap, SUBDIM_SIZE));
							decDim(ap);
							insertPtr(ap, mem_type);

							pnp = new Pnode('+');
							addPnode(&mainPcode, pnp);
							if ( ip0->type == INDIR_ITEM ) ip0->type = TEMP_ITEM;
							if ( ip0->type == PID_ITEM )   ip0->type = ID_ITEM;
							if ( ip0->type == DIR_ITEM )   ip0->type = CON_ITEM;
							pnp->items[2] = ip1;
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp();
							pnp->items[0]->attr = ap;
							PUSH(pnp->items[0]->clone());
							
							TOP()->type = INDIR_ITEM;
							return;
						}
						if ( ptrWeight(ip0->attr) > 1 )
						{
							attrib *ap = cloneAttr(attr);
							reducePtr(ap);
							ip1 = makeOffset(ip1, sizer(ap, INDIR_SIZE));

							pnp = new Pnode('+');
							addPnode(&mainPcode, pnp);
							pnp->items[2] = ip1;
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp();
							pnp->items[0]->attr = ap;
							PUSH(pnp->items[0]->clone());

							TOP()->type = INDIR_ITEM;
							return;
						}
					default:
						break;
				}
				delete ip0; delete ip1;
				errPrint("illegal array offset operation!");
			}
			break;

		default:
//			run3(op);
			break;
	}
}

Item *Pcoder :: makeTemp(void)
{
	int i = 0;

	while ( tempIndexMask[i/32] & (1 << (i%32)) )
	{
		if ( ++i >= MAX_TEMP_INDEX )
		{
			errPrint("Temp var out of range!");
			break;
		}
	}
	tempIndexMask[i/32] |= 1 << (i%32);
	return tmpItem(i);
}

Item *Pcoder :: makeTemp(attrib *ap)
{
	Item *ip = makeTemp();
	ip->attr = cloneAttr(ap);
	ip->attr->dataBank = 0;
	ip->attr->isExtern = 0;
	return ip;
}

Item *Pcoder :: makeTemp(int size)
{
	Item *ip = makeTemp();
	int type = (size == 1)? CHAR:
			   (size == 2)? INT	:
			   (size == 3)? SHORT: LONG;
	ip->attr = newAttr(type);
	ip->attr->dataBank = 0;
	ip->attr->isExtern = 0;
	return ip;
}

Item *Pcoder :: makeOffset(Item *ip, int scale)
{
	if ( ip->type == CON_ITEM )
		ip->val.i *= scale;
	else if ( scale > 1 )
	{
		Pnode *pnp = new Pnode('*');
		addPnode(&mainPcode, pnp);
		pnp->items[1] = ip;
		pnp->items[2] = intItem(scale);
		pnp->items[0] = makeTemp();
		pnp->items[0]->attr = newAttr(INT);
		pnp->items[0]->attr->isUnsigned = 1;
		ip = pnp->items[0]->clone();
	}
	return ip;
}