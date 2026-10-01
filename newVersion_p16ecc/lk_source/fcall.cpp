#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "fcall.h"

FcallMgr fcallMgr;

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

void FcallMgr :: printFunc(void)
{
	printf("funcList length: %d\n", funcCount);
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
		printf("(%d) %s [R=%d] ... (%d)\n", fp->funcIndex, fp->funcName, fp->root, fp->dataSize);
}

///////////////////////////////////////////////////////////////
void FcallMgr :: outputCallPath(FILE *fout)
{
	int main_max = 0;
	int interrupt_max = 0;

	if ( fout == NULL ) fout = stdout;
	fprintf (fout, "\n************ CALLING PATH ************\n");

	int *queue = new int[funcCount];
	for (int i = 0; i < funcCount; i++)
	{
		FuncAttr *fp = getFunc(i);
		if ( fp == NULL ) continue;

		if ( fp->funcType == MAIN_FUNC || fp->funcType == ISR_FUNC || fp->funcType == NULL_FUNC )
		{
			queue[0] = i;
			outputCallPath(fout, queue, 1);
		}
	}

	delete queue;
	fprintf (fout, "\n");
}

bool withinList(int *list, int length, int id)
{
	for (int i = 0; i < length; i++)
		if ( list[i] == id ) return true;

	return false;
}

void FcallMgr :: outputCallPath(FILE *fout, int *queue, int length)
{
	bool terminated = true;
	for (int i = 0; i < funcCount; i++)
	{
		int I = queue[length-1];
		if ( FNODE(I, i) && I != i && !withinList(queue, length, i) )
		{
			terminated = false;
			queue[length] = i;
			outputCallPath(fout, queue, length+1);
		}
	}

	if ( terminated && length > 1 )
	{
		for (int i = 0; i < length; i++)
		{
			fprintf(fout, "[%d]", i);
			for (int j = i; j--;) fprintf(fout, " ");
			FuncAttr *fp = getFunc(queue[i]);
			fprintf(fout, "%s\n", fp->funcName);
		}
		fprintf(fout, "\n");
	}
}
