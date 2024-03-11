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

static bool convertJump(int code, int *rev_code, int *par_count);
static bool swapJump(int code, int *rev_code);

bool Optimizer :: group3(Pnode *pnp)
{
	int ptype = pnp->type;
	Pnode *p1 = next(pnp, 1);
	Pnode *p2 = next(pnp, 2);
	int rev_type, n;

	if ( convertJump(ptype, &rev_type, &n) && p1 && p2 )		// {J_Z, [X, Lx]}
	{															// {GOTO, [Ly]}
		if ( p1->type == GOTO && p2->type == LABEL &&			// {LABEL, [Lx]}
			 same(pnp->items[n-1], p2->items[0]) )
		{
			pnp->type = rev_type;
			pnp->updateName(n-1, p1->items[0]->val.s);
			delete p1;
			return true;
		}
	}

	if ( ptype == '&' && ip0->type == TEMP_ITEM && p1 &&		// {'&', [t, X, 2^N]}
		 bitSelect(ip2, &n)				     	 	  &&		// {J_Z, [t, Lx]}
		 (p1->type == P_JZ || p1->type == P_JNZ) 	  &&
		 same(ip0, p1->items[0])					  &&
		 unusedTmp(p1->next, ip0->val.i)	 		  )
	{
		p1->type = (p1->type == P_JZ)? P_JBZ: P_JBNZ;
		p1->updateItem(0, ip1);
		p1->items[2] = p1->items[1];
		p1->items[1] = intItem(n);
		pnp->items[1] = NULL;
		delete pnp;
		return true;
	}

	if ( (ptype == P_JEQ || ptype == P_JNE) &&					// {P_JEQ, [X, 0, Lx]}
		 ip1->type == CON_ITEM && ip1->val.i == 0 )
	{
		pnp->type = (ptype == P_JEQ)? P_JZ: P_JNZ;
		pnp->updateItem(1, ip2);
		pnp->items[2] = NULL;
		return true;
	}

	int swap_type;
	if ( swapJump(pnp->type, &swap_type) &&						// {P_JEQ, [N, X, Lx]}
		 ip0->type == CON_ITEM && ip1->type != CON_ITEM )
	{
		pnp->type = swap_type;
		pnp->items[0] = ip1;
		pnp->items[1] = ip0;
		return true;
	}

	if ( ptype == '=' && ip0->type == TEMP_ITEM && p1 && p2 )	// {'=', [t, X]}
	{															// {P_INC, [X, N]}
		if ( (p1->type == INC_OP || p1->type == DEC_OP) &&		// {P_JZ, [t, Lx]}
			 same(ip1, p1->items[0]) 				    &&
			 (p2->type == P_JZ   || p2->type == P_JNZ)  &&
			 same(ip0, p2->items[0])				    &&
			 unusedTmp(p2->next, ip0->val.i)			 )
		{
			if ( p2->type == P_JZ )
				p2->type = (p1->type == INC_OP)? P_JZ_INC:  P_JZ_DEC;
			else
				p2->type = (p1->type == INC_OP)? P_JNZ_INC: P_JNZ_DEC;

			p2->items[2] = p2->items[1];
			p2->items[1] = p1->items[1];
			p2->updateItem(0, p1->items[0]);
			p1->items[0] = p1->items[1] = NULL;
			delete pnp;
			delete p1;
			return true;
		}
	}

	return false;
}

static bool convertJump(int code, int *rev_code, int *par_count)
{
	switch ( code )
	{
		case P_JZ:	*rev_code = P_JNZ;	*par_count = 2;	return true;
		case P_JNZ:	*rev_code = P_JZ;	*par_count = 2;	return true;
		case P_JEQ:	*rev_code = P_JNE;	*par_count = 3;	return true;
		case P_JNE:	*rev_code = P_JEQ;	*par_count = 3;	return true;
		case P_JGT:	*rev_code = P_JLE;	*par_count = 3;	return true;
		case P_JGE:	*rev_code = P_JLT;	*par_count = 3;	return true;
		case P_JLT:	*rev_code = P_JGE;	*par_count = 3;	return true;
		case P_JLE:	*rev_code = P_JGT;	*par_count = 3;	return true;
		case P_JBZ:	*rev_code = P_JBNZ;	*par_count = 3;	return true;
		case P_JBNZ:*rev_code = P_JBZ;	*par_count = 3;	return true;
	}
	return false;
}

static bool swapJump(int code, int *rev_code)
{
	switch ( code )
	{
		case P_JEQ:	*rev_code = code;	return true;
		case P_JNE:	*rev_code = code;	return true;
		case P_JGT:	*rev_code = P_JLT;	return true;
		case P_JGE:	*rev_code = P_JLE;	return true;
		case P_JLT:	*rev_code = P_JGT;	return true;
		case P_JLE:	*rev_code = P_JGE;	return true;
	}
	return false;
}
