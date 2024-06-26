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

static char *expVal(item_t *ip, Segment *seg, Symbol *slist, int addr, int *val);
static int   expVal(int v0, int v1, int type);

///////////////////////////////////////////////////////////////////////////////////////
char *expValue(item_t *ip, Segment *seg, Symbol *slist, int *val, int addr)
{
	char *lbl0, *lbl1;
	int  val0, val1;

	if ( ip == NULL ) return NULL;

	switch ( ip->type )
	{
		case ':':
			lbl0 = expVal(ip->left,  seg, slist, addr, &val0);
			lbl1 = expVal(ip->right, seg, slist, addr, &val1);
			if ( val1 >= 0x2000 ) val1 = TO_BANKED_ADDR(val1);
			*val = (val0 & ~0x7f) | (val1 & 0x7f);
			return lbl0? lbl0: lbl1;

		default:
			return expVal(ip, seg, slist, addr, val);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
static char *expVal(item_t *ip, Segment *seg, Symbol *slist, int addr, int *val)
{
	Symbol *symb;
	char *lbl0, *lbl1;
	int  val0, val1;

	*val = 0;
	if ( ip == NULL ) return NULL;

	switch ( ip->type )
	{
		case TYPE_VALUE:
			*val = ip->data.val;
			return NULL;

		case TYPE_SYMBOL:
			symb = searchSymbol(slist, ip->data.str, seg);
			if ( symb )	*val = symb->value;

			// if 'symbol' not found in the 'slist', report it.
			return symb? NULL: ip->data.str;

		case '.':
			*val = addr;
			return NULL;

		case '+':	case '-':	case '*':	case '/':	case '%':
		case '|':	case '^':	case '&':	case RSHIFT:	case LSHIFT:
			lbl0 = expVal(ip->left,  seg, slist, addr, &val0);
			lbl1 = expVal(ip->right, seg, slist, addr, &val1);
			*val = expVal(val0, val1, ip->type);
			return lbl0? lbl0: lbl1;

		case '~':
			return expVal(ip->left,  seg, slist, addr, val);
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
		default:		return 0;
	}
}