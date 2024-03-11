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

#define CROSS_CALL(f1,f2)	(ENODE(f1,f2) || ENODE(f2,f1))


int FcallMgr :: funcType(int f_index)
{
	FuncAttr *fp = getFunc(f_index);
	if ( fp )
	{
		if ( strcmp(fp->funcName, "main") == 0 )
			return MAIN_FUNC;

		if ( strcmp(fp->segment->name(), "CODE1") == 0 )
			return (fp->segment->isBEG ||
					fp->segment->isEND) ? NULL_FUNC: ISR_FUNC;

		for (int i = 0; i < funcCount; i++)
		{
			if ( i != f_index && ENODE(i, f_index) )
				return SUB_FUNC;
		}
	}
	return NULL_FUNC;
}

int FcallMgr :: funcDataSize(int f_index)
{
	FuncAttr *fp = getFunc(f_index);
	return fp? fp->segment->dataSize: 0;
}

void FcallMgr :: findRoot(int f_index, char *trace_list, char *root_list)
{
	if ( !trace_list[f_index] )
	{
		trace_list[f_index] = 1;
		for (int i = 0; i < funcCount; i++)
		{
			if ( !trace_list[i] && BNODE(i, f_index) )
			{
				switch ( funcType(i) )
				{
					case MAIN_FUNC:
					case ISR_FUNC:
					case NULL_FUNC:
						root_list[i] = 1;
						break;

					default:
						findRoot(i, trace_list, root_list);
						break;
				}
			}
		}
	}
}

bool FcallMgr :: joinInGroup(int *group, int length, int f_index)
{
	for (int i = 0; i < length; i++)
	{
		FuncAttr *fp1 = getFunc(group[i]);
		FuncAttr *fp2 = getFunc(f_index);

		if ( fp2->standalone 				||
			 CROSS_CALL(group[i], f_index)  ||
			 fp1->funcType != fp2->funcType ||
			 memcmp(fp1->rootList, fp2->rootList, funcCount) ) return false;
	}
	return true;
}

void FcallMgr :: validation(int f_index)
{
	if ( funcType(f_index) == SUB_FUNC )
	{
		bool isr_root = false;
		bool main_root= false;
		bool null_root= false;
		FuncAttr *fptr = getFunc(f_index);
		for (int i = 0; i < funcCount; i++)
		{
			if ( fptr->rootList[i] )
				switch ( funcType(i) )
				{
					case MAIN_FUNC:	main_root = true;	break;
					case ISR_FUNC:	isr_root  = true;	break;
					case NULL_FUNC:	null_root = true;	break;
				}
		}

		// multiple roots, can't share memory with other functions
		fptr->standalone = (main_root && isr_root ) ||
				 		   (main_root && null_root) ||
				 	 	   (isr_root  && null_root);

		for (int i = 0; i < funcCount && !fptr->standalone; i++)
		{
			if ( funcType(i) == ISR_FUNC && isr_root )
				fptr->rootList[i] = 1;
		}
	}
}