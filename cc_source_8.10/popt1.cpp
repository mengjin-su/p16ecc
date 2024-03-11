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

/****************************************************************
case-1:	Merge label lines
	Label1:
	Label2:		->	Label1:
*/
bool Optimizer :: case1(Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);
	if ( pnp->type == LABEL && p1 && p1->type == LABEL )
	{
		updateLbl(p1->items[0]->val.s, ip0->val.s, p1);
		delete p1;
		return true;
	}
	return false;
}

/****************************************************************
case-2:	Remove unused label lines.
*/
bool Optimizer :: case2(Pnode *pnp)
{
	if ( pnp->type == LABEL && unusedLbl(head, ip0->val.s, pnp) )
	{
		delete pnp;
		return true;
	}
	return false;
}

/****************************************************************
case-3: Remove useless GOTO line
	GOTO Label;
	Label:
*/
bool Optimizer :: case3(Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);
	if ( pnp->type == GOTO && p1 && p1->type == LABEL )
	{
		if ( !strcmp(ip0->val.s, p1->items[0]->val.s) )
		{
			delete pnp;
			return true;
		}
	}
	return false;
}

/****************************************************************
case-4: Convert '+/-' operation to simple '=' assignment

	%i = #x + N	->	%i = #(x+N)
*/
bool Optimizer :: case4(Pnode *pnp)
{
	int code = pnp->type;
	if ( (code == '+' || code == '-' || code == '*' || code == '/' ||
		  code == '%' || code == '&' || code == '|' || code == '^' ) &&
		 ITYPE(ip1, IMMD_ITEM) && ITYPE(ip2, CON_ITEM) )
	{
		if ( ip2->val.i != 0 )
		{
			char buf[32];
			sprintf(buf, "%c%d)", code, ip2->val.i);
			std::string str = ip1->val.s;
			str.insert(0, "(");	str += buf;
			pnp->updateName(1, (char*)str.c_str());
		}
		pnp->type = '=';
		pnp->updateItem(2, NULL);
		return true;
	}

	return false;
}

/****************************************************************
case-5: Make jump/goto faster / remove useless GOTO statements
*/
bool Optimizer :: case5(Pnode *pnp)
{
	Pnode *p1 = next(pnp, 1);
	if ( pnp->type == LABEL && p1 && p1->type == GOTO &&
		 strcmp(ip0->val.s, p1->items[0]->val.s) != 0 &&
		 updateGoto(ip0->val.s, p1->items[0]->val.s, p1) )
	{	// make all 'jump to LABELx' to 'jump to LABELy'
		return true;
	}
	// following optimizations have some risk!
	if ( pnp->type == RETURN && p1 && p1->type == GOTO )
	{	// remove useless GOTO
		delete p1;
		return true;
	}
	if ( pnp->type == GOTO && p1 && p1->type == GOTO )
	{	// remove useless GOTO
		delete p1;
		return true;
	}
	return false;
}