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

Optimizer :: Optimizer(Pcoder *_pcoder)
{
	memset(this, 0, sizeof(Optimizer));
	pcoder = _pcoder;
}

void Optimizer :: run(void)
{
	head = pcoder->mainPcode;
	bool done = false;
	while ( !done )
	{
		done = true;
		for(Pnode *pnp = head; pnp; pnp = pnp->next)
		{
			ip0 = pnp->items[0];
			ip1 = pnp->items[1];
			ip2 = pnp->items[2];
			if ( pnp->type == P_FUNC_BEG ) funcPtr = pnp;
			if ( pnp->type == P_FUNC_END ) funcPtr = NULL;

			if ( group1(pnp) ) { done = false; break; }
			if ( group2(pnp) ) { done = false; break; }
			if ( group3(pnp) ) { done = false; break; }
		}
	}
}

Pnode *Optimizer :: next(Pnode *p, int offset)
{
	bool forward = (offset > 0);
	while ( p )
	{
		if ( (p->type == ';' || p->type == P_SRC_CODE) )
		{
			if ( forward ) { p = p->next; }
			else	 	   { p = p->last; }
			continue;
		}

		if ( p->type == P_FUNC_END || p->type == P_FUNC_BEG )
			return NULL;

		if ( offset == 0 )
			return p;

		if ( forward ) { p = p->next; offset--; }
		else	   	   { p = p->last; offset++; }
	}
	return NULL;
}


bool Optimizer :: updateLbl(char *old_lbl, char *new_lbl, Pnode *p)
{
	bool ret = false;
	for(Pnode *pnp = funcPtr; pnp; pnp = pnp->next)
	{
		if ( pnp->type == P_FUNC_END ) break;
//		if ( pnp == p ) continue;
		if ( pnp->type == LABEL ) continue;
		for(int i = 0; i < 3 && pnp->items[i]; i++)
		{
			Item *ip = pnp->items[i];
			if ( ip->type == LBL_ITEM && !strcmp(ip->val.s, old_lbl) )
			{
				free (ip->val.s);
				ip->val.s = dupStr(new_lbl);
				ret = true;
			}
		}
	}
	return ret;
}

bool Optimizer :: updateGoto(char *old_lbl, char *new_lbl, Pnode *p1)
{
	bool ret = false;
	for(Pnode *pnp = funcPtr; pnp; pnp = pnp->next)
	{
		if ( pnp->type == P_FUNC_END ) break;
		if ( pnp == p1 || pnp->type == LABEL ) continue;
		for(int i = 0; i < 3 && pnp->items[i]; i++)
		{
			Item *ip = pnp->items[i];
			if ( ip->type == LBL_ITEM && !strcmp(ip->val.s, old_lbl) )
			{
				free (ip->val.s);
				ip->val.s = dupStr(new_lbl);
				ret = true;
			}
		}
//		if ( pnp->type == GOTO && strcmp(pnp->items[0]->val.s, old_lbl) == 0 )
//		{
//			pnp->items[0]->updateName(new_lbl);
//			ret = true;
//		}
	}
	return ret;
}

bool Optimizer :: unusedLbl(Pnode *pnp, char *lbl, Pnode *p)
{
	for(; pnp; pnp = pnp->next)
	{
		if ( pnp->type != LABEL )
		for(int i = 0; pnp != p && i < 3 && pnp->items[i]; i++)
		{
			Item *ip = pnp->items[i];
			if ( ip->type == LBL_ITEM && !strcmp(ip->val.s, lbl) )
				return false;
		}
	}
	return true;
}

bool Optimizer :: endOfScope(Pnode *pnp)
{
	return (pnp == NULL 			||
			pnp->type == ';' 		||
			pnp->type == P_FUNC_END );
}

bool Optimizer :: unusedTmp(Pnode *pnp, int tmp_idx)
{
	for (; pnp; pnp = pnp->next)
	{
		for (int i = 3; i--;)
		{
			Item *ip = pnp->items[i];
			if ( ip )
			{
				if ( ITYPE(ip, TEMP_ITEM) && ip->val.i == tmp_idx )
				{
					if ( i > 0 )
						return false;

					return assignmentCode(pnp->type, ALL_OP);
				}
				if ( ITYPE(ip, INDIR_ITEM) && ip->val.i == tmp_idx )
					return false;
			}
		}
		if ( endOfScope(pnp) )
			return true;
	}
	return true;
}

bool Optimizer :: bitSelect(Item *ip, int *n)
{
	if ( ip && ip->type == CON_ITEM )
	{
		unsigned int num = (unsigned int)ip->val.i;
		int position = 0;
		for (; num; num >>= 1)
		{
			if ( num & 1 )
			{
				*n = position;
				return (num == 1);
			}
			position++;
		}
	}
	return false;
}

bool Optimizer :: bitDeselect(Item *ip, int *n, int size)
{
	if ( ip && ip->type == CON_ITEM )
	{
		unsigned int num = (~ip->val.i) & ((1 << size) - 1);
		int position = 0;
		for (; num; num >>= 1)
		{
			if ( num & 1 )
			{
				*n = position;
				return (num == 1);
			}
			position++;
		}
	}

	return false;
}

bool Optimizer :: assignmentCode(int code, int types)
{
	return ((types & MATH_OP ) && (code == '+' || code == '-' || code == '*' || code == '/' || code == '%')) ||
		   ((types & LOGIC_OP) && (code == '|' || code == '&' || code == '^')							   ) ||
		   ((types & MOVE_OP ) && (code == '=')                         								   ) ||
		   ((types & MONO_OP ) && (code == '!' || code == '~' || code == NEG_OF)						   ) ||
		   ((types & SHIFT_OP) && (code == LEFT_OP || code == RIGHT_OP)									   ) ;
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
				if ( i == 0 && assignmentCode(pnp->type, ALL_OP) )
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

bool Optimizer :: accReferenced(Pnode *pnp)
{
	for (; pnp; pnp = pnp->next)
	{
		if ( pnp->type == CALL || pnp->type == P_CALL )
			return true;

		if ( endOfScope(pnp) )
			break;
	}
	return false;
}

bool Optimizer :: replaceIndir(Pnode *pnp, Item *ip, int temp_index)
{
	for (int i = 3; pnp && i--;)
	{
		Item *iptr = pnp->items[i];
		if ( iptr == NULL ) continue;
		if ( iptr->type == INDIR_ITEM && iptr->val.i == temp_index )
		{
			if ( (assignmentCode(pnp->type, ALL_OP) && i == 0) 							   ||
				 (assignmentCode(pnp->type, ALL_OP) && IS_TEMP(pnp->items[0], temp_index)) ||
				 unusedTmp(pnp->next, temp_index) 									)
			{
				attrib *ap = cloneAttr(iptr->attr);
				ap->dataBank = ip->attr->dataBank;
				ap->isVolatile = ip->attr->isVolatile;
				pnp->updateItem(i, ip->clone());
				pnp->items[i]->updateAttr(ap);
				return true;
			}
		}
	}
	return false;
}
