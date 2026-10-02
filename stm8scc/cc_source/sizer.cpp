#include <stdio.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "option.h"
#include "sizer.h"
#include "nlist.h"

Sizer sizer;

int Sizer :: size(attrib *ap, SIZER_OPTION opt)
{
	int n = 0;

	if ( opt == INDIR_SIZE )
	{
		if ( ptrWeight(ap) > 0 )
		{
			attrib *attr = cloneAttr(ap);
			reducePtr(attr);
			n = size(attr, TOTAL_SIZE);
			delAttr(attr);
			return n;
		}
	}
	if ( opt == SUBDIM_SIZE )
	{
		attrib *attr = cloneAttr(ap);
		decDim(attr);
		n = size(attr, TOTAL_SIZE);
		delAttr(attr);
		return n;
	}

	int dim_size = (opt == ATTR_SIZE)? 1: dimScale(ap);

	if ( ap->ptrVect || ap->isFptr )
	{
		n = 2;
		if ( EXT_MODE && (ap->isFptr || (ap->ptrVect && ap->ptrVect[1] == CONST)) )
			n = 3;
	}
	else
		switch ( ap->type )
		{
			case CHAR:	n = (BF_OFFSET(ap) && opt == TOTAL_SIZE)? 0: 1;	break;
			case INT:
			case SHORT:
			case LONG:	n = ap->type - CHAR + 1;	break;

			case STRUCT:
			case UNION:
				if ( ap->newData )
				{
					n = size((node*)ap->newData, ap->type, opt == TOTAL_SIZE);
					break;
				}
			default:
				n = -1;
				if ( ap->type )
					printf("something wrong! - %d\n", ap->type); // it should never happen!
		}

	return n * dim_size;
}

int	Sizer :: size(bool neg_val, int val)
{
	unsigned int value = (unsigned int)val;

	if ( neg_val )
	{
		if ( val < 0 )
			return (val >= -128    )? CHAR:
				   (val >= -32768  )? INT:
				   (val >= -8388608)? SHORT: LONG;

		return (val < 128	 )? CHAR:
			   (val < 32768  )? INT:
			   (val < 8388608)? SHORT: LONG;
	}

	return (value <= 0xffU	  )?  CHAR:
		   (value <= 0xffffU  )?  INT:
		   (value <= 0xffffffU)?  SHORT: LONG;
}

int Sizer :: size(node *np, int atype, bool total_size)
{
	int n = 0, v;

	switch ( np->type )
	{
		case NODE_LIST:
			for (int i = 0; i < LIST_LENGTH(np); i++)
			{
				v = size(LIST_NODE(np, i), atype, total_size);
				if ( atype == STRUCT ) n += v;
				else if ( n < v )	   n  = v;
			}
			break;

		case NODE_OPR:
			if ( np->opr.oper == DATA_DECL )
			{
				attrib *attr = np->opr.attr;
				v = size(attr, TOTAL_SIZE);

				if ( atype == STRUCT ) n += v;
				else if ( n < v )  	   n  = v;
				break;
			}
			// fall through here
		default:// it should never happen
			printf ("node type : %d\n", np->type);
	}
	return n;
}

int Sizer :: memberCount(attrib *ap)
{
	if ( ap->type != STRUCT && ap->type != UNION )
		return 1;

	int n = 0;
	node *np = (node*)ap->newData;
	for (int i = 0; i < LIST_LENGTH(np); i++)
	{
		node *pp = LIST_NODE(np, i);
		n += LIST_LENGTH(pp);
	}
	return n;
}

attrib *Sizer :: memberAttr(attrib *ap, int index)
{
	if ( ap->type == STRUCT )
	{
		node *np = (node*)ap->newData;
		for (int i = 0; i < LIST_LENGTH(np); i++)
		{
			node *pp = LIST_NODE(np, i);
			for (int j = 0; j < LIST_LENGTH(pp); j++)
			{
				node *ppp = LIST_NODE(pp, j);	// variable id list
				if ( index-- <= 0 )
					return ppp->opr.attr;
			}
		}
	}
	return NULL;
}

attrib *Sizer :: memberAttr(attrib *ap, char *id_name)
{
	if ( ap->type == STRUCT || ap->type == UNION )
	{
		node *np = (node*)ap->newData;
		for (int i = 0; i < LIST_LENGTH(np); i++)
		{
			node *pp = LIST_NODE(np, i);
			for (int j = 0; j < LIST_LENGTH(pp); j++)
			{
				node *ppp = LIST_NODE(pp, j);	// variable id list
				node *inp = OPR_NODE(ppp, 0);

				if ( strcmp(inp->id.name, id_name) == 0 )
					return ppp->opr.attr;
			}
		}
	}
	return NULL;
}

attrib *Sizer :: memberAttrClone(attrib *ap, int index)
{
	if ( ap->type == STRUCT )
	{
		node *np = (node*)ap->newData;
		for (int i = 0; i < LIST_LENGTH(np); i++)
		{
			node *pp = LIST_NODE(np, i);
			for (int j = 0; j < LIST_LENGTH(pp); j++)
			{
				node *ppp = LIST_NODE(pp, j);	// variable id list
				if ( index-- <= 0 )
					return cloneAttr(ppp->opr.attr);
			}
		}
	}
	return NULL;
}

attrib *Sizer :: memberAttrClone(attrib *ap, char *id_name)
{
	if ( ap->type == STRUCT || ap->type == UNION )
	{
		node *np = (node*)ap->newData;
		for (int i = 0; i < LIST_LENGTH(np); i++)
		{
			node *pp = LIST_NODE(np, i);
			for (int j = 0; j < LIST_LENGTH(pp); j++)
			{
				node *ppp = LIST_NODE(pp, j);	// variable id list
				node *inp = OPR_NODE(ppp, 0);
				if ( strcmp(inp->id.name, id_name) == 0 )
					return cloneAttr(ppp->opr.attr);
			}
		}
	}
	return NULL;
}

int Sizer :: memberOffset(attrib *ap, char *id_name)
{
	if ( ap->type == STRUCT )
	{
		int offset = 0;
		node *np = (node*)ap->newData;
		for (int i = 0; i < LIST_LENGTH(np); i++)	// lines of declair
		{
			node *pp = LIST_NODE(np, i);
			for (int j = 0; j < LIST_LENGTH(pp); j++)
			{
				node *ppp = LIST_NODE(pp, j);		// variable id list
				node *inp = OPR_NODE(ppp, 0);

				if ( strcmp(inp->id.name, id_name) == 0 ) // found member
					return (BF_OFFSET(ppp->opr.attr) > 0)? offset - 1: offset;

				offset += size(ppp->opr.attr, TOTAL_SIZE);
			}
		}
	}
	return 0;
}
