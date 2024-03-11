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

bool shortenOper(int code, int *new_code);

//////////////////////////////////////////////////////////////////////////
bool Optimizer :: case11(Pnode *pnp)
{
	int new_type;

	if ( shortenOper(pnp->type, &new_type) &&
		 same(ip0, ip1) && !same(ip1, ip2) && !same(ip0, ip2) )
	{
		pnp->type = new_type;
		pnp->updateItem(1, ip2);
		pnp->items[2] = NULL;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool Optimizer :: case12(Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);
	Pnode *p2 = next(pnp, 2);
	if ( pnp->type == '=' && ITYPE(ip0, TEMP_ITEM) && p1 && p2 )
	{
		if ( (p1->type == INC_OP || p1->type == DEC_OP) &&
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

//////////////////////////////////////////////////////////////////////////
bool Optimizer :: case13(Pnode *pnp)
{
	bool updated = false;	
	Pnode *p1 = next(pnp, 1);
	if ( pnp->type == '=' &&
		 ITYPE(ip0, TEMP_ITEM) && ITYPE(ip1, ACC_ITEM) &&
		 cmpAttr(ip0->attr, ip1->attr) == 0 && !accReferenced(p1) )
	{
		bool updated = false;
		bool done = false;
		while ( !endOfScope(p1) && !done )
		{
			for (int i = 3; i-- && !done;)
			{
				Item *ip = p1->items[i];
				if ( ip && same(ip0, ip) )
				{
					if ( i > 0 || !assignmentCode(p1->type) )
					{
						ip->type = ACC_ITEM;
						updated = true;
					}
					else
						done = true;	// stop here!
				}
			}
			p1 = p1->next;
		}
		if ( updated ) delete pnp;
	}
	return updated;
}

bool Optimizer :: case14 (Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);
	if ( assignmentCode(pnp->type) && ITYPE(ip0, TEMP_ITEM) &&
		 p1 && PTYPE(p1, '=') && same(ip0, p1->items[1]) 	&&
		 unusedTmp(p1->next, ip0->val.i)					)
	{
		pnp->updateItem(0, p1->items[0]);
		p1->items[0] = NULL;
		delete p1;
		return true;
	}
	return false;
}

bool Optimizer :: accReferenced(Pnode *pnp)
{
	for (; pnp; pnp = pnp->next)
	{
//		for (int i = 0; i < 3; i++)
//		{
//			Item *ip = pnp->items[i];
//			if ( ip == NULL ) continue;
//			if ( ip->type == ACC_ITEM )
//				return true;
//		}
		if ( pnp->type == CALL || pnp->type == P_CALL )
			return true;
		
		if ( endOfScope(pnp) )
			break;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////
bool shortenOper(int code, int *new_code)
{
	switch ( code )
	{
		case '+':	*new_code = ADD_ASSIGN;	return true;
		case '-':	*new_code = SUB_ASSIGN;	return true;
		case '*':	*new_code = MUL_ASSIGN;	return true;
		case '/':	*new_code = DIV_ASSIGN;	return true;
		case '%':	*new_code = MOD_ASSIGN;	return true;
		case '&':	*new_code = AND_ASSIGN;	return true;
		case '|':	*new_code = OR_ASSIGN;	return true;
		case '^':	*new_code = XOR_ASSIGN;	return true;
	}
	return false;
}