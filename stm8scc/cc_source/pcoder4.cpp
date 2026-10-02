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
#include "option.h"

void Pcoder :: run4(oprNode_t *op)
{
	int depth = DEPTH();
	Pnode *pnp;
	Item *ip0;
	int code;
	node *np;

	switch ( code = OPR_TYPE(op) )
	{
		case SIZEOF:
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = POP();
				attrib *attr = ip0->attr;
				int n;
				switch ( ip0->type )
				{
					case ID_ITEM:
					case IMMD_ITEM:
					case TEMP_ITEM:
						n = sizer.size(attr, TOTAL_SIZE);
						if ( n <= 0 )
						{	// must be external virtual array specifier ...
							// (query owner function)
							Dnode *dp = (Dnode*)ip0->home;
							if ( ip0->type == IMMD_ITEM &&
								 dp && dp->func == NULL /* owner = NONE */)
							{
								char s[1024];
								sprintf(s, "%s$sizeof$", ip0->val.s);
								ip0->updateName(s);
								PUSH(ip0);
								return;
							}
							break;
						}
						PUSH(intItem(n));
						return;

					case DIR_ITEM:
					case INDIR_ITEM:
					case PID_ITEM:
						n = sizer.size(attr, INDIR_SIZE);
						PUSH(intItem(n));
						return;

					default:
						break;
				}

				delete ip0;
				errPrint("can't size the data");
			}
			break;

		case LABEL:
		case GOTO:
			np = OPR_NODE(op, 0);
			pnp = new Pnode(code, lblItem(np->id.name));
			addPnode (&mainPcode, pnp);
			break;

		case NEG_OF:
		case '!':
		case '~':
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = POP();
				attrib *attr = ip0->attr;
				switch ( ip0->type )
				{
					case CON_ITEM:
						switch ( code )
						{
							case NEG_OF:
								ip0->val.i *= -1;
								ip0->attr->isNeg ^= 1;
								break;
							case '!':
								ip0->val.i = ip0->val.i? 0: 1;
								break;
							case '~':
								ip0->val.i ^= -1;
								break;
						}
						PUSH(ip0);
						return;

					case ID_ITEM:
					case TEMP_ITEM:
					case ACC_ITEM:
						if ( ip0->isOperable() )
						{
							pnp = new Pnode(code);
							addPnode(&mainPcode, pnp);
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp(attr);
							PUSH(pnp->items[0]->clone());
							return;
						}
						break;

					case IMMD_ITEM:
					case LBL_ITEM:
						if ( code == '!' )
						{
							ip0 = intItem(0);
							PUSH(ip0);
						}
						else
						{
							int size = 2;
							if ( EXT_MODE &&
								 (ip0->type == LBL_ITEM      ||
								  ip0->attr->dataBank == CONST) ) size = 3;

							pnp = new Pnode(code);
							addPnode(&mainPcode, pnp);
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp(size);
							if ( code == '~' ) pnp->items[0]->attr->isUnsigned = 1;
							PUSH(pnp->items[0]->clone());
						}
						return;

					case DIR_ITEM:
					case PID_ITEM:
					case INDIR_ITEM:
						if ( ip0->isOperable() )
						{
							pnp = new Pnode(code);
							addPnode(&mainPcode, pnp);
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp();
							pnp->items[0]->attr = cloneAttr(attr);
							reducePtr(pnp->items[0]->attr);

							PUSH(pnp->items[0]->clone());
							return;
						}
					default:
						break;
				}
				delete ip0;
				errPrint("can't operate!");
			}
			break;

		case CAST:
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = POP();
				attrib *ap = op->attr;		// casting attribute

				if ( !(ap->ptrVect ||
					   ap->type == CHAR  || ap->type == INT ||
					   ap->type == SHORT || ap->type == LONG) )
				{
					delete ip0;
					errPrint("improper casting type!");
					return;
				}
				if ( !ip0->isOperable() )
				{
					delete ip0;
					errPrint("can't be casted!");
					return;
				}
				if ( ip0->type == CON_ITEM )
				{
					ip0->updateAttr(cloneAttr(ap));
					PUSH(ip0);
					return;
				}
				pnp = new Pnode('=');
				addPnode(&mainPcode, pnp);
				pnp->items[1] = ip0;
				pnp->items[0] = makeTemp(ap);
				PUSH(pnp->items[0]->clone());
				return;
			}
			break;

		default:
			run5(op);
	}
}
