#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "segment.h"
#include "fcall.h"

FcallMgr :: FcallMgr()
{
	memset(this, 0, sizeof(FcallMgr));
}

FcallMgr :: ~FcallMgr()
{
	for (int i = 0; i < funcCount; i++)
	{
		delete [] basicVectorMatrix [i];
		delete [] extendVectorMatrix[i];
	}
	delete [] basicVectorMatrix;
	delete [] extendVectorMatrix;

	while ( funcList )	// delete name list
	{
		free(funcList->fileName);
		free(funcList->funcName);
		FuncAttr *next = funcList->next;
		delete funcList->rootList;
		delete funcList;
		funcList = next;
	}
}

/*
	function node created natively from .obj file: [U fname N]
*/
void FcallMgr :: addFcallName(line_t *lp, Segment *seg)
{
	if ( itemCount(lp->items) > 0 && lp->items->type == TYPE_SYMBOL )
	{
		FuncAttr *p = new FuncAttr;
		memset(p, 0, sizeof(FuncAttr));

		p->fileName = dupStr(lp->fname);
		p->funcName = dupStr(lp->items->data.str);
		p->segment  = seg;
		p->funcIndex= funcCount++;		// function log index
		p->dataSize = seg->dataSize;	// function local data size

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

void FcallMgr :: addFcallLink(line_t *lp)
{
	// add the function call description to the list (if not exist)
	if ( itemCount(lp->items) >= 2 &&
		 lp->items->type 	   == TYPE_SYMBOL &&	// caller's name
		 lp->items->next->type == TYPE_SYMBOL &&	// callee's name
		 searchFcall(lp->fname,
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
			fp->global = true;
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

///////////////////////////////////////////////////////
void FcallMgr :: createBasicMatrix(void)
{
	basicVectorMatrix = new char *[funcCount];
	for (int i = 0; i < funcCount; i++)
	{
		basicVectorMatrix[i] = new char[funcCount];
		memset(basicVectorMatrix[i], 0, funcCount);
	}

	// build up the basic calling chain ...
	for (Fcall *fp = fcallList; fp; fp = fp->next)
	{
		FuncAttr *fp1 = getFunc(fp->fileName, fp->caller, IS_CALLER);
		FuncAttr *fp2 = getFunc(fp->fileName, fp->callee, IS_CALLEE);

		if ( fp1 && fp2 && fp1->funcIndex != fp2->funcIndex )
			BNODE(fp1->funcIndex, fp2->funcIndex) = 1;
	}
}

void FcallMgr :: createExtendMatrix(void)
{
	extendVectorMatrix = new char *[funcCount];
	for (int i = 0; i < funcCount; i++)
	{
		extendVectorMatrix[i] = new char[funcCount];
		memcpy(extendVectorMatrix[i], basicVectorMatrix[i], funcCount);
	}

	// extending the calling relationship...
	// 	 if N(i,j) and N(j,k), then N(i,k) = 2 which means it's extended link.
	for (bool done = false; !done;)
	{
		done = true;
		for (int i = 0; i < funcCount; i++)
		for (int j = 0; j < funcCount; j++)
		{
			if ( ENODE(i, j) )
				for (int k = 0; k < funcCount; k++)
					if ( ENODE(j, k) && !ENODE(i, k) && i != k )
					{
						ENODE(i, k) = 2;
						done = false;
					}
		}
	}

	for (int i = 0; i < funcCount; i++)
	{
		FuncAttr *fp = getFunc(i);
		fp->funcType = funcType(i);
		fp->rootList = new char[funcCount];
		memset(fp->rootList, 0, funcCount);

		char trace_list[funcCount];
		memset(trace_list, 0, funcCount);

		findRoot(i, trace_list, fp->rootList);
		validation(i);
	}
}

FuncAttr *FcallMgr :: getFunc(char *file, char *func, FUNC_MODE mode)
{
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
	{
		if ( strcmp(func, fp->funcName) )
			continue;

		if ( strcmp(file, fp->fileName) == 0 ||	// within same file
			 (mode == IS_CALLEE && fp->global) )
			return fp;
	}

	return NULL;
}

FuncAttr *FcallMgr :: getFunc(int func_idx)
{
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
		if ( fp->funcIndex == func_idx ) return fp;

	return NULL;
}
/*
void FcallMgr :: createShareGroups(void)
{
	if ( funcCount > 0 )
	{	// generate & extending the calling matrix ...
		createBasicMatrix();
		createExtendMatrix();

		// generate primary function list;
		primaryFunc = new char[funcCount];
		memset(primaryFunc, 0, funcCount);
		for (int i = 0; i < funcCount; i++)
		{
			bool isPrime = true;
			FuncAttr *fp = getFunc(i);

			for (int j = 0; j < funcCount; j++)
				if ( ENODE(j, i) ) isPrime = false;

			if ( isPrime || strcmp(fp->funcName, "main") == 0 )
				primaryFunc[i] = 1;

			if ( fp->segment->isName("CODE1") )
				primaryFunc[i] = 1;
		}
	}
}
*/