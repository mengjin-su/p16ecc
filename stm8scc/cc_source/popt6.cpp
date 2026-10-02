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
#include "popt.h"
#include "sizer.h"

#define ID_TO_TEMP(p)	(p->items[0] && p->items[1] && p->items[1]->attr && \
						 p->items[0]->type == TEMP_ITEM && \
						 p->items[1]->type == ID_ITEM && p->items[1]->attr->ptrVect && \
						 (p->type == '=' || (p->type == '+' && p->items[2]->type == CON_ITEM)))
#define DIR_TO_TEMP(p)	(p->items[0] && p->items[1] && p->items[1]->attr && \
						 p->items[0]->type == TEMP_ITEM && \
						 p->items[1]->type == DIR_ITEM && !(p->items[1]->val.i & ~0xffff) && \
						 (p->type == '=' || (p->type == '+' && p->items[2]->type == CON_ITEM)))

static bool exchaneableOpr(int old_opr, int *new_opr);

bool Optimizer :: case22(Pnode *pnp)
{
	int new_opr;
	if ( exchaneableOpr(pnp->type, &new_opr) &&
		 !equivalent(ip0, ip1) && equivalent(ip0, ip2) )
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
	if ( (PTYPE(pnp, P_DEC) || PTYPE(pnp, P_INC)) && PTYPE(p1, P_JNZ) )
	{
		Item *ip = p1->items[0];
		if ( ip0->acceSize() == 1 && ip1->val.i == 1 && same(ip0, ip) )
		{
			pnp->type = PTYPE(pnp, P_DEC)? P_DJNZ: P_IJNZ;
			pnp->updateItem(1, p1->items[1]);
			p1->items[1] = NULL;
			delete p1;
			return true;
		}
	}
	return false;
}

bool Optimizer :: case24(Pnode *pnp)
{
	int type = pnp->type;
	if ( ip1 && ip1->type == CON_ITEM && ip2 && ip2->type == CON_ITEM )
		switch ( type )
		{
			case '+':
			case '-':
				ip1->val.i += (type == '+')? ip2->val.i: -ip2->val.i;
				pnp->type = '=';
				pnp->updateItem(2, NULL);
				return true;
		}

	if ( equivalent(ip0, ip1) && pnp->type == '+' && ip2->type == CON_ITEM )
	{
		pnp->type = P_INC;
		pnp->updateItem(1, ip2);
		pnp->items[2] = NULL;
		return true;
	}
	Pnode *pnp1 = next(pnp, 1);
	if ( pnp1 && pnp1->type == P_INC &&
		 type == '=' && ip1->type == CON_ITEM &&
		 equivalent(ip0, pnp1->items[0]) 		)
	{
		pnp1->type = '=';
		pnp1->items[1]->val.i += ip1->val.i;
		delete pnp;
		return true;
	}
	if ( ID_TO_TEMP(pnp) || DIR_TO_TEMP(pnp) )
	{
		Item *ip = ip1->clone();
		ip->type = PID_ITEM;
		if ( type == '+' ) ip->bias += ip2->val.i;
		if ( DIR_TO_TEMP(pnp) ) {
			char *s = new char[10];
			sprintf(s, "%d", ip->val.i);
			ip->val.s = s;
		}

		bool ret_code = replaceIndir(pnp->next, ip, ip0->val.i);
		if ( ret_code ) delete pnp;
		delete ip;
		return ret_code;
	}
	return false;
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

				case ID_ITEM:
					if ( related(ip, iptr) )
						return false;
					break;

				default:
					break;
			}
		}
	}
	return ret_code;
}
