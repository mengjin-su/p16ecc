#include <stdio.h>
#include <string.h>
extern "C" {
#include "common.h"
}
#include "p16asm.h"

static void asm0(char *filename);

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		asm0(argv[i]);
	}
	return 0;
}

static void asm0(char *filename)
{
	curFile = filename;
	printf ("assembling '%s' ...\n", filename);
	
	// using Lex and Bison to parse the input file ...
	errorCnt  = 0;
	line_t *lp = _main(filename);
	if ( errorCnt != 0 ) return;
	
	P16E_asm p16asm(filename, ".obj");
	if ( p16asm.errorCount != 0 ) return;

	p16asm.run(lp, ASM_PASS1);
	if ( p16asm.errorCount != 0 ) return;

	p16asm.run(lp, ASM_PASS2);
}