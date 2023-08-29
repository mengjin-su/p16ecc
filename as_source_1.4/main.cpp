#include <stdio.h>
#include <string.h>
extern "C" {
#include "common.h"
}
#include "p16asm.h"

int useBsr6 = 0;
int	libFile = 0;

static void asm0(char *filename);

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if ( argv[i][0] == '-' )	// option
		{
			if ( strcmp(argv[i], "-X") == 0 )
				useBsr6 = 1;
			else if ( strcmp(argv[i], "-a") == 0 )
				libFile = 1;
			else
				printf("unknown option '%s'!\n", argv[i]);
			continue;
		}

		int length = strlen(argv[i]);
		if ( !(length > 4 && strcasecmp(&argv[i][length-4], ".asm") == 0) )
		{
			printf("improper file type/name '%s'!\n", argv[i]);
			continue;
		}

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

	P16E_asm p16asm(filename, libFile? ".lib": ".obj");
	if ( p16asm.errorCount != 0 ) return;

	p16asm.run(lp, ASM_PASS1);
	if ( p16asm.errorCount != 0 ) return;

	p16asm.run(lp, ASM_PASS2);
	if ( p16asm.errorCount > 0 ) return;

	p16asm.output(lp);
}