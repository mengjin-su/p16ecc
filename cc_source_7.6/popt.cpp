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

			if ( case1 (pnp) ) { done = false; break; }
			if ( case2 (pnp) ) { done = false; break; }
			if ( case3 (pnp) ) { done = false; break; }
			if ( case4 (pnp) ) { done = false; break; }
			if ( case5 (pnp) ) { done = false; break; }
			if ( case6 (pnp) ) { done = false; break; }
			if ( case7 (pnp) ) { done = false; break; }
			if ( case8 (pnp) ) { done = false; break; }
			if ( case9 (pnp) ) { done = false; break; }
			if ( case10(pnp) ) { done = false; break; }
			if ( case11(pnp) ) { done = false; break; }
			if ( case12(pnp) ) { done = false; break; }
			if ( case13(pnp) ) { done = false; break; }
			if ( case14(pnp) ) { done = false; break; }
			if ( case15(pnp) ) { done = false; break; }
			if ( case16(pnp) ) { done = false; break; }
			if ( case17(pnp) ) { done = false; break; }
			if ( case18(pnp) ) { done = false; break; }
			if ( case19(pnp) ) { done = false; break; }
			if ( case20(pnp) ) { done = false; break; }
			if ( case21(pnp) ) { done = false; break; }
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

					return assignmentCode(pnp->type);
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
