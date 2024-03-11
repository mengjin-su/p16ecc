#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "nlist.h"
#include "prescan.h"
#include "sizer.h"
#include "display.h"

#define GET_COMMENT(p)		if(((node*)(p))->id.src) src = ((node*)(p))->id.src
#define CLR_COMMENT(p)		if(((node*)(p))->id.src == src) src = NULL


PreScan :: PreScan(Nlist *list)	// constructor
{
	nlist = list;
	errCount = 0;
	tagSeed  = 0;
	src   = NULL;
}

node *PreScan :: scan(node *np)
{
	if ( np != NULL )
	{
		GET_COMMENT(np);

		switch ( np->type )
		{
			case NODE_ID:	// scan an Id node
				return scan((idNode_t*)np);

			case NODE_STR:	// scan a String node
			case NODE_CON:	// scan a Constant node
				break;

			case NODE_OPR:	// scan an Operation node
				return scan((oprNode_t *)np);

			case NODE_LIST:	// scan a List node
				for (int i = 0; i < np->list.nops; i++)
					np->list.ptr[i] = scan(np->list.ptr[i]);
				break;
		}
	}
	return np;
}

node *PreScan :: scan(idNode_t *ip)
{
	Nnode *dp = nlist->search(ip->name);
	if ( dp && (dp->type == ENUM || dp->type == DEFINE) && dp->nops() == 1 )
	{
		node *new_np = cloneNode(dp->np[0]);
		new_np->con.src = ip->src, ip->src = NULL;

		if ( new_np->type == NODE_ID )
		{
			if ( ip->init ) moveNode(&new_np->id.init, &ip->init);
			if ( ip->dim  ) moveNode(&new_np->id.dim , &ip->dim);
			if ( ip->parp ) moveNode(&new_np->id.parp, &ip->parp);
		}
		else if ( ip->init || ip->dim || ip->parp )
			errPrint("error in #define replacement");

		CLR_COMMENT((node*)ip), delNode((node*)ip);
		ip = (idNode_t*)new_np;
	}
	else if ( dp && dp->type == SBIT && dp->nops() == 1 )
	{
		node *np = oprNode(SBIT, 1);
		np->opr.op[0] = cloneNode(dp->np[0]);
		CLR_COMMENT(np), delNode(np);
		return np;
	}

	if ( ip->type == NODE_ID )
	{
		ip->init = scan(ip->init);
		ip->parp = scan(ip->parp);
		ip->dim  = scan(ip->dim);
		if ( ip->dim )	// check if dimensions are defined...
		{
			node *lp = ip->dim; // get dimension list
			for (int i = 0; i < lp->list.nops; i++)
				if ( lp->list.ptr[i]->type != NODE_CON )
				{
					errPrint("id dimension undefined!", ip->name);
					break;
				}
		}
	}
	return (node*)ip;
}

