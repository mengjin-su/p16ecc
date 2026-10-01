#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
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
				int n;

				if ( ip0->type == IMMD_ITEM && !ip0->attr->dimVect )
					n = 2;
				else
					n = sizer.size(ip0->attr, TOTAL_SIZE);

				if ( n <= 0 )	// external virtual array
				{
					Dnode *dp = (Dnode*)ip0->home;
					if ( ip0->type == IMMD_ITEM && dp && dp->func == NULL )
					{
						std::string s = ip0->val.s;
						s += "$sizeof$";
						ip0->updateName((char*)s.c_str());
						PUSH(ip0);
						return;
					}
				}

				delete ip0;
				if ( n > 0 )
					PUSH(intItem(n));
				else
					errPrint("can't size the data");
			}
			break;

		case LABEL:
		case GOTO:
			np = OPR_NODE(op, 0);
			pnp = new Pnode(code, lblItem(np->id.name));
			addPnode(&mainPcode, pnp);
			break;

		case NEG_OF:
		case '!':
		case '~':
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = POP();
				attrib *attr = ip0->attr;
				if ( ip0->isOperable() )
					switch ( ip0->type )
					{
						case ID_ITEM:
						case TEMP_ITEM:
						case ACC_ITEM:
							pnp = new Pnode(code);
							addPnode(&mainPcode, pnp);
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp(attr);
							PUSH(pnp->items[0]->clone());
							return;

						case IMMD_ITEM:
						case LBL_ITEM:
							if ( code == '!' )
							{
								delete ip0;
								ip0 = intItem(0);
							}
							else if ( code == '~' )
							{
								std::string s = "(~";
								s += ip0->val.s;	s += ")";
								ip0->updateName((char*)s.c_str());
							}
							else // NEG_OF
							{
								std::string s = "(0-";
								s += ip0->val.s; s += ")";
								ip0->updateName((char*)s.c_str());
							}
							PUSH(ip0);
							return;

						case DIR_ITEM:
						case PID_ITEM:
						case INDIR_ITEM:
							pnp = new Pnode(code);
							addPnode(&mainPcode, pnp);
							pnp->items[1] = ip0;
							pnp->items[0] = makeTemp();
							pnp->items[0]->attr = cloneAttr(attr);
							reducePtr(pnp->items[0]->attr);

							PUSH(pnp->items[0]->clone());
							return;
						default:
							break;
					}
				delete ip0;
			}
			errPrint("can't operate!");
			break;

		case CAST:
			run(OPR_NODE(op, 0));
			if ( DEPTH() == (depth+1) )
			{
				ip0 = POP();
				attrib *ap = op->attr;		// casting attribute

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
			break;
	}
}
