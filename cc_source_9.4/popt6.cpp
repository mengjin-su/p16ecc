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

static bool exchaneableOpr(int old_type, int *new_type);

bool Optimizer :: group6(Pnode *pnp)
{
	int ptype = pnp->type;
	int new_type;
	Pnode *p1 = next(pnp, 1);	
	
	if ( exchaneableOpr(ptype, &new_type) && same(ip0, ip2) )					// {'+', [X, Y, X]}
	{
		pnp->type = new_type;
		pnp->updateItem(2, NULL);
		return true;
	}

	if ( ptype == '=' &&														// {'=', [t, ACC]}
		 ip0->type == TEMP_ITEM && ip1->type == ACC_ITEM &&
		 cmpAttr(ip0->attr, ip1->attr) == 0 && !accReferenced(p1) )
	{
		bool updated = false;
		for (bool done = false; !endOfScope(p1) && !done;)
		{
			for (int i = 3; i-- && !done;)
			{
				Item *ip = p1->items[i];
				if ( ip && same(ip0, ip) )
				{
					if ( i > 0 || !assignmentCode(p1->type, ALL_OP) )
					{
						ip->type = ACC_ITEM;
						updated  = true;
					}
					else
						done = true;	// stop here!
				}
			}
			p1 = p1->next;
		}
		if ( updated )
		{
			delete pnp;
			return true;
		}
		p1 = next(pnp, 1);	
	}

	if ( (ptype == DEC_OP || ptype == INC_OP) && p1 && p1->type == P_JNZ )		// {DEC_OP, [X, 1]}
	{																			// {P_JNZ, [X, Lx]}
		Item *ip = p1->items[0];
		if ( ip0->acceSize() == 1 && ip1->val.i == 1 && same(ip0, ip) )
		{
			pnp->type = (ptype == DEC_OP)? P_DJNZ: P_IJNZ;
			pnp->updateItem(1, p1->items[1]);
			p1->items[1] = NULL;
			delete p1;
			return true;
		}
	}
/*
	if ( ptype == '=' && p1 && p1->type == '=' &&								 // {'=', [*t, X]}
		 ip0->type == TEMP_ITEM && ip0->attr->ptrVect && ip1->type == ID_ITEM && // {'=', [Y, (t)]}
		 p1->items[1]->type == INDIR_ITEM && ip0->val.i == p1->items[1]->val.i )
	{
		ip1->type = PID_ITEM;
		ip1->updateAttr(p1->items[1]->attr);	p1->items[1]->attr = NULL;
		pnp->updateItem(0, p1->items[0]);		p1->items[0] = NULL;
		delete p1;
		return true;
	}
*/
	if ( (ptype == '=' || (ptype == '+' && ip2->type == CON_ITEM)) && ip0->type == TEMP_ITEM && p1 )
	{
		bool ret_code;

    	if ( ip1->type == ID_ITEM && ip1->attr && ip1->attr->ptrVect )			// {'+', [t, *X, N]}
        {																		// {op, [.. (t) ..]}
        	Item *ip = ip1->clone();
		    if ( ptype == '+' ) ip->bias += ip2->val.i;

            ip->type = PID_ITEM;
    		ret_code = replaceIndir(p1, ip, ip0->val.i);
    		delete ip;

    		if ( ret_code )
    		{
				delete pnp;
				return true;
			}
	   }

	   if ( ip1->type == CON_ITEM && ip1->attr && ip1->attr->ptrVect )			// {'+', [t, *(M), N]}
	   {																		// {op, [.. (t) ..]}
            Item *ip = ip1->clone();
            if ( ptype == '+' ) ip->val.i += ip2->val.i;
            ip->type = DIR_ITEM;
			
            ret_code = replaceIndir(p1, ip, ip0->val.i);
            delete ip;

            if ( ret_code )
            {
				delete pnp;
				return true;
			}
    	}
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