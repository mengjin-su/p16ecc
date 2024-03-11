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

static bool exchaneableOpr(int old_opr, int *new_opr);						 

bool Optimizer :: case22(Pnode *pnp)
{
	int new_opr;
	if ( exchaneableOpr(pnp->type, &new_opr) && same(ip0, ip2) )
	{
		pnp->type = new_opr;
		pnp->updateItem(2, NULL);
		return true;
	}
	return false;
}

bool Optimizer :: case23(Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);
	if ( (PTYPE(pnp, DEC_OP) || PTYPE(pnp, INC_OP)) && PTYPE(p1, P_JNZ) )
	{
		Item *ip = p1->items[0];
		if ( ip0->acceSize() == 1 && ip1->val.i == 1 && same(ip0, ip) )
		{
			pnp->type = PTYPE(pnp, DEC_OP)? P_DJNZ: P_IJNZ;
			pnp->updateItem(1, p1->items[1]);
			p1->items[1] = NULL;
			delete p1;
			return true;
		}
	}

	if ( PTYPE(pnp, '=') && PTYPE(p1, '=') &&
		 ip0->type == TEMP_ITEM && ip0->attr->ptrVect && ip1->type == ID_ITEM &&
		 p1->items[1]->type == INDIR_ITEM && ip0->val.i == p1->items[1]->val.i )
	{
		ip1->type = PID_ITEM;
		ip1->updateAttr(p1->items[1]->attr);	p1->items[1]->attr = NULL;
		pnp->updateItem(0, p1->items[0]);		p1->items[0] = NULL;
		delete p1;
		return true;
	}
	return false;
}

bool Optimizer :: case24(Pnode *pnp)
{
	if ( ip0 && ip0->type == TEMP_ITEM && 
		 ip1 && ip1->type == ID_ITEM && ip1->attr && ip1->attr->ptrVect &&
		 (PTYPE(pnp, '=') || (PTYPE(pnp, '+') && ip2->type == CON_ITEM)) )
	{
		Item *ip = ip1->clone();
		if ( PTYPE(pnp, '+') ) ip->bias += ip2->val.i;		

		ip->type = PID_ITEM;
		bool ret_code = replaceIndir(pnp->next, ip, ip0->val.i);
		if ( ret_code ) delete pnp;
		delete ip;
		return ret_code;		
	}			 

	if ( ip0 && ip0->type == TEMP_ITEM && 
		 ip1 && ip1->type == CON_ITEM  && ip1->attr && ip1->attr->ptrVect &&
		 (PTYPE(pnp, '=') || (PTYPE(pnp, '+') && ip2->type == CON_ITEM)) 	)
	{
		Item *ip = ip1->clone();
		if ( PTYPE(pnp, '+') ) ip->val.i += ip2->val.i;

		ip->type = DIR_ITEM;
		bool ret_code = replaceIndir(pnp->next, ip, ip0->val.i);
		if ( ret_code ) delete pnp;
		delete ip;
		return ret_code;
	}
	return false;
}

bool Optimizer :: replaceIndir(Pnode *pnp, Item *ip, int temp_index)
{
	bool ret_code = false;
	for (; !endOfScope(pnp); pnp = pnp->next)
	{
		for (int i = 3; i--;)
		{
			Item *iptr = pnp->items[i];
			if ( iptr == NULL ) continue;
			switch ( iptr->type )
			{
				case TEMP_ITEM:
					if ( iptr->val.i == temp_index )
						return ret_code;
					break;

				case INDIR_ITEM:
					if ( iptr->val.i == temp_index )
					{
						attrib *ap = cloneAttr(iptr->attr);
						ap->dataBank = ip->attr->dataBank;
						ap->isVolatile = ip->attr->isVolatile;
						pnp->updateItem(i, ip->clone());
						pnp->items[i]->updateAttr(ap);
						ret_code = true;
					}
					break;

				default:
					break;
			}
		}
	}
	return ret_code;
}

static bool exchaneableOpr(int old_opr, int *new_opr)
{
	switch ( old_opr )
	{
		case '+':	*new_opr = ADD_ASSIGN;	return true;
		case '*':	*new_opr = MUL_ASSIGN;	return true;
		case '&':	*new_opr = AND_ASSIGN;	return true;
		case '|':	*new_opr = OR_ASSIGN;	return true;
		case '^':	*new_opr = XOR_ASSIGN;	return true;
	}
	return false;
}