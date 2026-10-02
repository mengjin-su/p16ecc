#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "item.h"
#include "pnode.h"
#include "pcoder.h"
#include "pcoder.h"

void Pcoder :: run9(oprNode_t *op)
{
	int depth = DEPTH();
	int code  = OPR_TYPE(op);	
	Pnode *pnp;
	
	switch ( code )
	{
		case PRAGMA:
			pnp = new Pnode(PRAGMA);
			addPnode(&mainPcode, pnp);
			pnp->items[0] = idItem(op->op[0]->id.name);
			if ( op->nops > 1 )
			{
				run(op->op[1]);
				if ( DEPTH() == (depth+1) )
					pnp->items[1] = POP();
				else
					errPrint("unknown item in pragma!");
			}
			break;
			
		case AASM:
			pnp = new Pnode(AASM);
			addPnode(&mainPcode, pnp);
			pnp->items[0] = strItem(op->op[0]->str.str);
			break;
	}
}