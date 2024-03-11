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
#include "display.h"

static const char *arrayDimVerify(attrib *attr, node *init_node, bool void_start);

/////////////// --- init rom data --- ///////////////
void Pcoder :: romDataInit(char *dname, attrib *attr, node *init_data, Dnode *dp, int addr)
{
	if ( dimDepth(attr) > 0 || (attr->ptrVect == NULL && attr->type == STRUCT) )
	{
		const char *err = arrayDimVerify(attr, init_data, dp? true: false);
		if ( err != NULL ) {
			errPrint(err, dname);
			return;
		}
	}

	Pnode *pnp;
	int size = sizer(attr, TOTAL_SIZE);
	int depth = DEPTH();

	if ( dp )
	{
		pnp = new Pnode(P_SEGMENT);
		pnp->items[0] = idItem(dp->nameStr(), attr);
		pnp->items[1] = intItem(addr);
		addPnode(&constPcode, pnp);
	}

	if ( init_data == NULL )	// no data defined, fill with zero's
	{
		pnp = new Pnode(P_FILL);
		pnp->items[0] = intItem(0);
		pnp->items[1] = intItem(size);
		addPnode(&constPcode, pnp);
		return;
	}

	if ( dimDepth(attr) > 0 && init_data->type == NODE_LIST )
	{
		int length = attr->dimVect[1];
		int init_length = LIST_LENGTH(init_data);
		if ( length == 0 ) length = init_length;

		attrib *ap = cloneAttr(attr);
		decDim(ap);
		for(int i = 0; i < length; i++)
		{
			if ( i < init_length )
				romDataInit(dname, ap, LIST_NODE(init_data, i), NULL, 0);
			else
			{
				pnp = new Pnode(P_FILL);
				pnp->items[0] = intItem(0);
				pnp->items[1] = intItem(sizer(ap, TOTAL_SIZE));
				addPnode(&constPcode, pnp);
			}
		}
		delAttr(ap);
		return;
	}

	if ( dimDepth(attr) > 0 )		// is an array
	{
		if ( dimDepth(attr) == 1 && init_data->type == NODE_STR )
		{
			char *str = init_data->str.str;
			int length = attr->dimVect[1];
			if ( length == 0 ) length = strlen(str)+1;

			attrib *ap  = cloneAttr(attr);
			decDim(ap);
			int size = sizer(ap, ATTR_SIZE);
			for (int i = 0; i < length; i++)
			{
				pnp = new Pnode(P_FILL);
				pnp->items[0] = intItem(str[i]);
				pnp->items[1] = intItem(size);
				addPnode(&constPcode, pnp);
			}
			delAttr(ap);
		}
		return;
	}

	if ( attr->ptrVect || attr->type != STRUCT )
	{
		if ( init_data->type == NODE_LIST )
		{
			romDataInit(dname, attr, LIST_NODE(init_data, 0), NULL, 0);
			return;
		}
	}
	else
	{
		if ( init_data->type == NODE_LIST )
		{
			int list_length = LIST_LENGTH(init_data);
			int struct_length = memberCount(attr);

			for (int i = 0; i < struct_length; i++)
			{
				attrib *ap = memberAttr(attr, i);
				if ( i < list_length )
					romDataInit(dname, ap, LIST_NODE(init_data, i), NULL, 0);
				else
				{
					pnp = new Pnode(P_FILL);
					pnp->items[0] = intItem(0);
					pnp->items[1] = intItem(sizer(ap, ATTR_SIZE));
					addPnode(&constPcode, pnp);
				}
			}
		}
		else
			errPrint("'struct' init error!", dname);
		return;
	}

	Pnode *backup = mainPcode;
	mainPcode = NULL;
	run(init_data);
	if ( DEPTH() == (depth+1) )
	{
		Item *ip = POP();
		int type = ip->type;
		if ( type == CON_ITEM || type == IMMD_ITEM || type == LBL_ITEM )
		{
			pnp = new Pnode(P_FILL);
			pnp->items[0] = ip;
			pnp->items[1] = intItem(size);
			addPnode(&constPcode, pnp);
			mainPcode = backup;
			return;
		}
		delete ip;
	}

	while ( DEPTH() > depth ) DEL();
	errPrint("rom init data error!", dname);
	delPnodes(&mainPcode);
	mainPcode = backup;
}

/////////////// --- init ram data --- ///////////////
void Pcoder :: ramDataInit(char *dname, attrib *attr, node *init_data, Dnode *dp, int addr)
{
	Pnode *pnp;

	if ( init_data->type == NODE_LIST )
	{
		if ( dimDepth(attr) > 0 || (attr->ptrVect == NULL && attr->type == STRUCT) )
		{
			const char *err = arrayDimVerify(attr, init_data, true);
			if ( err != NULL )
			{
				errPrint(err, dname);
				return;
			}
			char buf[1024];
			sprintf(buf, "%s$init$", dp->nameStr());

			// create data image in rom space
			pnp = new Pnode(P_SEGMENT);
			pnp->items[0] = idItem(buf, attr);
			pnp->items[0]->home = dp;
			addPnode(&constPcode, pnp);

			romDataInit(dname, attr, init_data, NULL, addr);

			Item *des = immdItem(dp->nameStr(), attr);
			Item *src = immdItem(buf, attr); src->attr->dataBank = CONST;

			pnp = new Pnode(P_COPY);
			pnp->items[0] = des;	pnp->items[0]->home = dp;
			pnp->items[1] = src;	pnp->items[1]->home = NULL;
			pnp->items[2] = intItem(dp->size());
			addPnode(&initPcode, pnp);
		}
		else
			ramDataInit(dname, attr, LIST_NODE(init_data, 0), dp, addr);

		return;
	}

	int depth = DEPTH();
	Pnode *backup = mainPcode;
	mainPcode = NULL;
	run(init_data);

	if ( DEPTH() == (depth+1) )
	{
		int type = TOP()->type;
		if ( type == CON_ITEM || type == IMMD_ITEM || type == LBL_ITEM )
		{
			pnp = new Pnode('=');
			pnp->items[0] = idItem(dp->nameStr(), attr);
			pnp->items[0]->home = dp;
			pnp->items[1] = POP();
			addPnode(&initPcode, pnp);
			mainPcode = backup;
			return;
		}
	}

	while ( DEPTH() > depth ) DEL();
	errPrint("ram init data error!", dname);
	delPnodes(&mainPcode);
	mainPcode = backup;
}

static const char *arrayDimVerify(attrib *attr, node *init_node, bool void_start)
{
	int list_length = LIST_LENGTH(init_node);
	if ( list_length <= 0 ) return "init data error!";

	if ( dimDepth(attr) > 0 ) 			// array specify
	{
		int *v = attr->dimVect;
		for (int i = 0; i < v[0]; i++) {
			if ( v[i+1] == 0 )			// undefined dimension?
			{
				if ( i == 0 && void_start )	// most outside array
					v[1] = list_length;
				else
					return "init data array error!";
			}
		}
	}
	return NULL;
}