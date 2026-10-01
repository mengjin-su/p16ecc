#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "fcall.h"

#define SAME_CHAIN(f1,f2)	(ENODE(f1,f2) || ENODE(f2,f1))

void FcallMgr :: createBasicMatrix(void)
{
	basicFcallMatrix = new char *[funcCount];
	for (int i = 0; i < funcCount; i++)
	{
		basicFcallMatrix[i] = new char[funcCount];
		memset(basicFcallMatrix[i], 0, funcCount);
	}

	// build up the basic calling chain ...
	for (Fcall *fp = fcallList; fp; fp = fp->next)
	{
		FuncAttr *fp1 = getFunc(fp->fileName, fp->caller, IS_CALLER);
		FuncAttr *fp2 = getFunc(fp->fileName, fp->callee, IS_CALLEE);

		if ( fp1 && fp2 && fp1->funcIndex != fp2->funcIndex )
			FNODE(fp1->funcIndex, fp2->funcIndex) = 1;
	}
}

void FcallMgr :: createExtendMatrix(void)
{
	extendFcallMatrix = new char *[funcCount];
	for (int i = 0; i < funcCount; i++)
	{
		extendFcallMatrix[i] = new char[funcCount];
		memcpy(extendFcallMatrix[i], basicFcallMatrix[i], funcCount);
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

	for (int i = 0; i < funcCount; i++)	// assign function type
	{
		FuncAttr *fp = getFunc(i);
		fp->funcType = funcType(i);
	}

	for (int i = 0; i < funcCount; i++)	// assign function root
		findRoot(i);
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

FuncAttr *FcallMgr :: getFunc(int func_id)
{
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
		if ( fp->funcIndex == func_id )	return fp;

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
int FcallMgr :: funcType(int f_index)
{
	FuncAttr *fp = getFunc(f_index);
	if ( fp )
	{
		if ( strcmp(fp->funcName, "main") == 0 )
			return MAIN_FUNC;

		if ( strcmp(fp->segment->name(), "CODE1") == 0 )
			return (fp->segment->isBEG || fp->segment->isEND)? NULL_FUNC:
															   ISR_FUNC;

		for (int i = 0; i < funcCount; i++)
		{
			if ( i != f_index && ENODE(i, f_index) )
				return SUB_FUNC;
		}
	}
	return NULL_FUNC;
}

bool FcallMgr :: joinInGroup(int *group, int group_length, int f_index0)
{
	FuncAttr *fp0 = getFunc(f_index0);
	int     type0 = fp0->funcType;

	for (int i = 0; i < group_length; i++)
	{
		int  f_index1 = group[i];
		FuncAttr *fp1 = getFunc(f_index1);
		int     type1 = fp1->funcType;

		if ( f_index1 == f_index0 ) continue;

		if ( fp0->multiRoots ) return false;
		if ( fp0->root != fp1->root ) return false;
		if ( SAME_CHAIN(f_index0, f_index1) ) return false;
		if ( type0 != type1 ) return false;
	}
	return true;
}

void FcallMgr :: findRoot(int f_index)
{
	FuncAttr *fp = getFunc(f_index);

	fp->multiRoots = false;
	fp->root = (funcType(f_index) == NULL_FUNC)? ((f_index+1) << 8): 0;

	for (int i = 0; i < funcCount; i++)
	{
		if ( (i != f_index) && ENODE(i, f_index) )
			switch ( funcType(i) )
			{
				case MAIN_FUNC:
					if ( fp->root && fp->root != MAIN_FUNC ) fp->multiRoots = true;
					fp->root = MAIN_FUNC;
					break;
				case ISR_FUNC:
					if ( fp->root && fp->root != ISR_FUNC ) fp->multiRoots = true;
					fp->root = ISR_FUNC;
					break;
				case NULL_FUNC:
					if ( fp->root && fp->root != ((i+1) << 8) )	fp->multiRoots = true;
					fp->root = (i+1) << 8;
					break;
			}
	}
}
