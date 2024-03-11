#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "fcall.h"

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

bool FcallMgr :: sameGroup(int f1, int f2)
{
	for (int i = 0; i < funcCount; i++)
	{
		if ( ENODE(i, f1) && ENODE(i, f2) )	return true;
		if ( ENODE(i, f1) && i == f2 )		return true;
		if ( ENODE(i, f2) && i == f1 )		return true;
	}
	return false;
}

bool FcallMgr :: withinGroup(int *id_list, int length, int f_id)
{
	for (int i = 0; i < length; i++)
		if ( !sameGroup(id_list[i], f_id) )
			return false;

	return true;
}

bool FcallMgr :: testFuncCall(int *id_list, int length, int f_id)
{
	if ( length == 0 )
		return true;

	// called from other group (more than one primary function)?
	if ( withinPrimary(f_id) > 1 )
		return false;

	// within the same group?
	if ( !withinGroup(id_list, length, f_id) )
		return false;

	int  list[funcCount];
	memset(list, 0, sizeof(list));
	for (int i = 0; i < length; i++)
		list[id_list[i]] = 1;

	for (int i = 0; i < funcCount; i++)
	{
		// in the group, call each other?
		if ( list[i] != 0 && sameGroup(i, f_id) &&
			 (ENODE(i, f_id) || ENODE(f_id, i)) )
			return false;
	}
	return true;
}

int FcallMgr :: withinPrimary(int fid)
{
	int n = 0;
	for (int i = 0; i < funcCount; i++)
		if ( primaryFunc[i] && ENODE(i, fid) )
			n++;
	return n;
}

void FcallMgr :: printFunc(void)
{
	printf("funcList length: %d\n", funcCount);
	for (FuncAttr *fp = funcList; fp; fp = fp->next)
		printf("(%d)%s[%d] ... (%d)\n", fp->funcIndex, fp->funcName, fp->funcIndex, fp->dataSize);
}

void FcallMgr :: printMatrix(void)
{
	for (int i = 0; i < funcCount; i++)
	{
		printf("%02d:", i);
		for (int j = 0; j < funcCount; j++)
			printf(" %d", ENODE(i, j));
		printf("\n");
	}
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
		FuncAttr *mp = getFunc(i);
		queue[0] = i;

		if ( strcmp(mp->funcName, "main") == 0 || strstr(mp->funcName, "$interrupt") )
		{
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
