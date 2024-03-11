#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "item.h"
#include "sizer.h"

static int conAttr(Item *ip);

//////////////////////////////////////////////////////////////////////
Item :: Item(ITEM_TYPE t)
{
	memset(this, 0, sizeof(Item));
	type = t;
}

//////////////////////////////////////////////////////////////////////
Item :: ~Item()
{
	if ( type == STR_ITEM || type == ID_ITEM   ||
		 type == LBL_ITEM || type == IMMD_ITEM || type == LBL_ITEM )
		free(val.s);

	delAttr(attr);
}

Item *Item :: clone(void)
{
	Item *ip = new Item(type);

	ip->attr = cloneAttr(attr);
	ip->home = home;
	ip->bias = bias;

	switch ( ip->type )
	{
		case ID_ITEM:	case IMMD_ITEM:	case STR_ITEM:	case LBL_ITEM:	case PID_ITEM:
			ip->val.s = dupStr(val.s);
			break;

        case PTR_ITEM:
            ip->val.p = val.p;
            break;

        default:
        	ip->val.i = val.i;
			break;
	}
	return ip;
}

void Item :: updateAttr(attrib *ap, int data_bank)
{
	delAttr(attr);
	attr = ap;
	if ( attr && type == TEMP_ITEM )
		attr->isStatic = attr->isExtern = 0;
	if ( attr && data_bank != -1 )
		attr->dataBank = data_bank;
}

void Item :: updatePtr (int *p)
{
	if ( attr )
	{
		deletePtr(attr);
		attr->ptrVect = p;
	}
}

void Item :: updateName(char *new_name)
{
	if ( type == ID_ITEM || type == IMMD_ITEM || type == LBL_ITEM )
	{
		free(val.s);
		val.s = dupStr(new_name);
	}
}

bool Item :: isWritable(void)
{
	switch ( type )
	{
		case TEMP_ITEM:
		case ACC_ITEM:
			return true;

		case ID_ITEM:
			return (attr->dataBank == CONST)? false: true;

		case PID_ITEM:
		case INDIR_ITEM:
		case DIR_ITEM:
			if ( attr->ptrVect[1] != CONST ) return true;

		default:
			return false;
	}
}

bool Item :: isOperable(void)
{
	if ( isMonoVal() || type == STR_ITEM )
		return true;

	if ( attr == NULL || (attr->dimVect && !attr->ptrVect) )
		return false;

	if ( attr->isFptr )
		return true;

	switch ( type )
	{
		case INDIR_ITEM:	// indirect address Item
		case PID_ITEM:		// pointer variable Item
		case DIR_ITEM:		// direct address Item
			return !(ptrWeight(attr) == 1 && (attr->type == STRUCT || attr->type == UNION));

		case TEMP_ITEM:
		case ACC_ITEM:
		case ID_ITEM:
			return !(ptrWeight(attr) == 0 && (attr->type == STRUCT || attr->type == UNION));

		default:
			return false;
	}
}

bool Item :: isMonoVal(void)
{
    if ( type == CON_ITEM || type == IMMD_ITEM || type == LBL_ITEM )
		return true;

	if ( attr == NULL || dimDepth(attr) > 0 || attr->type == SBIT )
		return false;

	if ( type == DIR_ITEM || type == PID_ITEM || type == INDIR_ITEM )
	{
		if ( ptrWeight(attr) > 1 )
			return false;

		return (attr->type >= CHAR && attr->type <= LONG);
	}

	if ( ptrWeight(attr) || type == IMMD_ITEM )
		return false;

	return (attr->type >= CHAR && attr->type <= LONG);
}

bool Item :: isAccePtr(void)
{
	if ( dimDepth(attr) != 0 )
		return false;

	switch ( type )
	{
		case CON_ITEM:
		case ID_ITEM:
		case TEMP_ITEM:
		case ACC_ITEM:
			return (ptrWeight(attr) > 0);

		case DIR_ITEM:
		case PID_ITEM:
		case INDIR_ITEM:
			return (ptrWeight(attr) > 1);

		default:
			return false;
	}
}

