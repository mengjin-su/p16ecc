#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "fcall.h"

void FcallMgr :: outputCallPath(FILE *fout)
{
	if ( fout == NULL ) fout = stdout;
	fprintf (fout, "\n************ CALLING PATH ************\n");

	int *queue = new int[funcCount];
	for (int i = 0; i < funcCount; i++)
	{
		FuncAttr *fp = getFunc(i);
		queue[0] = i;

		if ( fp->funcType == MAIN_FUNC || fp->funcType == ISR_FUNC || fp->funcType == NULL_FUNC )
			outputCallPath(fout, queue, 1);
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
		if ( BNODE(I, i) && I != i && !withinList(queue, length, i) )
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