node *PreScan :: scan(oprNode_t *op)
{
	Nnode *ndp = NULL;
	attrib *ap = NULL;
	node  *np = NULL, *p1 = NULL, *p2 = NULL;

	switch ( op->oper )
	{
		case '{':
			if ( op->nops > 0 )
			{
				nlist->addLayer();
				op->op[0] = scan(op->op[0]);
				nlist->delLayer();
			}
			return (node*)op;

		case DEFINE:	// add a macro definition
			if ( op->nops > 1 ) p1 = scan(op->op[1]), op->op[1] = NULL;
			if ( op->nops > 2 ) p2 = scan(op->op[2]), op->op[2] = NULL;
			ndp = nlist->add(op->op[0]->id.name, DEFINE, p1, p2);
			if ( ndp == NULL )
			{
				errPrint("redefined name", op->op[0]->id.name);
				delNode(p1), delNode(p2);
			}

			CLR_COMMENT(op); delNode((node*)op);
			return NULL;

		case UNDEF:		// remove a macro definition
			nlist->del(op->op[0]->id.name);
			CLR_COMMENT(op); delNode((node*)op);
			return NULL;

		case _IFDEF:
			if ( nlist->search(op->op[0]->id.name, DEFINE) )
				np = scan(op->op[1]), op->op[1] = NULL;
			else if ( op->nops > 2 )
				np = scan(op->op[2]), op->op[2] = NULL;

			CLR_COMMENT(op); delNode((node*)op);
			return np;

		case _IFNDEF:
			if ( !nlist->search(op->op[0]->id.name, DEFINE) )
				np = scan(op->op[1]), op->op[1] = NULL;
			else if ( op->nops > 2 )
				np = scan(op->op[2]), op->op[2] = NULL;

			CLR_COMMENT(op); delNode((node*)op);
			return np;

		case _IF:
			op->op[0] = scan(op->op[0]);
			if ( op->op[0] && op->op[0]->type == NODE_CON )
			{
				if ( op->op[0]->con.value )
				{
					np = scan(op->op[1]);
					op->op[1] = NULL;
				}
				else
				{
					np = scan(op->op[2]);
					op->op[2] = NULL;
				}
			}
			else
				errPrint("unsolved expr in '#if' ");

			CLR_COMMENT(op); delNode((node*)op);
			return np;

		case CALL:
			ndp = nlist->search(op->op[0]->id.name, DEFINE);
			op->op[1] = scan(op->op[1]);	// parameters...
			if ( ndp && ndp->np[1] && equListLength(op->op[1], ndp->np[0]) )
			{
				node *new_np = cloneNode(ndp->np[1]);
				for (int i = 0; op->op[1] && i < op->op[1]->list.nops; i++)
				{
					node *name_par = ndp->np[0]->list.ptr[i];
					node *real_par =  op->op[1]->list.ptr[i];
					new_np = replacePar(new_np, name_par, real_par);
				}

				new_np = scan(new_np);
				new_np->id.src = op->src;
				op->src = NULL; delNode((node*)op);
				GET_COMMENT(new_np);
				return new_np;
			}
			if ( ndp )
			{
				CLR_COMMENT(op); delNode((node*)op);
				return NULL;
			}
			break;

		case FUNC_DECL:
			// scan function returning value type
			attrRestore(&op->attr);

			np = op->op[0];	// FUNC_HEAD
			if ( np && np->type == NODE_OPR && np->opr.nops > 1 )
			{
				np = OPR_NODE(np, 1);		// parameter list
				for (int i = 0; i < np->list.nops; i++)
				{	// scan function parameter list
					node *parp = np->list.ptr[i];
					attrRestore(&parp->id.attr);
				}
			}

			op->op[1] = scan(op->op[1]);	// scan func body
			return (node*)op;

		case ENUM:
			enumNeg = 0;
			enumPos = 0;
			np = op->op[0]; // a list of exprs
			if ( np && np->type == NODE_LIST )
			{
				int  enum_v = 0;	// start default value
				char enum_s = 0;	// sign of enum value
				for (int i = 0; i < np->list.nops; i++, enum_v++)
				{
					node *p  = np->list.ptr[i];
					node *ep = p->id.init = scan(p->id.init);
					if ( ep )
					{	// enum item with init. value
						if ( ep->type == NODE_CON )
						{
							enum_v = ep->con.value;	// item value
							enum_s = ep->con.attr->isNeg;
						}
						else
							errPrint("unsolved value in enum");
					}

					if ( enum_v >= 0 ) enum_s = 0;

					if ( enum_s ) enumNeg = MIN(enumNeg, enum_v);
					else		  enumPos = MAX((unsigned int)enumPos,
												(unsigned int)enum_v);

					int type = sizer(enum_s != 0, enum_v);
					node *cp = conNode(enum_v, type);
					if ( nlist->add(p->id.name, ENUM, cp, NULL) == NULL )
					{
						errPrint("id redefined in enum", p->id.name);
						delNode(cp);
					}
				}
			}
			CLR_COMMENT(op), delNode((node*)op);
			return NULL;

		case DATA_DECL:
			ap = op->attr;
			ap->atAddr = scan((node*)ap->atAddr);
			if ( ap->type == STRUCT || ap->type == UNION )
			{
				if ( ap->newData ) 	// defined a struct/union here
				{
					if ( ap->typeName == NULL )
						ap->typeName = dupStr(tagLabel());

					ndp = nlist->add(ap->typeName, ap->type);

					if ( ndp == NULL )
						errPrint("struc/union redefined!", ap->typeName);
					else
					{	// scan the format of struct/union
						ap->newData = scan((node*)ap->newData);

						// check struct/union member names.
						if ( StUnNameCheck((node*)ap->newData, NULL) == false )
							errPrint("struct/union member id duplicated!");

						ndp->attr = cloneAttr(ap);
					}
				}
				else				// use a struct/union here
				{
					attrRestore(&op->attr, true);
					ap = op->attr;
				}
			}
			else if ( ap->type == TYPEDEF_NAME )
			{
				attrRestore(&op->attr, true);
				ap = op->attr;
			}

			if ( op->nops > 0 )
			{
				int mem_addr = -1;
				if ( ap->atAddr && ((node*)ap->atAddr)->type == NODE_CON )
					mem_addr = ((node*)ap->atAddr)->con.value;

				np = op->op[0];	// list of variables
				for (int i = 0; i < np->list.nops; i++)
				{
					node *inp = scan(np->list.ptr[i]);
					node *dnp = oprNode(DATA_DECL, 1);	// create a data node.

					dnp->opr.attr = cloneAttr(ap);

					if ( inp->id.attr )
					{
						dnp->opr.attr->ptrVect = inp->id.attr->ptrVect;
						inp->id.attr->ptrVect  = NULL;
						delAttr(inp->id.attr); inp->id.attr = NULL;
					}
					inp->id.parp = scan(inp->id.parp);
					inp->id.dim  = scan(inp->id.dim);

					dnp->opr.attr->dimVect= makeDim(inp->id.dim);
					dnp->opr.attr->isFptr = inp->id.fp_decl;
					dnp->opr.attr->parList= inp->id.parp;
					inp->id.parp = NULL;

					dnp->opr.op[0] = inp;
					np->list.ptr[i] = dnp;

					if ( mem_addr >= 0 )
					{
						delNode((node*)dnp->opr.attr->atAddr);
						dnp->opr.attr->atAddr = conNode(mem_addr, INT);
						mem_addr += sizer(dnp->opr.attr, TOTAL_SIZE);
					}

					/* for data error:  'void a;' */
					if ( !(dnp->opr.attr->isFptr || dnp->opr.attr->ptrVect) &&
					     dnp->opr.attr->type == VOID 					  )
						errPrint("invalid data type: ", inp->id.name);
				}

				op->op[0] = NULL;
				delNode((node*)op);
				return np;
			}

			return (node*)op;

		case TYPEDEF:
			ap = op->attr;
			ap->newData = scan((node*)ap->newData);

			if ( ap->type == STRUCT || ap->type == UNION )
			{
				if ( !StUnNameCheck((node*)ap->newData, NULL) )
					errPrint("struct/union member id duplicated!");
			}

			np  = op->op[0];
			ndp = nlist->add(np->id.name, TYPEDEF);

			if ( ndp )
			{
				if ( ap->type == ENUM )
				{
					int neg_size = sizer(enumNeg < 0, enumNeg);
					int pos_size = sizer(enumNeg < 0, enumPos);
					ndp->attr = newAttr(MAX(neg_size, pos_size));
					ndp->attr->isUnsigned = enumNeg? 0: 1;
				}
				else
				{
					attrib *aap = nlist->search(ap);
					if ( aap )
						ndp->attr = cloneAttr(aap);
					else
						errPrint("typedef name not defined!", ap->typeName);
				}
			}
			else
				errPrint("redefined type_name!", np->id.name);

			delNode((node*)op);
			return NULL;

		case SIZEOF:
			op->op[0] = scan(op->op[0]);
			if ( op->nops == 0 )
			{
				int n = 0;
				ap = op->attr;
				switch ( ap->type )
				{
					case TYPEDEF_NAME:
						ndp = nlist->search(ap->typeName, TYPEDEF);
						if ( ndp ) ap = ndp->attr;
						else errPrint("sizeof type error!", ap->typeName);
						break;

					case STRUCT:
					case UNION:
						if ( ap->newData == NULL )
						{
							ndp = nlist->search(ap->typeName, ap->type);
							if ( ndp ) ap = ndp->attr;
							else errPrint("sizeof type error!", ap->typeName);
						}
						break;
				}

				if ( ap == NULL )
					errPrint("data type not defined!");
				else {
					n = sizer(ap, ATTR_SIZE);
					if ( n < 0 )
						errPrint("sizeof type error!");
				}

				CLR_COMMENT(op), delNode((node*)op);
				return conNode(n, INT);
			}
			else if ( op->op[0]->type == NODE_STR )
			{
				int n = strlen(op->op[0]->str.str);
				CLR_COMMENT(op), delNode((node*)op);
				return conNode(n+1, INT);
			}
			break;

		case CAST:
			attrRestore(&op->attr);
			break;

		case SBIT:
			if ( op->nops == 1 )
				return (node*)op;

			np        = op->op[0];
			ndp       = nlist->add(np->id.name, SBIT);
			op->op[1] = scan(op->op[1]);

			if ( ndp == NULL )
				errPrint("sbit name redefined!", np->id.name);
			else
			{
				ndp->np[0] = op->op[1];
				op->op[1]  = NULL;
			}
			CLR_COMMENT(op), delNode((node*)op);
			return NULL;

		case PRAGMA:
			if ( op->nops > 1 )
				op->op[1] = scan(op->op[1]);
			return (node*)op;
	}

	for (int i = 0; i < op->nops; i++)
		op->op[i] = scan(op->op[i]);

	switch ( op->oper )
	{
		case '+': case '-': case '*': case '/': case '%':
		case '&': case '|': case '^':
		case LEFT_OP: case RIGHT_OP:
		case EQ_OP: case NE_OP: case AND_OP: case OR_OP:
		case '>': case '<':	case LE_OP:	case GE_OP:
			if ( op->op[0]->type == NODE_CON &&
				 op->op[1]->type == NODE_CON )
			{
				int n = mergeCon(op->op[0]->con.value,
								 op->op[1]->con.value, op->oper);
				CLR_COMMENT(op); delNode((node*)op);
				return conNode(n, INT);
			}
			break;

		case '~':
		case '!':
		case NEG_OF:
			if ( op->op[0]->type == NODE_CON )
			{
				int n = op->op[0]->con.value;
				n = (op->oper == '~')? ~n:
				    (op->oper == '!')? (n? 0:1):
									   -n;

				np = op->op[0]; op->op[0] = NULL;
				np->con.value = n;
				if ( op->oper == NEG_OF ) np->con.attr->isNeg ^= 1;
				CLR_COMMENT(op); delNode((node*)op);
				return np;
			}
			break;
	}
	return (node*)op;
}

