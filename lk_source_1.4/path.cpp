#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "path.h"

#define TOOL_NAME		"p16cc"	// compiler tool dir.
#define IS_SLASH(c) 	((c) == '/' || (c) == '\\')
#define IS_START(c)		((c) == '=' || (c) == ';' )

char *skipSP(char *s);

Path :: Path(char *env[])
{
	pathBuf = NULL;
	for (int i = 0; env[i]; i++)
	{
		if ( memcmp(env[i], "Path=", 5) && memcmp(env[i], "PATH=", 5) )
			continue;

		int len = strlen(TOOL_NAME);
		char *p = strstr(env[i]+5, TOOL_NAME);

		if ( p && IS_SLASH(*(p-1)) && IS_SLASH(*(p+len)) &&
				  memcmp(p+len+1, "bin", 3) == 0 		  )
		{
			while ( !IS_START(*p) ) p--, len++;
			pathBuf = new char[len+1];
			memcpy(pathBuf, p+1, len);
			pathBuf[len] = '\0';
			break;
		}
	}
}

Path :: ~Path()
{
	if ( pathBuf != NULL )
	{
		delete [] pathBuf;
		pathBuf = NULL;
	}
}

char *skipSP(char *s)
{
	while ( *s == ' ' || *s == '\t' ) s++;
	return s;
}
