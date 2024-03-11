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
#include "dlink.h"
#include "flink.h"
#include "sizer.h"
#include "pcoder.h"

Dlink *dataLink = NULL;	// data link
Flink *funcLink = NULL;	// func link;

#define SET_COMMENT(p)	if(((node*)(p))->id.src) src = ((node*)(p))->id.src
#define CLR_COMMENT(p)	if(((node*)(p))->id.src == src) src = NULL


Pcoder :: Pcoder()
{
	memset(this, 0, sizeof(Pcoder));

	funcLink = new Flink();
	dataLink = new Dlink(VAR_DLINK);	// global variable list
	
	curDlink = dataLink;				// current variable list
	curFnode = NULL;
	
	constGroup = new Const();
}

Pcoder :: ~Pcoder() {
	CLEAR();
	delete constGroup;
	delPnodes(&mainPcode);
	delPnodes(&initPcode);
	delPnodes(&constPcode);
}

void Pcoder :: errPrint (const char *msg, char *opt_msg)
{
	printf ("[ERROR] ");
	if ( src ) printf ("%s, #%d ... ", src->fileName, src->lineNum );
	printf ("%s", msg);	if ( opt_msg ) printf (" - %s", opt_msg);
	printf ("\n");
	errorCount++;
}

void Pcoder :: warningPrint (const char *msg, char *opt_msg)
{
	printf ("[WARNING] ");
	if ( src ) printf ("%s, #%d ... ", src->fileName, src->lineNum );
	printf ("%s", msg);	if ( opt_msg ) printf (" - %s", opt_msg);
	printf ("\n");
	warningCount++;
}

void Pcoder :: takeSrc(node *np)
{
	char buf[4096];

	SET_COMMENT(np);
	if ( np->id.src )
	{
		sprintf(buf, "%s #%d: %s", src->fileName, src->lineNum, src->srcCode);
		Pnode *pnp = new Pnode(P_SRC_CODE);
		pnp->items[0] = strItem(buf);
		addPnode(&mainPcode, pnp);
	}
}

void Pcoder :: run(node *np)
{
	if ( np == NULL ) return;
	Dnode *dp;
	Fnode *fp;
	Item  *iptr;

	takeSrc(np);
	switch ( np->type )
	{
		case NODE_CON:	// const Node
			PUSH(intItem(np->con.value, cloneAttr(np->con.attr)));
			break;

		case NODE_STR:	// string Node
			iptr = idItem(constGroup->add(np->str.str));
			iptr->attr = newAttr(CHAR);
			iptr->type = IMMD_ITEM;
			iptr->attr->dataBank = CONST;
			PUSH(iptr);
			break;

		case NODE_ID:	// ID node
			dp = curDlink->search(np->id.name, WHOLE_SEARCH);
			fp = funcLink->search(np->id.name);
			if ( dp )		// found in data link
			{
				Item *ip = (dp->attr->dimVect)? immdItem(dp->nameStr(), dp->attr):
											    idItem  (dp->nameStr(), dp->attr);
				ip->home = (void*)dp;
				PUSH(ip);
			}
			else if ( fp )	// found in function link
			{
				Item *ip = lblItem(np->id.name);
				ip->home = (void*)fp;
				PUSH(ip);
			}
			else
				errPrint("name not found!", np->id.name);
			break;

		case NODE_LIST:	// list Node
			for (int i = 0; i < np->list.nops; i++)
				run(np->list.ptr[i]);
			break;

		case NODE_OPR:	// operator Node
//			run1((oprNode_t *)np);
			break;
	}
}
