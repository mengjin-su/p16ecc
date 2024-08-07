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


#define TERMINATE_ASSIGN(t)	(t == '=' || t == '+' || t == '-' || \
							 t == '*' || t == '/' || t == '%' || \
							 t == '|' || t == '&' || t == '^' || \
							 t == LEFT_OP    || t == RIGHT_OP || \
							 t == '~' || t == '!' || t == NEG_OF || \
							 t == P_ARG_PASS )

#define CONTINUE_ASSIGN(t)	(t == ADD_ASSIGN || t == SUB_ASSIGN || \
							 t == MUL_ASSIGN || t == DIV_ASSIGN || \
							 t == MOD_ASSIGN || t == AND_ASSIGN || \
							 t == XOR_ASSIGN || t ==  OR_ASSIGN || \
							 t == LEFT_ASSIGN|| t == RIGHT_ASSIGN)

#define VOLATILE_OP0(t)		(t == RIGHT_ASSIGN || \
							 t == DIV_ASSIGN || t == MOD_ASSIGN)

#define VOLATILE_OP1(t)		(t == RIGHT_OP || t == '/' || t == '%' || \
							 t == DIV_ASSIGN || t == MOD_ASSIGN)

static void resizeTmpItem(Item *ip, int new_size);


bool Optimizer :: group5(Pnode *pnp)
{
	int ptype = pnp->type;

	if ( TERMINATE_ASSIGN(ptype) && ip0->type == TEMP_ITEM &&	// {'=', [t, x]}
		 unusedTmp(pnp->next, ip0->val.i) 					)	// 't' not used any more
	{
		delete pnp;
		return true;
	}

	if ( TERMINATE_ASSIGN(ptype) && ip0->type == TEMP_ITEM )	// {'=', [tn, x]} or {'*', [tn, x, y]} ...
	{
		for (int i = 0; i < ip0->val.i; i++)
		{
			if ( unusedTmp(pnp->next, i) )
			{
				replaceTmp(pnp->next, ip0->val.i, i);
				ip0->val.i = i;
				return true;
			}
		}
	}

	if ( TERMINATE_ASSIGN(ptype) || CONTINUE_ASSIGN(ptype) )	// {'+', [z, x, y]} or {'+=', [x, y]} ...
	{
		int size0 = ip0->acceSize();
		int size1 = ip1->acceSize();
		int size2 = ip2? ip2->acceSize(): 0;
		if ( size0 == 0 || size0 > 4 ) return false;

		if ( ip1 && ip1->type == TEMP_ITEM && !VOLATILE_OP1(ptype) )
		{
			if ( reduceTmpSize(pnp->last, ip1, size0) || size1 > size0 )
			{
				resizeTmpItem(ip1, size0);
				return true;
			}
		}

		if ( ip2 && ip2->type == TEMP_ITEM && !VOLATILE_OP1(ptype) )
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
	for(bool done = false; pnp && !done; pnp = pnp->last)
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