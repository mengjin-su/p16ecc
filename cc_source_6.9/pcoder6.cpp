#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "nlist.h"
#include "item.h"
#include "pnode.h"
#include "pcoder.h"
#include "dlink.h"
#include "flink.h"
#include "sizer.h"

void Pcoder :: run6(oprNode_t *op)
{
	int depth = DEPTH();
	Pnode *pnp;
	Item *ip0, *ip1, *tmp;
	int code = OPR_TYPE(op);
	int true_lbl, false_lbl;

	switch ( code )
	{
		case '*':	case '/':	case '%':
		case LEFT_OP:	case RIGHT_OP:
			run(OPR_NODE(op, 0));
			run(OPR_NODE(op, 1));
			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();
				ip0 = POP();
				
				if ( ip0->isMonoVal() && ip1->isMonoVal() )
				{
					pnp = new Pnode(code);
					addPnode(&mainPcode, pnp);
					pnp->items[1] = ip0;
					pnp->items[2] = ip1;
				
					Item *tmp = makeTemp();
					if ( code == LEFT_OP )
					{
						tmp->attr = newAttr(LONG);
						tmp->attr->isUnsigned = ip0->acceSign()? 0: 1;
					}
					else
						tmp->attr = maxMonoAttr(ip0, ip1, code);
						
					pnp->items[0] = tmp;
					PUSH(pnp->items[0]->clone());
					return;
				}
				delete ip0;
				delete ip1;
				errPrint("invalid operand(s)!");
			}
			break;
			
		case EQ_OP:	case NE_OP:
		case LE_OP: case GE_OP: 
		case '<': 	case '>':
			run(OPR_NODE(op, 0));
			run(OPR_NODE(op, 1));
			if ( DEPTH() == (depth+2) )
			{
				ip1 = POP();
				ip0 = POP();
				if ( !comparable(code, ip0, ip1) )
				{
					delete ip0;	delete ip1;
					errPrint("can't compare values!");
					return;
				}
				
				tmp = makeTemp();
				tmp->attr = newAttr(CHAR); tmp->attr->isUnsigned = 1;
				
				pnp = new Pnode('=');		addPnode(&mainPcode, pnp);	
				pnp->items[0] = tmp;
				pnp->items[1] = intItem(0);
				
				true_lbl  = getLbl();
				false_lbl = getLbl();
				compareBranch(code, ip0, ip1, true_lbl, false_lbl, true_lbl);
				
				PUT_LBL(true_lbl);
				pnp = new Pnode(INC_OP);	addPnode(&mainPcode, pnp);	
				pnp->items[0] = tmp;
				pnp->items[1] = intItem(1);
				
				PUT_LBL(false_lbl);
				PUSH(tmp->clone());
			}	
			break;
			
		case AND_OP:	// '&&'
			true_lbl  = getLbl();
			false_lbl = getLbl();
			
			pnp = new Pnode('=');		addPnode(&mainPcode, pnp);	
			tmp = makeTemp();
			tmp->attr = newAttr(CHAR); 
			tmp->attr->isUnsigned = 1;
			pnp->items[0] = tmp;
			pnp->items[1] = intItem(0);
			
			logicBranch(OPR_NODE(op, 0), true_lbl, false_lbl, true_lbl);
			PUT_LBL(true_lbl);
			
			true_lbl = getLbl();
			logicBranch(OPR_NODE(op, 1), true_lbl, false_lbl, true_lbl);
			PUT_LBL(true_lbl);
			
			pnp = new Pnode(INC_OP);	addPnode(&mainPcode, pnp);	
			pnp->items[0] = tmp->clone();
			pnp->items[1] = intItem(1);
			
			PUT_LBL(false_lbl);
			PUSH(tmp->clone());
			break;
			
		case OR_OP:		// '||'		
			true_lbl  = getLbl();
			false_lbl = getLbl();
			
			pnp = new Pnode('=');		addPnode (&mainPcode, pnp);	
			tmp = makeTemp();
			tmp->attr = newAttr(CHAR); tmp->attr->isUnsigned = 1;
			pnp->items[0] = tmp;
			pnp->items[1] = intItem(1);
			
			logicBranch(OPR_NODE(op, 0), true_lbl, false_lbl, false_lbl);
			PUT_LBL(false_lbl);
			
			false_lbl = getLbl();
			logicBranch(OPR_NODE(op, 1), true_lbl, false_lbl, false_lbl);
			PUT_LBL(false_lbl);
			
			pnp = new Pnode(DEC_OP);	addPnode (&mainPcode, pnp);	
			pnp->items[0] = tmp->clone();
			pnp->items[1] = intItem(1);
			
			PUT_LBL(true_lbl);
			PUSH(tmp->clone());
			break;
			
		default:
//			run7(op);
			break;
	}	
}	

