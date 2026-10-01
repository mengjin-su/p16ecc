#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}
#include "parse.h"

#define VERSION     "v0.1.0f"

void asm0(char *filename);

////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[], char *env[])
{
    printf("ST8AS Assembler, %s\n", VERSION);

    for (int i = 1; i < argc; i++)
    {
		char *p = argv[i];
		int   l = strlen(p);
		if ( l > 4 && strcmp(&p[l-4], ".asm") == 0 )
			asm0(p);   // parse source file
	}

    return error_cnt;
}

////////////////////////////////////////////////////////////////////////
/*
	parse the input source file, and generate the raw file in
	'line_t' format.
*/
////////////////////////////////////////////////////////////////////////
void asm0(char *filename)
{
    cur_file = filename;
    printf("assembling '%s' ...\n", filename);

	// using Lex and Bison to parse the source ...
	error_cnt  = 0;
    line_t *lp = _main(filename);

	if ( error_cnt > 0 )
		exit(1);

	parse(lp);
}
