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

bool convertJump(int code, int *rev_code, int *par_count);
bool swapJump(int code, int *rev_code);

//////////////////////////////////////////////////////////////////////////
bool Optimizer :: case6(Pnode *pnp)
{
	/*
		%i = x;		-> remove this line if temp(%i) not used later.
		x++;
	*/
	if ( pnp->type == '=' && ip0->type == TEMP_ITEM )
	{
		if ( unusedTmp(pnp->next, ip0->val.i) )
		{
			delete pnp;
			return true;
		}
	}
	return false;
}

bool Optimizer :: case7(Pnode *pnp)
{	/*
		P_JZ	'x', LABELx	|
		GOTO	LABELy		|	->	P_JNZ	'x', LABELy
		LABELx:				|		LABELx:
	*/
	Pnode *p1 = next(pnp, 1);
	Pnode *p2 = next(pnp, 2);
	int rev_type, n;
	if ( convertJump(pnp->type, &rev_type, &n) && p1 && p2 )
	{
		if ( p1->type == GOTO && p2->type == LABEL &&
			 same(pnp->items[n-1], p2->items[0]) )
		{
			pnp->type = rev_type;
			pnp->updateName(n-1, p1->items[0]->val.s);
			delete p1;
			return true;
		}
	}
	return false;
}

bool Optimizer :: case8(Pnode *pnp)
{	/*
		%i = x & (1 << N);	|
		J_Z %i, LABEL		|	->	P_JBZ	x, N, LABEL
	*/
	Pnode *p1 = next(pnp, 1);
	int type  = pnp->type;
	int n;
	if ( type == '&' && ITYPE(ip0, TEMP_ITEM) && p1 &&
		 bitSelect(ip2, &n)				     	 	&&
		 same(ip0, p1->items[0])					&&
		 (p1->type == P_JZ || p1->type == P_JNZ) 	&&
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
	return false;
}

bool Optimizer :: case9(Pnode *pnp)
{	/*
		%i = #IMMD_ITEM;	|
		...
		x = [%i]
	*/
	int i;
	if ( pnp->type == '=' &&
		 ITYPE(ip0, TEMP_ITEM) && ITYPE(ip1, IMMD_ITEM) )
	{
		Pnode *p1 = indirReferenced(pnp->next, ip0, &i);
		if ( p1 )
		{
			if ( ITYPE(p1->items[i], INDIR_ITEM) )
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
	return false;
}

bool Optimizer :: case10(Pnode *pnp)
{
	int swap_type;
	if ( (pnp->type == P_JEQ || pnp->type == P_JNE) &&
		 ITYPE(ip1, CON_ITEM) && ip1->val.i == 0 )
	{
		pnp->type = (pnp->type == P_JEQ)? P_JZ: P_JNZ;
		pnp->updateItem(1, ip2);
		pnp->items[2] = NULL;
		return true;
	}
	if ( swapJump(pnp->type, &swap_type) &&
		 ITYPE(ip0, CON_ITEM) && !ITYPE(ip1, CON_ITEM) )
	{
		pnp->type = swap_type;
		pnp->items[0] = ip1;
		pnp->items[1] = ip0;
		return true;
	}
	return false;
}

bool convertJump(int code, int *rev_code, int *par_count)
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

bool swapJump(int code, int *rev_code)
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

bool Optimizer :: assignmentCode(int code)
{
	switch ( code )
	{
		case '+':	case '-':	case '*':	case '/':	case '%':
		case '&':	case '|':	case '^':	case '=':	case '!':
		case LEFT_OP:	case RIGHT_OP:	case NEG_OF:	case '~':
			return true;
	}
	return false;
}

Pnode *Optimizer :: indirReferenced(Pnode *pnp, Item *tmp, int *index)
{
	int referenced = 0;
	Pnode *pp = NULL;
	for (; pnp; pnp = pnp->next)
	{
		for (int i = 3; i--;)	// backward scanning
		{
			Item *ip = pnp->items[i];
			if ( ip == NULL ) continue;

			if ( ITYPE(ip, INDIR_ITEM) && ip->val.i == tmp->val.i )
			{
				*index = i;		// tmp used as indir
				pp = pnp;		// log the pointer
				referenced++;	//
				continue;
			}
			if ( sameTemp(ip, tmp) )
			{	// tmp instance life time expired
				if ( i == 0 && assignmentCode(pnp->type) )
					return (referenced == 1)? pp: NULL;

				*index = i;		// tmp used as indir
				pp = pnp;		// log the pointer
				referenced++;	//
				continue;
			}
		}

		if ( endOfScope(pnp) )
			return (referenced == 1)? pp: NULL;
	}

	return (referenced == 1)? pp: NULL;
}
/*
Pnode *Optimizer :: tempReferenced(Pnode *pnp, Item *tmp, Item *aux, int *index)
{
	int referenced = 0;
	Pnode *pp = NULL;
	for (; pnp; pnp = pnp->next)
	{
		int type = pnp->type;
		if ( type == GOTO || type == CALL || type == P_CALL )
			return (referenced == 1)? pp: NULL;

		for (int i = 3; i--;)	// backward scanning
		{
			Item *ip = pnp->items[i];
			if ( ip == NULL ) continue;

			if ( sameTemp(ip, tmp) )
			{
				// tmp instance life time expired
				if ( i == 0 && assignmentCode(pnp->type) )
					return (referenced == 1)? pp: NULL;

                *index = i;     // tmp used
				pp = pnp;		// log the pointer
				referenced++;	//
				continue;
			}

			if ( ITYPE(ip, INDIR_ITEM) && ip->val.i == tmp->val.i )
			{
				*index = i;		// tmp used as indir
				pp = pnp;		// log the pointer
				referenced++;	//
				continue;
			}

			if ( aux && sameItems(ip, aux) )
			{
				if ( !referenced ) return NULL;
				aux = NULL;
			}
		}

		if ( endOfScope(pnp) )
			return (referenced == 1)? pp: NULL;
	}

	return (referenced == 1)? pp: NULL;
}
*/