void Pcoder :: compareBranch(int op, Item *ip0, Item *ip1, int true_lbl, int false_lbl, int next_lbl)
{
	int lbl;
	if ( true_lbl == next_lbl )
	{
		switch ( op )
		{
			case EQ_OP:	op = P_JNE;	break;	// '=='
			case NE_OP:	op = P_JEQ;	break;	// '!='
			case '<':	op = P_JGE;	break;	// '<'
			case '>':	op = P_JLE;	break;	// '>'
			case GE_OP:	op = P_JLT;	break;	// '>='
			case LE_OP:	op = P_JGT;	break;	// '<='
		}
		lbl = false_lbl;
	}
	else 
	{
		switch ( op )
		{
			case EQ_OP:	op = P_JEQ;	break;	// '=='
			case NE_OP:	op = P_JNE;	break;	// '!='
			case '<':	op = P_JLT;	break;	// '<'
			case '>':	op = P_JGT;	break;	// '>'
			case GE_OP:	op = P_JGE;	break;	// '>='
			case LE_OP:	op = P_JLE;	break;	// '<='
		}
		lbl = true_lbl;
	}
	
	Pnode *pnp = new Pnode(op);
	addPnode (&mainPcode, pnp);	
	pnp->items[0] = ip0;
	pnp->items[1] = ip1;
	pnp->items[2] = lblItem (lbl);
}

void Pcoder :: logicBranch(node *np, int true_lbl, int false_lbl, int next_lbl)
{
	if ( np == NULL )
	{
		errPrint("invalid branch operation!");
		return;
	}

	int depth = DEPTH();
	int lbl;
	switch ( np->type )
	{
		case NODE_CON:
		case NODE_ID:
			run(np);
			if ( DEPTH() == (depth+1) )
				logicBranch(POP(), true_lbl, false_lbl, next_lbl);
			break;
			
		case NODE_OPR:
            switch ( np->opr.oper )
            {
                case '!':
					takeSrc(np);
					logicBranch (np->opr.op[0], false_lbl, true_lbl, next_lbl);
					break;

				case EQ_OP:	case NE_OP:
				case GE_OP:	case LE_OP:
				case '>':	case '<':
					takeSrc(np);
					run(np->opr.op[0]);
					run(np->opr.op[1]);
					if ( DEPTH() == (depth+2) )
					{
						Item *ip1 = POP();
						Item *ip0 = POP();
						int  code = np->opr.oper;
						if ( comparable(code, ip0, ip1) )
						{
							compareBranch(code, ip0, ip1, true_lbl, false_lbl, next_lbl);
						}	
						else
						{
							delete ip0;
							delete ip1;
							errPrint("can't compare the values!");
						}
					}
					break;
					
                case AND_OP:	// x && y
					takeSrc(np);
					lbl = getLbl();
					logicBranch(np->opr.op[0], lbl, false_lbl, lbl);
					PUT_LBL(lbl);
					logicBranch(np->opr.op[1], true_lbl, false_lbl, next_lbl);
					break;
					
                case OR_OP:		// x || y
					takeSrc(np);
					lbl = getLbl();
					logicBranch(np->opr.op[0], true_lbl, lbl, lbl);
					PUT_LBL(lbl);
					logicBranch(np->opr.op[1], true_lbl, false_lbl, next_lbl);
					break;
					
                default:
					run(np);
					if ( DEPTH() == (depth+1) )
						logicBranch(POP(), true_lbl, false_lbl, next_lbl);
                    break;
			}
			break;
	}
}

void Pcoder :: logicBranch(Item *ip, int true_lbl, int false_lbl, int next_lbl)
{
	Pnode *pnp;
	
	if ( !ip->isOperable() )
	{
		delete ip;
		errPrint("invalid value type!");
		return;
	}
	
	if ( ip->type == CON_ITEM )
	{
		int n = ip->val.i;
		delete ip;
					
		if ( (true_lbl  == next_lbl &&  n) || 
			 (false_lbl == next_lbl && !n) )
			return;
						
		pnp = new Pnode(GOTO);
		addPnode (&mainPcode, pnp);	
		pnp->items[0] = lblItem(n? true_lbl: false_lbl);
	}
	else
	{
		int lbl = (true_lbl == next_lbl)? false_lbl: true_lbl;
		pnp = new Pnode((true_lbl == next_lbl)? P_JZ: P_JNZ);
		addPnode (&mainPcode, pnp);	
		pnp->items[0] = ip;
		pnp->items[1] = lblItem(lbl);
	}
}