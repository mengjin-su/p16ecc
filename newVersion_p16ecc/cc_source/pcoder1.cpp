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
#include "dlink.h"
#include "flink.h"
#include "sizer.h"
#include "pcoder.h"

void Pcoder :: run1(oprNode_t *op)
{
	attrib *ap = op->attr;
	char *fname;
	node *np;
	Dlink *dlk;

	switch ( op->oper )
	{
		case ';':
			addPnode(&mainPcode, new Pnode(';'));
			CLEAR();
			// clear Temp Index mask
			memset(&tempIndexMask, 0, sizeof(tempIndexMask));
			break;

		case '{':
			dlk = new Dlink(VAR_DLINK);
			curDlink->addChild(dlk);
			curDlink = dlk;
			run(op->op[0]) ;
			curDlink = dlk->parent;
			break;

		case DATA_DECL:
			if ( op->nops > 0 && op->op[0] )
			{
				node *np = op->op[0];	// variables id node
				attrib *ap = op->attr;
				bool data_def = ap->isExtern? false: true;
				int  mem_addr = -1;

				if ( ap->atAddr )
				{
					run((node*)ap->atAddr);
					Item *ip = POP();
					if ( ip && ip->type == CON_ITEM ) mem_addr = ip->val.i;
				}

				char *dname = np->id.name;	// data name
				Dnode *dp = curDlink->search(dname, LOCAL_SEARCH);

				if ( dp ) //
				{
					if ( cmpAttr(dp->attr, op->attr) )
					{
						errPrint("data type conflict!", dname);
						break;
					}

					if ( !dp->dimCheck(np->id.dim) )
					{
						errPrint("data dimension error!", dname);
						break;
					}

					if ( !dp->fptrCheck(np) )
					{
						errPrint("func pointer conflicts!", dname);
						break;
					}

					if ( data_def ) // data definition?
					{
						if ( dp->attr->isExtern )
							dp->attr->isExtern = 0;
						else
						{
							errPrint("data redefined!", dname);
							break;
						}
					}
				}
				else if ( funcLink && funcLink->search(dname) )
				{
					errPrint("name redefined!", dname);
					break;
				}

				if ( dp == NULL )
				{
					if ( curFnode )
						dp = curDlink->add(np, ap, ++curFnode->dIndex);
					else
						dp = curDlink->add(np, ap, 0);

					dp->atAddr = mem_addr;
				}

				dp->func = curFnode? curFnode->name: NULL;

				if ( data_def )
				{
					dp->dimUpdate((node*)np->id.dim);
					
					if ( np->id.init )	// data initialization ...
					{
						if ( dp->func && !ap->isStatic && ap->dataBank != CONST )
						{
							int depth = DEPTH();
							run(np->id.init);
							if ( DEPTH() == (depth+1) )
							{
								Item *ip = idItem(dp->nameStr(), dp->attr);
								ip->home = dp;
								Pnode *pnp = new Pnode('=', ip);
								pnp->items[1] = POP();
								addPnode(&mainPcode, pnp);
							}
						}
						else if ( ap->dataBank == CONST )
							romDataInit(dname, dp->attr, np->id.init, dp, mem_addr);
						else
							ramDataInit(dname, dp->attr, np->id.init, dp, mem_addr);
					}
					else if ( ap->dataBank == CONST )
					{
						errPrint("uninitialized constant item!");
					}
					else if ( dp->attr->dimVect )	// array defined
					{
						int *v = dp->attr->dimVect;
						for (int i = 0; i < v[0]; i++) {
							if ( v[i+1] <= 0 ) {
								errPrint("unknown array dimension!");
								break;
							}
						}
					}
				}
			}
			break;

		case FUNC_DECL:
			np = op->op[0];	// get function head
			fname = np->opr.op[0]->id.name;

			if ( curDlink->search(fname, LOCAL_SEARCH) )
				errPrint("name redefined!", fname);		// name used as data!
			else
			{
				bool func_def = (op->op[1] != NULL);	// func body exitst?
				bool err = false;

				Fnode *fp = funcLink->search(fname);	// function existed already?
				if ( fp == NULL )	// No, it's first time ...
				{
					fp = new Fnode(fname);				// create Fnode
					fp->attr = cloneAttr(ap);			// copy it's (return) type.
					funcLink->add(fp);					// add Fnode to Flink

					if ( np->opr.nops > 1 ) { 			// with parameter?
						node *pp = np->opr.op[1]; 		// parameter list
						fp->elipsis = pp->list.elipsis;
						for (int i = 0; i < pp->list.nops; i++)
						{
							node *parp = pp->list.ptr[i];

							if ( func_def && fp->dlink->search(parp->id.name, LOCAL_SEARCH) )
								errPrint("parameter name duplicatated!", NULL), err = true;

							Dnode *dp = fp->dlink->add(parp, parp->id.attr, 0);
							dp->func = fp->name;
						}
					}
				}
				else if ( fp->endLbl > 0 && func_def )
				{
					errPrint("function redefined!", fname), err = true;
				}
				else	// function has been declaired/defined - need confirming
				{
					if ( (np->opr.nops <= 1 && fp->dlink->dataCount() != 0) ||
						 (np->opr.nops >  1 && fp->dlink->dataCount() == 0) ||
						 (np->opr.nops >  1 && np->opr.op[1]->list.nops != fp->dlink->dataCount()) )
					{
						 errPrint("function parameter count conflict!", fname), err = true;
					}
					else if ( np->opr.nops > 1 )
					{
						np = np->opr.op[1];	// get parameter list
						for (int i = 0; !err && i < np->list.nops; i++)
						{
							Dnode *dp = fp->dlink->get(i);
							node  *pp = np->list.ptr[i];

							if ( cmpAttr(dp->attr, pp->id.attr) )
								errPrint("parameter type confilict!", fname), err = true;
							else if ( dp->fptrCheck(pp) != true )
								errPrint("parameter type confilict!", fname), err = true;
							else if ( func_def )
							{
								if ( !fp->dlink->nameCheck(pp->id.name, i) )
									errPrint("par name duplicated!", fname), err = true;
								else
									dp->nameUpdate(pp->id.name);
							}
						}
					}
				}

				if ( func_def && !err )
				{
					curDlink->addChild(fp->dlink);	// add par dlink to parent
					curDlink = fp->dlink;		// update current Dlink
					curFnode = fp;

					addPnode(&mainPcode, new Pnode(P_FUNC_BEG, ptrItem(fp)));
					fp->endLbl = getLbl();		// set end label for function

					run(op->op[1]);				// scan function body

					addPnode(&mainPcode, new Pnode(LABEL, lblItem(fp->endLbl)));
					addPnode(&mainPcode, new Pnode(P_FUNC_END, ptrItem(fp)));
					curFnode = NULL;
					curDlink = fp->dlink->parent;// recover current Dlink
				}
			}
			break;

		default:
			run2(op);
			break;
	}
}
