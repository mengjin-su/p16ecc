#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include "path.h"

#define CC_FOLDER_NAME	"p16ecc"	// compiler tool dir.
#define CC_FOLDER_LEN	strlen(CC_FOLDER_NAME)

#define IS_SLASH(c)		((c) == '/' || (inWindows && (c) == '\\'))
#define IS_SEPERATOR(c)	((c) == '=' || (inWindows && (c) == ';') || (!inWindows && (c) == ':') )


Path :: Path(char *env[])
{
	int inWindows = 0;
	ok = false;
	for (int i = 0; env[i]; i++)
	{
		char *p = env[i];

		if ( strncasecmp(p, "OS=WINDOWS", 10) == 0 )
			inWindows = 1;
		else if ( strncasecmp(p, "path=", 5) == 0 )
		{
			p = strstr(p, CC_FOLDER_NAME);

			if ( p && IS_SLASH(p[-1]) && IS_SLASH(p[CC_FOLDER_LEN]) &&
				 memcmp(p+CC_FOLDER_LEN+1, "bin", 3) == 0 )
			{
				int l = CC_FOLDER_LEN;
				while ( !IS_SEPERATOR(p[-1]) ) { p--; l++; }

				pathBuf = p;
				pathBuf = pathBuf.substr(0, l);	pathBuf += "/";
				ok = true;
//				printf("<path> = %s\n", pathBuf.c_str());
			}
		}
	}
}
