#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "nlist.h"
#include "item.h"
#include "pnode.h"
#include "pcoder.h"
#include "dlink.h"
#include "flink.h"
#include "sizer.h"

static int caseValCheck(int *val_list, int i, int val, int default_idx);

void Pcoder :: run7(oprNode_t *op)
{
	int depth = DEPTH();
	int code  = OPR_TYPE(op);
	int true_lbl, false_lbl, end_lbl, loop_lbl, continue_lbl;

	switch ( code )
	{
		case IF:
			true_lbl  = getLbl();
			false_lbl = getLbl();
			end_lbl   = getLbl();

			logicBranch(op->op[0], true_lbl, false_lbl, true_lbl);
			addPnode(&mainPcode, new Pnode(';'));

			PUT_LBL(true_lbl);
			run(op->op[1]);	// true statement

			if ( op->op[2] )
				addPnode(&mainPcode, new Pnode(GOTO, lblItem(end_lbl)));

			PUT_LBL(false_lbl);
			run(op->op[2]);	// false statement(can be NULL)

			PUT_LBL(end_lbl);
			break;

		case WHILE:	// while (...) ...
			loop_lbl  = getLbl();
			true_lbl  = getLbl();
			false_lbl = getLbl();

			PUT_LBL(loop_lbl);
			logicBranch(op->op[0], true_lbl, false_lbl, true_lbl);
			addPnode(&mainPcode, new Pnode(';'));

			PUT_LBL(true_lbl);

			continueStack.push(loop_lbl);
			breakStack.push(false_lbl);
			run(op->op[1]);
			continueStack.pop();
			breakStack.pop();

			addPnode(&mainPcode, new Pnode(GOTO, lblItem(loop_lbl)));
			PUT_LBL(false_lbl);
			break;

		case DO:	// do ... while (...);
			loop_lbl  = getLbl();
			true_lbl  = getLbl();
			false_lbl = getLbl();

			PUT_LBL(loop_lbl);

			continueStack.push(true_lbl);
			breakStack.push(false_lbl);
			run(op->op[0]);
			continueStack.pop();
			breakStack.pop();

			PUT_LBL(true_lbl);
			logicBranch(op->op[1], loop_lbl, false_lbl, false_lbl);
			PUT_LBL(false_lbl);
			break;

		case FOR:
			loop_lbl 	 = getLbl();
			true_lbl 	 = getLbl();
			end_lbl  	 = getLbl();
			continue_lbl = getLbl();

			run(op->op[0]);	addPnode(&mainPcode, new Pnode(';')); 	// ***

			PUT_LBL(loop_lbl);

			if ( op->op[1] )
			{
				logicBranch(op->op[1], true_lbl, end_lbl, true_lbl);
				PUT_LBL(true_lbl);
			}

			continueStack.push(continue_lbl);
			breakStack.push(end_lbl);
			run(op->op[3]);	addPnode(&mainPcode, new Pnode(';'));	// ***
			continueStack.pop();
			breakStack.pop();

			PUT_LBL(continue_lbl);
			run(op->op[2]);	addPnode(&mainPcode, new Pnode(';'));
			addPnode(&mainPcode, new Pnode(GOTO, lblItem(loop_lbl)));

			PUT_LBL(end_lbl);
			break;

		case BREAK:
			if ( breakStack.depth() > 0 )
				addPnode(&mainPcode, new Pnode(GOTO, lblItem(breakStack.top())));
			else
				errPrint("illegal 'break' statement!");
			break;

		case CONTINUE:
			if ( continueStack.depth() > 0 )
				addPnode (&mainPcode, new Pnode(GOTO, lblItem(continueStack.top())));
			else
				errPrint("illegal 'continue' statement!");
			break;

		case SWITCH:
			run(op->op[0]);

			if ( DEPTH() == (depth+1) )
			{
				Item *ip = POP();
				if ( ip->isMonoVal() ) 
					switchListProc(ip, op->op[1]);
				else
				{
					errPrint("can't evaluate for SWITCHing!");
					delete ip;
				}
			}
			break;

		default:
			run8(op);
			break;
	}
}

void Pcoder :: switchListProc(Item *ip, node *list)
{
	Pnode *pnp;
	int length = LIST_LENGTH(list);
	int *lbl_list = new int[length];	// case labels
	int *val_list = new int[length];	// case values
	int  end_lbl  = getLbl();			// end label
	int  default_idx = -1;				// default case label index

	if ( !(ip->type == ID_ITEM || ip->type == TEMP_ITEM || ip->type == ACC_ITEM) )								 
	{
		pnp = new Pnode('=');	addPnode (&mainPcode, pnp);
		pnp->items[1] = ip;
		pnp->items[0] = makeTemp();
		pnp->items[0]->attr = newAttr((ip->acceSize() == 1)? CHAR :
									  (ip->acceSize() == 2)? INT  :
									  (ip->acceSize() == 3)? SHORT: LONG);
		ip = pnp->items[0]->clone();
	}

	for (int i = 0; i < length; i++)
	{
		node *np = LIST_NODE(list, i);
		lbl_list[i] = getLbl();

		if ( np->opr.nops == 1 ) // is DEFAULT case?
		{
			if ( default_idx >= 0 )
			{
				errPrint("multiple 'default'!");
				return;
			}
			default_idx = i;
		}
		else
		{
			int depth = DEPTH();
			run(np->opr.op[0]);		// get 'case' value...
			if ( DEPTH() == (depth+1) )
			{
				Item *cip = POP();	// case N node
				if ( cip->type != CON_ITEM )
					errPrint("contant expected for 'case'!");
				else if ( caseValCheck(val_list, i, cip->val.i, default_idx) )
					errPrint("duplicated switch case value!");
				else
				{
					val_list[i] = cip->val.i;
					pnp = new Pnode(CASE);
					addPnode (&mainPcode, pnp);
					pnp->items[0] = ip->clone();
					pnp->items[1] = intItem(cip->val.i);
					pnp->items[2] = lblItem(lbl_list[i]);
				}
				delete cip;			// delete case N node
			}
		}
	}

	pnp = new Pnode(GOTO);	addPnode (&mainPcode, pnp);
	if ( default_idx < 0 )	// if no 'default' applied?
		pnp->items[0] = lblItem(end_lbl);
	else
		pnp->items[0] = lblItem(lbl_list[default_idx]);

	addPnode(&mainPcode, new Pnode(';'));

	breakStack.push(end_lbl);
	for (int i = 0; i < length; i++)
	{
		node *np = LIST_NODE(list, i);

		PUT_LBL(lbl_list[i]);

		if ( np->opr.nops == 1 )	// default:
			run(np->opr.op[0]);
		else						// case N:
			run(np->opr.op[1]);

		addPnode(&mainPcode, new Pnode(';'));
	}
	breakStack.pop();

	PUT_LBL(end_lbl);
	delete [] lbl_list;
	delete [] val_list;
	delete ip;
}

static int caseValCheck(int *val_list, int limit, int val, int default_idx)
{
	for (int i = 0; i < limit; i++)
	{
		if ( i != default_idx && val_list[i] == val )
			return -1;
	}
	return 0;
}