int PreScan :: mergeCon (int n1, int n2, int oper)
{
	switch ( oper )
	{
		case '+':	return n1 + n2;
		case '-':	return n1 - n2;
		case '*':	return n1 * n2;
		case '/':	return n1 / n2;
		case '%':	return n1 % n2;
		case '&':	return n1 & n2;
		case '|':	return n1 | n2;
		case '^':	return n1 ^ n2;
		case LEFT_OP: return n1 << n2;
		case RIGHT_OP: return n1 >> n2;
		case EQ_OP: return (n1 == n2)? 1: 0;
		case NE_OP: return (n1 != n2)? 1: 0;
		case AND_OP:return (n1 && n2)? 1: 0;
		case OR_OP: return (n1 || n2)? 1: 0;
		case '<':	return (n1 < n2)? 1: 0;
		case '>':	return (n1 > n2)? 1: 0;
		case LE_OP:	return (n1 <= n2)? 1: 0;
		case GE_OP:	return (n1 >= n2)? 1: 0;
	}
	return 0;
}

node *PreScan :: replacePar(node *np, node *name_par, node *real_par)
{
	if ( np )
		switch ( np->type )
		{
			case NODE_ID:
				if ( strcmp(np->id.name, name_par->id.name) == 0 )
				{
					CLR_COMMENT(np); delNode(np);
					return cloneNode(real_par);
				}
				break;

			case NODE_OPR:
				for (int i = 0; i < np->opr.nops; i++)
					np->opr.op[i] = replacePar(np->opr.op[i], name_par, real_par);
				break;

			case NODE_LIST:
				for (int i = 0; i < np->list.nops; i++)
					np->list.ptr[i] = replacePar(np->list.ptr[i], name_par, real_par);
				break;
		}

	return np;
}

