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

static bool shortenOper(int code, int *new_code);

bool Optimizer :: group2(Pnode *pnp)
{
	int ptype = pnp->type;

	if ( assignmentCode(ptype, MATH_OP | LOGIC_OP) &&					// {'+', [X, #Y, N]}
		 ip1->type == IMMD_ITEM && ip2->type == CON_ITEM )
	{
		if ( ip2->val.i != 0 )
		{
			char buf[32];
			sprintf(buf, "%c%d)", ptype, ip2->val.i);
			std::string str = ip1->val.s;
			str.insert(0, "(");	str += buf;
			pnp->updateName(1, (char*)str.c_str());
		}
		pnp->type = '=';
		pnp->updateItem(2, NULL);
		return true;
	}

	if ( ptype == '=' &&
		 ip0->type == TEMP_ITEM && ip1->type == IMMD_ITEM )				// {'=', [t, #X]}
	{
		int i;
		Pnode *p1 = indirReferenced(pnp->next, ip0, &i);
		if ( p1 )														// {op, [.. (t) ..]}
		{
			if ( p1->items[i]->type == INDIR_ITEM )
			{
				attrib *attr = cloneAttr(p1->items[i]->attr);
				reducePtr(attr);
				attr->dataBank = ip1->attr->dataBank;
				ip1->updateAttr(attr);
				ip1->type = ID_ITEM;
			}
			p1->updateItem(i, ip1);
			pnp->items[1] = NULL;
			delete pnp;
			return true;
		}
	}

	int new_type;
	if ( shortenOper(ptype, &new_type) &&								// {'+', [X, X, Y]}
		 same(ip0, ip1) && !same(ip1, ip2) && !same(ip0, ip2) )
	{
		pnp->type = new_type;
		pnp->updateItem(1, ip2);
		pnp->items[2] = NULL;
		return true;
	}

	Pnode *p1 = next(pnp, 1);

	if ( assignmentCode(ptype, ALL_OP) && ip0->type == TEMP_ITEM &&		// {'+', [t, x, y]}
		 p1 && p1->type == '=' && same(ip0, p1->items[1]) 		 &&		// {'=', [z, t]}
		 unusedTmp(p1->next, ip0->val.i)						 )
	{
		pnp->updateItem(0, p1->items[0]);
		p1->items[0] = NULL;
		delete p1;
		return true;
	}

	Pnode *p2 = next(pnp, 2);
	if ( ptype == '=' && ip0->type == TEMP_ITEM && p1 && p2 &&			// {'=', [t, x]}
		 (p1->type == INC_OP || p1->type == DEC_OP)     	&&			// {INC_OP, [x, N]}
		 same(ip1, p1->items[0]) 			  		  		&&			// {'=', [y, t]}
		 p2->type == '=' && related(ip0, p2->items[1]) 		&&
		 				   !related(ip1, p2->items[0]) 		&&
		 unusedTmp(p2->next, ip0->val.i) 					)
	{
		pnp->next= p2;
		p2->last = pnp;
		p1->next = p2->next;
		p1->last = p2;
		p2->next = p1;
		return true;
	}

	if ( ptype == '+' && ip0->type == TEMP_ITEM && ip2->type == CON_ITEM )	// {'+', [t, x, N]}
	{																		// {'+', [y, t, M]}
		if ( p1 && p1->type == '+' &&
			 same(ip0, p1->items[1]) && p1->items[2]->type == CON_ITEM &&
			 (related(p1->items[0], ip0) || unusedTmp(p1->next, ip0->val.i)) )
		{
			p1->updateItem(1, ip1);
			p1->items[2]->val.i += ip2->val.i;
			pnp->items[1] = NULL;
			delete pnp;
			return true;
		}
	}

	if ( (ptype == ADD_ASSIGN || ptype == SUB_ASSIGN) && ip1->type == CON_ITEM )	// {'+=', [X, N]}
	{
		pnp->type = (ptype == ADD_ASSIGN)? INC_OP: DEC_OP;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////
static bool shortenOper(int code, int *new_code)
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