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

#define TERMINATE_ASSIGN(c)	(c == '=' || c == '+' || c == '-' || \
							 c == '*' || c == '/' || c == '%' || \
							 c == '|' || c == '&' || c == '^' || \
							 c == LEFT_OP    				  || \
							 c == '~' || c == '!' || c == NEG_OF || \
							 c == P_ARG_PASS || c == P_ARG_PASS1 || \
							 c == P_MOV_INC  || c == P_MOV_DEC)

#define CONTINUE_ASSIGN(c)	(c == ADD_ASSIGN || c == SUB_ASSIGN || \
							 c == MUL_ASSIGN || c == DIV_ASSIGN || \
							 c == MOD_ASSIGN || c == AND_ASSIGN || \
							 c == XOR_ASSIGN || c ==  OR_ASSIGN || \
							 c == LEFT_ASSIGN|| c == RIGHT_ASSIGN)

#define VOLATILE_OP0(c)		(c == RIGHT_ASSIGN || \
							 c == DIV_ASSIGN || c == MOD_ASSIGN)

#define VOLATILE_OP1(c)		(VOLATILE_OP0(c) || c == '/' || c == '%')

#define VOLATILE_OP2(c)		(VOLATILE_OP1(c) && c != RIGHT_ASSIGN)

static void resizeTempItem(Item *ip, int size);

bool Optimizer :: case20 (Pnode *pnp)
{// minimize temp variable count ...
	if ( ITYPE(ip0, TEMP_ITEM) && TERMINATE_ASSIGN(pnp->type) )
	{
		for(int i = 0; i < ip0->val.i; i++)
		{
			if ( unusedTmp(pnp->next, i) )
			{
				replaceTemp(pnp->next, ip0->val.i, i);
				ip0->val.i = i;
				return true;
			}
		}
	}
	return false;
}

bool Optimizer :: case21 (Pnode *pnp)
{// minimize temp variable size...
	if ( TERMINATE_ASSIGN(pnp->type) || CONTINUE_ASSIGN(pnp->type) )
	{
		int size0 = ip0->acceSize();
		int size1 = ip1->acceSize();
		int size2 = ip2? ip2->acceSize(): 0;
		if ( size0 == 0 || size0 > 4 ) return false;

		if ( ITYPE(ip1, TEMP_ITEM) && !VOLATILE_OP1(pnp->type) )
		{
			if ( reduceTempSize(pnp->last, ip1, size0) || size1 > size0 )
			{
				resizeTempItem(ip1, size0);
				return true;
			}
		}
		if ( ITYPE(ip2, TEMP_ITEM) && !VOLATILE_OP2(pnp->type) )
		{
			if ( reduceTempSize(pnp->last, ip2, size0) || size2 > size0 )
			{
				resizeTempItem(ip2, size0);
				return true;
			}
		}
	}

	return false;
}

void Optimizer :: replaceTemp(Pnode *pnp, int temp_old, int temp_new)
{
	for (; pnp && !endOfScope(pnp); pnp = pnp->next)
	{
		for (int i = 0; i < 3; i++)
		{
			Item *ip = pnp->items[i];
			if ( ip == NULL )
				continue;

			if ( (ITYPE(ip, TEMP_ITEM) || ITYPE(ip, INDIR_ITEM)) &&
				 ip->val.i == temp_old )
			{
				ip->val.i = temp_new;
			}
		}
	}
}

bool Optimizer :: reduceTempSize(Pnode *pnp, Item *tmp, int size)
{
	bool result = false;
	bool done = false;

	for(; pnp && !done; pnp = pnp->last)
	{
		if ( PTYPE(pnp, ';') || PTYPE(pnp, P_SRC_CODE) )
			return result;

		for(int i = 3; i--;)
		{
			Item *ip = pnp->items[i];

			if ( ip == NULL || !ITYPE(ip, TEMP_ITEM) )
				continue;

			if ( ip->val.i == tmp->val.i )
			{
				if ( i == 0 && VOLATILE_OP0(pnp->type) ) {
					if ( ip->acceSize() != size ) {
						pnp->items[2] = pnp->items[1];
						pnp->items[1] = pnp->items[0];
						pnp->items[0] = ip->clone();
						resizeTempItem(pnp->items[0], size);
						pnp->type = (pnp->type == RIGHT_ASSIGN)? RIGHT_OP:
									(pnp->type == DIV_ASSIGN  )? '/': '%';
					}
					done = true;
					continue;	// not done until finishing this line.
				}
				if ( i == 1 && VOLATILE_OP1(pnp->type) ) {
					done = true;
					continue;	// not done until finishing this line.
				}
				if ( i == 2 && VOLATILE_OP2(pnp->type) ) {
					done = true;
					continue;	// not done until finishing this line.
				}

				if ( ip->acceSize() > size )
				{
					resizeTempItem(ip, size);
					result = true;
				}

				if ( i == 0 && TERMINATE_ASSIGN(pnp->type) )
					return result;
			}
		}
	}
	return result;
}

static void resizeTempItem(Item *ip, int size)
{
	if ( ip->acceSize() != size )
	{
		attrib *ap = newAttr(CHAR + size - 1);
		ap->isUnsigned = ip->acceSign()? 0: 1;
		ip->updateAttr(ap);
	}
}