bool Item :: isBitVal(void)
{
	switch ( type )
	{
		case CON_ITEM:
			return (val.i == 1 || val.i == 0);

		default:
			if ( attr && attr->type == SBIT )
				return true;
	}
	return false;
}

attrib *Item :: acceAttr(void)
{
	attrib *ap;
	switch ( type )
	{
		case ID_ITEM:
		case TEMP_ITEM:
		case ACC_ITEM:
			return cloneAttr(attr);

		case INDIR_ITEM:
		case PID_ITEM:
		case DIR_ITEM:
			ap = cloneAttr(attr);
			reducePtr(ap);
			return ap;

		default:
			return NULL;
	}
}

int	Item :: stepSize(void)
{
	int ptr_weight = ptrWeight(attr);
	bool indirect = false;
	switch ( type )
	{
		case IMMD_ITEM:
			return sizer(attr, ATTR_SIZE);
		case DIR_ITEM:
			if ( attr->type == SBIT ) return 0;
		case INDIR_ITEM:
		case PID_ITEM:
			ptr_weight--;
			indirect = true;
		default:
			break;
	}

	if ( ptr_weight > 1 || attr->isFptr )
		return 2;

	if ( !(ptr_weight || indirect) && attr->dimVect )
		return 0;

	bool pointed = (ptr_weight || type == IMMD_ITEM);
	switch ( attr->type )
	{
		case CHAR:	return 1;
		case INT:	return pointed? 2: 1;
		case SHORT:	return pointed? 3: 1;
		case LONG:	return pointed? 4: 1;
		case STRUCT:
		case UNION:	if ( pointed )
						return sizer(attr, INDIR_SIZE);
		default:	return 0;
	}
}

int Item :: storSize(void)
{
	switch ( type )
	{
		case ACC_ITEM:
		case TEMP_ITEM:
			return attr->ptrVect? 2: sizer(attr, ATTR_SIZE);

		case ID_ITEM:
			return sizer(attr, TOTAL_SIZE);

		case INDIR_ITEM:
		case PID_ITEM:
			return 2;

		default:
			return 0;
	}
}

int Item :: acceSize (void)
{
	switch ( type )
	{
		case LBL_ITEM:
		case STR_ITEM:
		case IMMD_ITEM:
			return 2;

		case CON_ITEM:
			return conAttr(this) - CHAR + 1;

		case TEMP_ITEM:
			if ( attr->ptrVect ) return 2;
		case ID_ITEM:
		case ACC_ITEM:
			return sizer(attr, ATTR_SIZE);

		case DIR_ITEM:
		case PID_ITEM:
		case INDIR_ITEM:
			return sizer(attr, INDIR_SIZE);

		default:
			break;
	}
	return 0;
}

