#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"

void *MALLOC(int size)
{
	char *s = malloc(size);
	if ( s == NULL ) exit(-1);	// out of memory space!
	memset(s, 0, size);
	return (void*)s;
}

char *dupStr(char *s)
{
	char *p = NULL;
	if ( s )
	{
		p = (char*)MALLOC(strlen(s)+1);
		strcpy(p, s);
	}
	return p;
}