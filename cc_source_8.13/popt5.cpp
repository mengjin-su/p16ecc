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

#define TERMINATE_ASSIGN(c)	(c == '=' || c == '+' || c == '-' || \
							 c == '*' || c == '/' || c == '%' || \
							 c == '|' || c == '&' || c == '^' || \
							 c == LEFT_OP    || c == RIGHT_OP || \
							 c == '~' || c == '!' || c == NEG_OF || \
							 c == P_ARG_PASS )

#define CONTINUE_ASSIGN(c)	(c == ADD_ASSIGN || c == SUB_ASSIGN || \
							 c == MUL_ASSIGN || c == DIV_ASSIGN || \
							 c == MOD_ASSIGN || c == AND_ASSIGN || \
							 c == XOR_ASSIGN || c ==  OR_ASSIGN || \
							 c == LEFT_ASSIGN|| c == RIGHT_ASSIGN)

#define VOLATILE_OP0(c)		(c == RIGHT_ASSIGN || \
							 c == DIV_ASSIGN || c == MOD_ASSIGN)

#define VOLATILE_OP1(c)		(c == RIGHT_OP || c == '/' || c == '%' || \
							 c == DIV_ASSIGN || c == MOD_ASSIGN)

static void resizeTmpItem(Item *ip, int new_size);

bool Optimizer :: case19(Pnode *pnp)
{// remove unused temp variable assignment ...
	if ( ITYPE(ip0, TEMP_ITEM) && TERMINATE_ASSIGN(pnp->type) &&
		 unusedTmp(pnp->next, ip0->val.i) 						)
	{
		delete pnp;
		return true;
	}			 
	return false;	
}

bool Optimizer :: case20(Pnode *pnp)
{// minimize temp variable count ...
	if ( ITYPE(ip0, TEMP_ITEM) && TERMINATE_ASSIGN(pnp->type) )
	{
		for(int i = 0; i < ip0->val.i; i++)
		{
			if ( unusedTmp(pnp->next, i) )
			{
				replaceTmp(pnp->next, ip0->val.i, i);
				ip0->val.i = i;
				return true;
			}
		}
	}
	return false;
}

bool Optimizer :: case21(Pnode *pnp)
{// minimize temp variable size...
	if ( TERMINATE_ASSIGN(pnp->type) || CONTINUE_ASSIGN(pnp->type) )
	{
		int size0 = ip0->acceSize();
		int size1 = ip1->acceSize();
		int size2 = ip2? ip2->acceSize(): 0;
		if ( size0 == 0 || size0 > 4 ) return false;

		if ( ip1 && ITYPE(ip1, TEMP_ITEM) && !VOLATILE_OP1(pnp->type) )
		{
			if ( reduceTmpSize(pnp->last, ip1, size0) || size1 > size0 )
			{
				resizeTmpItem(ip1, size0);
				return true;
			}
		}
		if ( ip2 && ITYPE(ip2, TEMP_ITEM) && !VOLATILE_OP1(pnp->type) )
		{
			if ( reduceTmpSize(pnp->last, ip2, size0) || size2 > size0 )
			{
				resizeTmpItem(ip2, size0);
				return true;
			}
		}
	}

	return false;
}

void Optimizer :: replaceTmp(Pnode *pnp, int tmp_old, int tmp_new)
{
	for (; pnp && !endOfScope(pnp); pnp = pnp->next)
	{
		for (int i = 0; i < 3; i++)
		{
			Item *ip = pnp->items[i];
			if ( ip == NULL )
				continue;

			if ( (ITYPE(ip, TEMP_ITEM) || ITYPE(ip, INDIR_ITEM)) &&
				 ip->val.i == tmp_old )
			{
				ip->val.i = tmp_new;
			}
		}
	}
}

bool Optimizer :: reduceTmpSize(Pnode *pnp, Item *tmp, int new_size)
{
	bool ret_code = false;
	bool done = false;

	for(; pnp && !done; pnp = pnp->last)
	{
		if ( PTYPE(pnp, ';') || PTYPE(pnp, P_SRC_CODE) )
			return ret_code;

		for(int i = 3; i--;)
		{
			Item *ip = pnp->items[i];

			if ( ip == NULL || !ITYPE(ip, TEMP_ITEM) )
				continue;

			if ( ip->val.i == tmp->val.i )
			{
				if ( i == 0 && VOLATILE_OP0(pnp->type) ) {
					done = true;
					continue;	// not done until finishing this line.
				}
				if ( i != 0 && VOLATILE_OP1(pnp->type) ) {
					done = true;
					continue;	// not done until finishing this line.
				}

				if ( ip->acceSize() > new_size )
				{
					resizeTmpItem(ip, new_size);
					ret_code = true;
				}

				if ( i == 0 && TERMINATE_ASSIGN(pnp->type) )
					return ret_code;
			}
		}
	}
	return ret_code;
}

static void resizeTmpItem(Item *ip, int new_size)
{
	if ( ip->acceSize() != new_size )
	{
		attrib *ap = newAttr(CHAR + new_size - 1);
		ap->isUnsigned = ip->acceSign()? 0: 1;
		ip->updateAttr(ap);
	}
}
