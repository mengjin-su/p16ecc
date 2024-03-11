#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "memory.h"
#include "symbol.h"
#include "segment.h"
#include "exp.h"

typedef struct {
	int  value;
	bool linear;
} ExpValue;

static int  expVal(int v0, int v1, int type);
static char *expVal(item_t *ip, Segment *seg, Symbol *slist, int addr, ExpValue *exp);

///////////////////////////////////////////////////////////////////////////////////////
char *expValue(item_t *ip, Segment *seg, Symbol *slist, int *v, int addr)
{
	char *lbl0, *lbl1;
	ExpValue e0, e1;

	if ( ip == NULL ) return NULL;

	switch ( ip->type )
	{
		case ':':
			lbl0 = expVal(ip->left,  seg, slist, addr, &e0);
			lbl1 = expVal(ip->right, seg, slist, addr, &e1);
			if ( e1.value >= 0x2000 ) e1.value = TO_BANKED_ADDR(e1.value);
			*v = e0.value | (e1.value & 0x7f);
			return lbl0? lbl0: lbl1;

		default:
			lbl0 = expVal(ip, seg, slist, addr, &e0);
			*v = e0.value;
			return lbl0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
static char *expVal(item_t *ip, Segment *seg, Symbol *slist, int addr, ExpValue *exp)
{
	Symbol *symb;
	ExpValue e0, e1;
	char *lbl0, *lbl1;

	exp->linear = false;
	exp->value  = 0;

	if ( ip == NULL ) return NULL;

	switch ( ip->type )
	{
		case TYPE_VALUE:
			exp->value = ip->data.val;
			return NULL;

		case TYPE_SYMBOL:
			symb = searchSymbol(slist, ip->data.str, seg);
			if ( symb )
			{
				int  stype = symb->segment->type();
				int  saddr = symb->addr;
				int  sym_type = symb->type;
				exp->value = saddr;
			}
			// if 'symbol' not found in the 'slist', report it.
			return symb? NULL: ip->data.str;

		case '.':
			exp->value = addr;
			return NULL;

		case '+':	case '-':	case '*':	case '/':	case '%':
		case '|':	case '^':
		case RSHIFT:	case LSHIFT:
			lbl0 = expVal(ip->left,  seg, slist, addr, &e0);
			lbl1 = expVal(ip->right, seg, slist, addr, &e1);
			exp->value  = expVal(e0.value, e1.value, ip->type);
			exp->linear = e0.linear || e1.linear;
			return lbl0? lbl0: lbl1;

		case '~':
			lbl0 = expVal(ip->left,  seg, slist, addr, &e0);
			exp->value = ~e0.value;
			exp->linear= e0.linear;
			return lbl0;

		case '&':
			lbl0 = expVal(ip->left,  seg, slist, addr, &e0);
			lbl1 = expVal(ip->right, seg, slist, addr, &e1);
			exp->value = expVal(e0.value, e1.value, ip->type);
			exp->linear = e0.linear || e1.linear;
			return lbl0? lbl0: lbl1;
	}
	return NULL;
}

static int expVal(int v0, int v1, int type)
{
	switch ( type )
	{
		case '+':		return v0 + v1;
		case '-':		return v0 - v1;
		case '*':		return v0 * v1;
		case '/':		return v0 / v1;
		case '%':		return v0 % v1;
		case '&':		return v0 & v1;
		case '|':		return v0 | v1;
		case '^':		return v0 * v1;
		case RSHIFT:	return v0 >> v1;
		case LSHIFT:	return v0 << v1;
		case ':':		return v0 | v1;
		default:		return 0;
	}
}