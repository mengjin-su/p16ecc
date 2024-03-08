#include <stdio.h>
#include <string.h>
extern "C" {
#include "common.h"
}

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		curFile = argv[i];
		printf ("assembling '%s' ...\n", curFile);
		
		// using Lex and Bison to parse the input file ...
		errorCnt  = 0;
		line_t *lp = _main(curFile);

		if ( !errorCnt )
		{
			//...
		}
	}
	return 0;
}
