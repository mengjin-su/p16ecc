#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "item.h"
#include "pnode.h"
#include "pcoder.h"
#include "pcoder.h"
#include "popt.h"
#include "display.h"

bool Optimizer :: group1(Pnode *pnp)
{
	int ptype = pnp->type;
	Pnode *p1 = next(pnp, 1);

	if ( ptype == LABEL && p1 && p1->type == LABEL )			// {LABEL, [Lx]}
	{															// {LABEL, [Ly]}
		updateLbl(p1->items[0]->val.s, ip0->val.s, p1);
		delete p1;
		return true;
	}

	if ( ptype == LABEL && unusedLbl(head, ip0->val.s, pnp) )	// {LABEL, [Lx]}
	{
		delete pnp;
		return true;
	}

	if ( ptype == GOTO && p1 && p1->type == LABEL &&			// {GOTO, [Lx]}
		 !strcmp(ip0->val.s, p1->items[0]->val.s) )				// {LABEL, [Lx]}
	{
		delete pnp;
		return true;
	}

	if ( ptype == LABEL && p1 && p1->type == GOTO 	  &&		// {LABEL, [Lx]}
		 strcmp(ip0->val.s, p1->items[0]->val.s) != 0 &&		// {GOTO, [Ly]}
		 updateGoto(ip0->val.s, p1->items[0]->val.s, p1) )
	{	// make all 'jump to LABELx' to 'jump to LABELy'
		return true;
	}

	if ( ptype == GOTO && p1 && !(p1->type == LABEL    ||		// {GOTO, [Lx]}
								  p1->type == P_FUNC_END) )		// {...}
	{
		delete p1;
		return true;
	}

	// following optimizations have some risk!
	if ( ptype == RETURN && p1 && p1->type == GOTO )			// {RETURN}
	{	// remove useless GOTO									// {GOTO, [Lx]}
		delete p1;
		return true;
	}
/*
	if ( ptype == '=' && ip0->type == TEMP_ITEM )				// {'=', [t, X]}
	{
		if ( unusedTmp(pnp->next, ip0->val.i) )
		{
			delete pnp;
			return true;
		}
	}
*/
	return false;
}