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

#define IS_CONST(ip,n)		(ITYPE(ip, CON_ITEM) && ip->val.i == n)

bool Optimizer :: group4(Pnode *pnp)
{
	int ptype = pnp->type;

	if ( IS_CONST(ip1, 0) )		// ip1 is constance '0'
	{
		switch (ptype)
		{
			case '+':			// {'+', [x, 0, y]}
			case '-':			// {'+', [x, 0, y]}
			case '|':			// {'|', [x, 0, y]}
			case '^':			// {'^', [x, 0, y]}
				pnp->type = '=';
				pnp->updateItem(1, ip2);
				pnp->items[2] = NULL;
				return true;

			case '*':			// {'*', [x, 0, y]}
			case '/':			// {'/', [x, 0, y]}
			case '%':			// {'%', [x, 0, y]}
			case '&':			// {'&', [x, 0, y]}
			case LEFT_OP:		// {'<<', [x, 0, y]}
			case RIGHT_OP:		// {'>>', [x, 0, y]}
				pnp->type = '=';
				pnp->updateItem(2, NULL);
				return true;

			case ADD_ASSIGN:	// {'+=', [x, 0]}
			case SUB_ASSIGN:	// {'-=', [x, 0]}
			case OR_ASSIGN:		// {'|=', [x, 0]}
			case XOR_ASSIGN:	// {'^=', [x, 0]}
			case LEFT_ASSIGN:	// {'<<=', [x, 0]}
			case RIGHT_ASSIGN:	// {'>>=', [x, 0]}
			case INC_OP:		// {INC_OP, [x, 0]}
			case DEC_OP:		// {DEC_OP, [x, 0]}
				delete pnp;
				return true;
		}
	}

	if ( IS_CONST(ip2, 0) )		// ip2 is constance '0'
	{
		switch (ptype)
		{
			case '+':			// {'+', [x, y, 0]}
			case '-':			// {'-', [x, y, 0]}
			case '|':			// {'|', [x, y, 0]}
			case '^':			// {'^', [x, y, 0]}
			case LEFT_OP:		// {'<<', [x, y, 0]}
			case RIGHT_OP:		// {'>>', [x, y, 0]}
				pnp->type = '=';
				pnp->updateItem(2, NULL);
				return true;

			case '*':			// {'*', [x, y, 0]}
			case '&':			// {'&', [x, y, 0]}
				pnp->type = '=';
				pnp->updateItem(1, ip2);
				pnp->items[2] = NULL;
				return true;
		}
	}

	if ( IS_CONST(ip1, 1) )		// ip1 is constance '1'
	{
		switch (ptype)
		{
			case '*':			// {'*', [x, 1, y]}
				pnp->type = '=';
				pnp->updateItem(1, ip2);
				pnp->items[2] = NULL;
				return true;

			case MUL_ASSIGN:	// {'*=', [x, 1]}
			case DIV_ASSIGN:	// {'/=', [x, 1]}
				delete pnp;
				return true;

			case MOD_ASSIGN:	// {'%=', [x, 1]}
				pnp->type = '=';
				ip1->val.i = 0;
				return true;
		}
	}

	if ( IS_CONST(ip2, 1) )		// ip2 is constance '1'
	{
		switch (ptype)
		{
			case '*':			// {'*', [x, y, 1]}
			case '/':			// {'/', [x, y, 1]}
				pnp->type = '=';
				pnp->updateItem(2, NULL);
				return true;
			case '%':			// {'%', [x, y, 1]}
				pnp->type = '=';
				ip2->val.i = 0;
				pnp->updateItem(1, ip2);
				pnp->items[2] = NULL;
				return true;
		}
	}

	int n;
	if ( bitSelect(ip1, &n) )	// ip1 is constance (1 << N)
	{
		switch (ptype)
		{
			case MUL_ASSIGN:	// {'*=', [x, 2^N]}
			case DIV_ASSIGN:	// {'/=', [x, 2^N]}
				pnp->type = (ptype == MUL_ASSIGN)? LEFT_ASSIGN: RIGHT_ASSIGN;
				ip1->val.i = n;
				return true;

			case MOD_ASSIGN:	// {'%=', [x, 2^N]}
				if ( !ip0->acceSign() )
				{
					pnp->type = AND_ASSIGN;
					ip1->val.i = (1 << n) - 1;
					return true;
				}
				break;
		}
	}

	if ( bitSelect(ip2, &n) )	// ip2 is constance (1 << N)
	{
		switch (ptype)
		{
			case '*':			// {'*', [x, y, 2^N]}
			case '/':			// {'/', [x, y, 2^N]}
				pnp->type = (ptype == '*')? LEFT_OP: RIGHT_OP;
				ip2->val.i = n;
				return true;

			case '%':			// {'%', [x, y, 2^N]}
				if ( !ip1->acceSign() )
				{
					pnp->type = '&';
					ip2->val.i = (1 << n) - 1;
					return true;
				}
				break;
		}
	}

	return false;
}