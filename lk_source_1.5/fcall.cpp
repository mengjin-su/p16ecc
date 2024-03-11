#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "fcall.h"

FcallMgr :: FcallMgr()
{
	memset(this, 0, sizeof(FcallMgr));
}

FcallMgr :: ~FcallMgr()
{
	while ( funcList )	// delete name list
	{
		free(funcList->fileName);
		free(funcList->funcName);
		FuncAttr *next = funcList->next;
		delete funcList;
		funcList = next;
	}
	
	if ( primaryFunc )
		delete [] primaryFunc;
}

/*
	function node created natively from .obj file:

		[U fname N]
*/
void FcallMgr :: addFunc(line_t *lp, Segment *seg)
{
	int cnt = itemCount(lp->items);

	if ( cnt > 0 && lp->items->type == TYPE_SYMBOL )
	{
		FuncAttr *p = new FuncAttr;
		memset(p, 0, sizeof(FuncAttr));

		p->fileName = dupStr(lp->fname);
		p->funcName = dupStr(lp->items->data.str);
		p->segment  = seg;
		p->funcIndex= funcCount++;

		// log function local data size...
		if ( cnt > 1 && lp->items->next->type == TYPE_VALUE )
			p->dataSize = lp->items->next->data.val;

		if ( funcList )
		{
			FuncAttr *tmp = funcList;
			while ( tmp->next ) tmp = tmp->next;
			tmp->next = p;
		}
		else
			funcList = p;
	}
}

void FcallMgr :: addFcall(line_t *lp)
{
	int cnt = itemCount(lp->items);

	// add the function call description to the list (if not exist)
	if ( cnt >= 2 &&
		 lp->items->type 	   == TYPE_SYMBOL &&	// caller's name
		 lp->items->next->type == TYPE_SYMBOL &&	// callee's name
		 searchFcall (lp->fname,
					  lp->items->data.str,
					  lp->items->next->data.str) == NULL )
	{
		Fcall *p = new Fcall;
		p->fileName = dupStr(lp->fname);
		p->caller   = dupStr(lp->items->data.str);
		p->callee   = dupStr(lp->items->next->data.str);
		p->next  	= NULL;

		if ( fcallList == NULL )	// append new node to list
			fcallList = p;
		else
		{
			Fcall *fp = fcallList;
			while ( fp->next ) fp = fp->next;
			fp->next = p;
		}
	}
}

void FcallMgr :: setGlobal(line_t *lp, char *func)
{
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
	{
		if ( strcmp (lp->fname, fp->fileName) == 0 &&
			 strcmp (func,      fp->funcName) == 0 )
		{
			fp->global = 1;
			return;
		}
	}
}

Fcall *FcallMgr :: searchFcall(char *fname, char *caller, char *callee)
{
	for (Fcall *fp = fcallList; fp; fp = fp->next)
	{
		if ( strcmp(fp->fileName, fname) == 0 &&
			 strcmp(fp->caller, caller)  == 0 &&
			 strcmp(fp->callee, callee)  == 0 )	return fp;
	}
	return NULL;
}
