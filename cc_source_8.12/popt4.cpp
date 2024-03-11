#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "item.h"
#include "pnode.h"
#include "pcoder.h"
#include "pcoder.h"
#include "popt.h"
#include "sizer.h"

#define CONST0(ip)	(ip && ITYPE(ip, CON_ITEM) && ip->val.i == 0)
#define CONST1(ip)	(ip && ITYPE(ip, CON_ITEM) && ip->val.i == 1)

bool Optimizer :: case15(Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);
	Pnode *p2 = next(pnp, 2);
	if ( PTYPE(pnp, '=') && ITYPE(ip0, TEMP_ITEM) 	  &&
		 (PTYPE(p1, INC_OP) || PTYPE(p1, DEC_OP))     &&
		 same(ip1, p1->items[0]) 			  		  &&
		 PTYPE(p2, '=') && related(ip0, p2->items[1]) &&
		 				  !related(ip1, p2->items[0]) &&
		 unusedTmp(p2->next, ip0->val.i) 				)
	{
		pnp->next = p2;
		p2->last = pnp;
		p1->next = p2->next;
		p1->last = p2;
		p2->next = p1;
		return true;
	}
	return false;
}

bool Optimizer :: case16(Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);

	if ( PTYPE(pnp, '+') && ITYPE(ip0, TEMP_ITEM) && ITYPE(ip2, CON_ITEM) )
	{
		if ( p1 && PTYPE(p1, '+') &&
			 same(ip0, p1->items[1]) && ITYPE(p1->items[2], CON_ITEM) &&
			 (related(p1->items[0], ip0) || unusedTmp(p1->next, ip0->val.i)) )
		{
			p1->updateItem(1, ip1);
			p1->items[2]->val.i += ip2->val.i;
			pnp->items[1] = NULL;
			delete pnp;
			return true;
		}
	}
	return false;
}

bool Optimizer :: case17(Pnode *pnp)
{
	if ( PTYPE(pnp, '+') || PTYPE(pnp, '-') )
	{
		if ( CONST0(ip2) )
		{
			pnp->type = '=';
			pnp->updateItem(2, NULL);
			return true;
		}
		if ( CONST0(ip1) )
		{
			pnp->type = PTYPE(pnp, '+')? '=': NEG_OF;
			pnp->updateItem(1, ip2);
			pnp->items[2] = NULL;
			return true;
		}
	}
	if ( (PTYPE(pnp, '*') || PTYPE(pnp, '&')) && (CONST0(ip1) || CONST0(ip2)) )
	{
		pnp->type = '=';
		if ( CONST0(ip1) )
			pnp->updateItem(2, NULL);
		else
		{
			pnp->updateItem(1, ip2);
			pnp->items[2] = NULL;
		}
		return true;
	}
	if ( (PTYPE(pnp, '|') || PTYPE(pnp, '^')) && (CONST0(ip1) || CONST0(ip2)) )
	{
		pnp->type = '=';
		if ( CONST0(ip2) )
			pnp->updateItem(2, NULL);
		else
		{
			pnp->updateItem(1, ip2);
			pnp->items[2] = NULL;
		}
		return true;
	}
	if ( (PTYPE(pnp, '/') || PTYPE(pnp, '%')) && CONST0(ip1) )
	{
		pnp->type = '=';
		pnp->updateItem(2, NULL);
		return true;
	}
	if ( (PTYPE(pnp, ADD_ASSIGN) || PTYPE(pnp, SUB_ASSIGN)  ||
		  PTYPE(pnp, OR_ASSIGN)  || PTYPE(pnp, XOR_ASSIGN)  ||
		  PTYPE(pnp, INC_OP)  	 || PTYPE(pnp, DEC_OP)	    ||
		  PTYPE(pnp, LEFT_ASSIGN)|| PTYPE(pnp, RIGHT_ASSIGN)) && CONST0(ip1) )
	{
		delete pnp;
		return true;
	}
	if ( (PTYPE(pnp, MUL_ASSIGN) || PTYPE(pnp, AND_ASSIGN)) && CONST0(ip1) )
	{
		pnp->type = '=';
		return true;
	}
	if ( (PTYPE(pnp, MUL_ASSIGN) || PTYPE(pnp, DIV_ASSIGN)) && CONST1(ip1) )
	{
		delete pnp;
		return true;
	}
	if ( (PTYPE(pnp, ADD_ASSIGN) || PTYPE(pnp, SUB_ASSIGN)) && ITYPE(ip1, CON_ITEM) )
	{
		pnp->type = PTYPE(pnp, ADD_ASSIGN)? INC_OP: DEC_OP;
		return true;
	}
	return false;
}

bool Optimizer :: case18(Pnode *pnp)
{
	int n;
	if ( PTYPE(pnp, '*') && bitSelect(ip2, &n) )
	{
		pnp->type = LEFT_OP;
		ip2->val.i = n;
		return true;
	}
	if ( PTYPE(pnp, MUL_ASSIGN) && bitSelect(ip1, &n) )
	{
		pnp->type = LEFT_ASSIGN;
		ip1->val.i = n;
		return true;
	}
	if ( PTYPE(pnp, '/') && bitSelect(ip2, &n) )
	{
		pnp->type = RIGHT_OP;
		ip2->val.i = n;
		return true;
	}
	if ( PTYPE(pnp, DIV_ASSIGN) && bitSelect(ip1, &n) )
	{
		pnp->type = RIGHT_ASSIGN;
		ip1->val.i = n;
		return true;
	}
	if ( PTYPE(pnp, '%') && CONST1(ip2) )
	{
		pnp->type = '=';
		ip2->val.i = 0;
		pnp->updateItem(1, ip2);
		pnp->items[2] = NULL;
		return true;
	}
	if ( PTYPE(pnp, '%') && !ip1->acceSign() && bitSelect(ip2, &n) )
	{
		pnp->type = '&';
		ip2->val.i = (1 << n) - 1;
		return true;
	}
	if ( PTYPE(pnp, MOD_ASSIGN) && !ip0->acceSign() && bitSelect(ip1, &n) )
	{
		pnp->type = AND_ASSIGN;
		ip1->val.i = (1 << n) - 1;
		return true;
	}
	if ( PTYPE(pnp, '*') && (CONST1(ip1) || CONST1(ip2)) )
	{
		pnp->type = '=';
		if ( CONST1(ip1) )
		{
			pnp->updateItem(1, ip2);
			pnp->items[2] = NULL;
		}
		else
			pnp->updateItem(2, NULL);
		return true;
	}
	if ( PTYPE(pnp, '/') && CONST1(ip2) )
	{
		pnp->type = '=';
		pnp->updateItem(2, NULL);
		return true;
	}
	if ( (PTYPE(pnp, MUL_ASSIGN) || PTYPE(pnp, DIV_ASSIGN)) && CONST1(ip1) )
	{
		delete pnp;
		return true;
	}
	return false;
}