void PreScan :: errPrint(const char *msg, char *extra)
{
	printf ("[ERROR] %s", msg);
	if ( extra ) printf(" - %s", extra);
	if ( src ) printf(" ... %s, #%d", src->fileName, src->lineNum);
	printf("\n");
	errCount++;
}

char *PreScan :: tagLabel(void)
{
	static char buf[32];
	sprintf(buf, "_??%d_", tagSeed++);
	return buf;
}

void PreScan :: attrRestore(attrib **attr, bool update_flags)
{
	attrib *ap = *attr;
	Nnode *ndp = NULL;

	if ( ap == NULL ) return;

	switch ( ap->type )
	{
		case TYPEDEF_NAME:
			ndp = nlist->search(ap->typeName, TYPEDEF);
			if ( ndp == NULL )
			{
				errPrint("type name undefined!", ap->typeName);
				return;
			}
			break;

		case STRUCT:
		case UNION:
			if ( ap->typeName == NULL && !ap->isTypedef )
				return;

			ndp = nlist->search(ap->typeName, ap->type);
			if ( ndp == NULL )
			{
				errPrint("struct/union undefined!", ap->typeName);
				return;
			}
			break;

		default:
			return;
	}

	attrib *new_ap = cloneAttr(ndp->attr);
	new_ap->ptrVect = ap->ptrVect;
	ap->ptrVect = NULL;
	if ( update_flags )
	{
		new_ap->isExtern   = ap->isExtern;
		new_ap->isStatic   = ap->isStatic;
		new_ap->isVolatile = ap->isVolatile;
		new_ap->dataBank   = ap->dataBank;
		moveNode((node**)&new_ap->atAddr, (node**)&ap->atAddr);
	}

	delAttr(ap);
	new_ap->isTypedef = 0;
	*attr = new_ap;
}

bool PreScan :: StUnNameCheck(node *np, char *name)
{
	for (int i = 0; i < LIST_LENGTH(np); i++)
	{
		node *nnp = LIST_NODE(np, i);
		for (int j = 0; j < LIST_LENGTH(nnp); j++)
		{
			node *inp = LIST_NODE(nnp, j);
			char *id  = inp->opr.op[0]->id.name;

			if ( name == NULL )
			{
				if ( StUnNameCheck(np, id) == false )
					return false;
			}
			else if ( name != id )
			{
				if ( strcmp(name, id) == 0 )
					return false;
			}
		}
	}
	return true;
}
