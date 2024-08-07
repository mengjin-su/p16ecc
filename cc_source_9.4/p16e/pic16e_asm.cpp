#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pic16e_inst.h"
#include "pic16e_asm.h"

P16E_ASM :: P16E_ASM(FILE *fileout, bool opt)
{
	fout = fileout;
	asmOpt = new P16E_asmOPT(fileout);
	optimizeEnabled = opt;
}

P16E_ASM :: ~P16E_ASM()
{
	delete asmOpt;
}

void P16E_ASM :: output(char *s)
{
	if ( s )
	{
		if ( optimizeEnabled )
			asmOpt->output(s);
		else
			fprintf(fout, "%s", s);
	}
}

void P16E_ASM :: label(char *lbl, bool c, char *opr)
{
	if ( optimizeEnabled )
		asmOpt->label(lbl, c, opr);
	else
	{
		fprintf(fout, "%s:", lbl);
		if ( c ) fprintf(fout, ":");
		if ( opr ) fprintf(fout, "\t%s", opr);
		fprintf(fout, "\n");
	}
}

void P16E_ASM :: code(char *inst)
{
	if ( optimizeEnabled )
		asmOpt->code(inst);
	else
		fprintf(fout, "\t%s\n", inst);
}

void P16E_ASM :: code(char *inst, char *opr1, char *opr2)
{
	if ( optimizeEnabled )
		asmOpt->code(inst, opr1, opr2);
	else
	{
		fprintf(fout, "\t%s\t%s", inst, opr1);
		if ( opr2 ) fprintf(fout, ", %s", opr2);
		fprintf(fout, "\n");
	}
}

void P16E_ASM :: code(char *inst, int opr1, char *opr2)
{
	if ( optimizeEnabled )
		asmOpt->code(inst, opr1, opr2);
	else
	{
		fprintf(fout, "\t%s\t%d", inst, opr1);
		if ( opr2 ) fprintf(fout, ", %s", opr2);
		fprintf(fout, "\n");
	}
}

void P16E_ASM :: code(char *inst, char *opr1, int opr2)
{
	if ( optimizeEnabled )
		asmOpt->code(inst, opr1, opr2);
	else
		fprintf(fout, "\t%s\t%s, %d\n", inst, opr1, opr2);
}

void P16E_ASM :: code(char *inst, int opr1, int opr2)
{
	if ( optimizeEnabled )
		asmOpt->code(inst, opr1, opr2);
	else
		fprintf(fout, "\t%s\t%d, %d\n", inst, opr1, opr2);
}
