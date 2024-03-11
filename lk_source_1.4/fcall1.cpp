#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "fcall.h"

void FcallMgr :: createBasicMatrix(void)
{
	basicVectorMatrix  = new char *[funcCount];
	extendVectorMatrix = new char *[funcCount];

	// create the calling matrix ...
	for (int i = 0; i < funcCount; i++)
	{
		basicVectorMatrix[i] = new char[funcCount];
		memset(basicVectorMatrix[i], 0, funcCount);

		extendVectorMatrix[i] = new char[funcCount];
	}

	// build up the basic calling chain ...
	for (Fcall *fp = fcallList; fp; fp = fp->next)
	{
		FuncAttr *fp1 = searchFunc(fp->fileName, fp->caller, IS_CALLER);
		FuncAttr *fp2 = searchFunc(fp->fileName, fp->callee, IS_CALLEE);

		if ( fp1 && fp2 && fp1->funcIndex != fp2->funcIndex )
			FNODE(fp1->funcIndex, fp2->funcIndex) = 1;
	}
}

///////////////////////////////////////////////////////
void FcallMgr :: createExtendMatrix(void)
{
	for (int i = 0; i < funcCount; i++)
		memcpy(extendVectorMatrix[i], basicVectorMatrix[i], funcCount);

	// extending the calling relationship...
	// 	 if N(i,j) and N(j,k), then N(i,k) = 2.
	bool done = false;
	while ( !done )
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
}

FuncAttr *FcallMgr :: searchFunc(char *file, char *func, FUNC_MODE mode)
{
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
	{
		if ( strcmp(func, fp->funcName) == 0 &&
			 strcmp(file, fp->fileName) == 0 )
			return fp;
	}

	for (FuncAttr *fp = funcList; fp; fp = fp->next)
	{
		if ( mode == IS_CALLEE &&
			 strcmp(func, fp->funcName) == 0 && fp->global )
			return fp;
	}

	return NULL;
}

FuncAttr *FcallMgr :: searchFunc(int func_id)
{
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
		if ( fp->funcIndex == func_id )
			return fp;
	return NULL;
}

int FcallMgr :: funcDataSize(int func_id)
{
	FuncAttr *fp = searchFunc(func_id);
	return fp? fp->dataSize: 0;
}