int Item :: acceSign (void)
{
	switch ( type )
	{
		case CON_ITEM:
			return (attr && attr->isNeg)? 1: 0;

		case TEMP_ITEM:
		case ID_ITEM:
		case ACC_ITEM:
			if ( ptrWeight(attr) > 0 )
				return 0;

			if ( isMonoVal() )
				return attr->isUnsigned? 0: 1;

			break;

		case DIR_ITEM:
		case PID_ITEM:
		case INDIR_ITEM:
			if ( ptrWeight(attr) > 1 )
				return 0;

			if ( isMonoVal() )
				return attr->isUnsigned? 0: 1;
			break;

		default:
			break;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
//	making an Item
//////////////////////////////////////////////////////////////////////
Item *intItem(int val, attrib *ap)
{
	Item *ip = new Item(CON_ITEM);
	ip->val.i = val;
	ip->attr  = ap? ap: newAttr(INT);
	return ip;
}

Item *strItem(char *str)
{
	Item *ip = new Item(STR_ITEM);
	ip->val.s = dupStr (str);
	return ip;
}

Item *lblItem(int lbl)
{
	Item *ip = new Item(LBL_ITEM);
	ip->val.s = (char *)MALLOC (16);
	sprintf (ip->val.s, "_$L%d", lbl);
	return ip;
}

Item *lblItem(char *str)
{
	Item *ip = new Item(LBL_ITEM);
	ip->val.s = dupStr(str);
	return ip;
}

Item *idItem(char *name, attrib *attr)
{
	Item *ip = new Item(ID_ITEM);
	ip->val.s = dupStr(name);
	ip->attr = cloneAttr(attr);
	return ip;
}

Item *ptrItem(void *ptr)
{
	Item *ip = new Item(PTR_ITEM);
	ip->val.p = ptr;
	return ip;
}

Item *immdItem(char *str, attrib *attr)
{
	Item *ip = new Item(IMMD_ITEM);
	ip->val.s = dupStr(str);
	ip->attr = cloneAttr(attr);
	return ip;
}

Item *tmpItem(int index, attrib *attr)
{
	Item *ip  = new Item(TEMP_ITEM);
	ip->val.i = index;
	ip->attr  = attr;
	return ip;
}

Item *accItem(attrib *attr)
{
	Item *ip = new Item(ACC_ITEM);
	ip->attr = attr;
	return ip;
}

bool moveMatch(Item *ip0, Item *ip1)
{
	attrib *ap0  = ip0->acceAttr();
	attrib *ap1  = ip1->acceAttr();
	int ptrVect0 = ap0? ptrWeight(ap0) + dimDepth(ap0): 0;
	int ptrVect1 = ap1? ptrWeight(ap1) + dimDepth(ap1): 0;
	bool retVal  = true;

	if ( ip0->isOperable() && ip1->isOperable() )
		return true;

	if ( ap0 && ap0->dimVect )
	{
		retVal = false;
	}
	else if ( ptrVect0 && ptrVect1 )	// remove pointer between
	{
		retVal = true;
	}
	else if ( ap0 && ap1 )
	{
        if ( ap0->dimVect || ap1->dimVect )
        {
            retVal = false; // can't move an array
        }
        else if ( (ap0->ptrVect || ap0->isFptr) &&
                  (ap1->ptrVect || ap1->isFptr) )
        {
            retVal = true;
		}
		else if ( !(ap0->ptrVect || ap0->isFptr) &&
				  !(ap1->ptrVect || ap1->isFptr) )
		{
            if ( ap0->type == ap1->type &&
                (ap0->type == STRUCT || ap0->type == UNION) )
			{
                if ( memberCount(ap0) != memberCount(ap1) )
					retVal = false;
				else
				{
					for (int i = 0; i < memberCount(ap0); i++)
					{
						attrib *a0 = memberAttr(ap0, i);
						attrib *a1 = memberAttr(ap1, i);
						if ( sizer(a0, TOTAL_SIZE) != sizer(a1, TOTAL_SIZE) )
							retVal = false;
					}
				}
			}
			else if ( ap0->type == STRUCT || ap0->type == UNION ||
					  ap1->type == STRUCT || ap1->type == UNION )
			{
				retVal = false;
			}
		}
	}
	else if ( ap0 || ap1 )
	{
		attrib *ap = ap1? ap1: ap0;

		if ( !(ap->ptrVect || ap->isFptr) &&
			 (ap->type == STRUCT || ap->type == UNION) )
			retVal = false;
	}
	delAttr(ap0), delAttr(ap1);
	return retVal;
}


static int conAttr(Item *ip)
{
	return sizer(ip->attr->isNeg? true: false, ip->val.i);
}

attrib *maxMonoAttr(Item *ip0, Item *ip1, int op)
{
	int t0   = (ip0->type == CON_ITEM)? conAttr(ip0): ip0->attr->type;
	int t1   = (ip1->type == CON_ITEM)? conAttr(ip1): ip1->attr->type;
	bool us0 = ip0->acceSign()? false: true;
	bool us1 = ip1->acceSign()? false: true;

	int t = (t0 >= t1)? t0: t1;
	if ( op == '&' )
	{
		if ( ip0->type == CON_ITEM ) t = t1;
		if ( ip1->type == CON_ITEM ) t = t0;
	}
	else if ( op == '*' )	// extend size for '*' operation
	{
		t = t0 + t1 - 2*(CHAR-1);
		if ( t > 4 ) t = 4;
		t += CHAR - 1;
	}
	else if ( op == '/' || op == '%' || op == RIGHT_OP )
		t = t0;
	else if ( op == '+' || op == '-' )
	{
		if ( ++t > LONG ) t = LONG;
	}

	attrib *ap = newAttr(t);
	ap->isUnsigned = (us0 && us1)? 1: 0;
	return ap;
}

attrib *maxSizeAttr(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	int sign0 = ip0->acceSign();
	int sign1 = ip1->acceSign();
	if ( size0 < size1 ) size0 = size1;
	if ( size0 == 0 || size0 > 4 ) return NULL;

	attrib *attr = newAttr(size0+CHAR-1);
	attr->isUnsigned = (sign0 || sign1)? 1: 0;
	return attr;
}

bool comparable(int code, Item *ip0, Item *ip1)
{
	if ( ip0->isMonoVal() && ip1->isMonoVal() )
		return true;

	if ( ip0->isBitVal() && ip1->isBitVal() )
		return (code == EQ_OP || code == NE_OP);

	if ( ip0->isOperable() && ip1->isOperable() )
		return (code == EQ_OP || code == NE_OP);

	return false;
}

bool same(Item *ip0, Item *ip1, bool fsr_check)
{
	if ( ip0 == NULL && ip1 == NULL ) return true;
	if ( ip0 == NULL || ip1 == NULL ) return false;
	if ( ip0->type != ip1->type ) return false;

	if ( !fsr_check )
	{
		if ( cmpAttr(ip0->attr, ip1->attr) || ip0->bias != ip1->bias )
			return false;
	}

	switch ( ip0->type )
	{
		case CON_ITEM:
		case TEMP_ITEM:
		case DIR_ITEM:
		case INDIR_ITEM:	return (ip0->val.i == ip1->val.i);

		case PID_ITEM:
		case IMMD_ITEM:
		case LBL_ITEM:
		case ID_ITEM:
		case STR_ITEM:		return strcmp(ip0->val.s, ip1->val.s)? false: true;
		case ACC_ITEM:		return true;
        default:            return false;
	}
	return false;
}

bool sameTemp(Item *ip0, Item *ip1)
{
	if ( ip0 && ip1 )
	{
		if ( ip0->type == TEMP_ITEM && ip1->type == TEMP_ITEM )
			return (ip0->val.i == ip1->val.i);
	}
	return false;
}

bool overlap(Item *ip0, Item *ip1)
{
	if ( ip0->type != ip1->type || ip0->acceSize() != ip1->acceSize() )
		return false;

	if ( same(ip0, ip1) || sameTemp(ip0, ip1) )
		return true;

	return (ip0->type == ACC_ITEM);
}

bool related(Item *ip0, Item *ip1)
{
	if ( !(ip0 && ip1) )
		return false;

	if ( overlap(ip0, ip1) )
		return true;

	if ( (ip0->type == TEMP_ITEM || ip0->type == INDIR_ITEM) &&
		 (ip1->type == TEMP_ITEM || ip1->type == INDIR_ITEM) )
		return (ip0->val.i == ip1->val.i);

	if ( (ip0->type == ID_ITEM || ip0->type == PID_ITEM) &&
		 (ip1->type == ID_ITEM || ip1->type == PID_ITEM) )
		return (strcmp(ip0->val.s, ip1->val.s) == 0)? true: false;

	return false;
}
