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
			FNODE(fp1->funcIndex, fp2->funcIndex) = 1;
	}
}

///////////////////////////////////////////////////////
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
	
	for (Fcall *fcp = fcallList; fcp; fcp = fcp->next)
		printf("file=%s, %s -> %s\n", fcp->fileName, fcp->caller, fcp->callee);

	for (int i = 0; i < funcCount; i++)
	{
		FuncAttr *fp = getFunc(i);
		Segment  *sp = fp->segment;
		printf("file=%s, func=%s, segtype=%s\n", fp->fileName, fp->funcName, sp->name());
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

FuncAttr *FcallMgr :: getFunc(int func_id)
{
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
		if ( fp->funcIndex == func_id )	return fp;
	
	return NULL;
}

int FcallMgr :: funcDataSize(int func_id)
{
	FuncAttr *fp = getFunc(func_id);
	return fp? fp->dataSize: 0;
}
