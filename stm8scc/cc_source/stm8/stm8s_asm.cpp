#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm8s_inst.h"
#include "stm8s_reg.h"
#include "stm8s_asm.h"

STM8S_ASM :: STM8S_ASM(FILE *fileout)
{
	fout = fileout;
	outputBuffered = false;
	memset(instBuf,    0, sizeof(instBuf));
	memset(oprand1Buf, 0, sizeof(oprand1Buf));
	memset(oprand2Buf, 0, sizeof(oprand2Buf));
}

void STM8S_ASM :: output(char *s)
{
	if ( s ) fprintf(fout, "%s", s);
	outputBuffered = false;
}

void STM8S_ASM :: output(int n)
{
	if ( fout ) fprintf(fout, "%d", n);
	outputBuffered = false;
}

void STM8S_ASM :: output(const char *s)
{
	output((char*)s);
	outputBuffered = false;
}

void STM8S_ASM :: output_ln(char *s)
{
	if ( s && fout ) fprintf(fout, "%s", s);

	if ( fout ) fprintf(fout, "\n");
	outputBuffered = false;
}

void STM8S_ASM :: output(char c)
{
	if ( fout ) fprintf(fout, "%c", c);
	outputBuffered = false;
}

void STM8S_ASM :: label(int lbl)
{
	fprintf(fout, "_$L%d:\n", lbl);
	outputBuffered = false;
}

void STM8S_ASM :: label(char *lbl, bool c)
{
	if ( fout ) {
		fprintf(fout, "%s", lbl);
		if ( c ) fprintf(fout, ":");
		outputBuffered = false;
	}
}

void STM8S_ASM :: label_ln(char *lbl, bool c)
{
	if ( fout ) {
		if ( c ) fprintf(fout, "%s:\n", lbl);
		else 	 fprintf(fout, "%s\n", lbl);
		outputBuffered = false;
	}
}

void STM8S_ASM :: icode(char *inst)
{
	fprintf(fout, "\t%s", inst);
	int len = strlen(inst);
	if ( len > 7 ) len = 7;
	while ( len++ < 8 ) fprintf(fout, " ");
}

void STM8S_ASM :: code0(char *inst)
{
	icode(inst);
	fprintf(fout, "\n");
	outputBuffered = false;
}

void STM8S_ASM :: code1(char *inst, const char *opr)
{
	icode(inst);
	fprintf(fout, "%s\n", opr);
	outputBuffered = false;
}

void STM8S_ASM :: code1(char *inst, char *opr, char immd)
{
	icode(inst);
	if ( immd ) fprintf(fout, "#");
	fprintf(fout, "%s\n", opr);
	outputBuffered = false;
}

void STM8S_ASM :: code1(char *inst, int opr, char immd)
{
	icode(inst);
	if ( immd ) fprintf(fout, "#");
	fprintf(fout, "%d\n", opr);
	outputBuffered = false;
}

void STM8S_ASM :: code2(char *inst, char *opr1, char *opr2, char immd)
{
	if ( !redundant(inst, opr1, opr2, immd) )
	{
		icode(inst);
		fprintf(fout, "%s, ", opr1);
		if ( immd ) fprintf(fout, "#");
		fprintf(fout, "%s\n", opr2);
	}
}

void STM8S_ASM :: code2(char *inst, char *opr1, int opr2, char immd)
{
	if ( !redundant(inst, opr1, opr2, immd) )
	{
		icode(inst);
		fprintf(fout, "%s, ", opr1);
		if ( immd ) fprintf(fout, "#");
		fprintf(fout, "%d\n", opr2);
	}
}

void STM8S_ASM :: code2(char *inst, int opr1, char *opr2, char immd)
{
	if ( !redundant(inst, opr1, opr2, immd) )
	{
		icode(inst);
		fprintf(fout, "%d, ", opr1);
		if ( immd ) fprintf(fout, "#");
		fprintf(fout, "%s\n", opr2);
	}
}

void STM8S_ASM :: code2(char *inst, int opr1, int opr2, char immd)
{
	if ( !redundant(inst, opr1, opr2, immd) )
	{
		icode(inst);
		fprintf(fout, "%d, ", opr1);
		if ( immd ) fprintf(fout, "#");
		fprintf(fout, "%d\n", opr2);
	}
}
#if 0
void STM8S_ASM :: code2(char *inst, STM8S_reg *reg, int val, char immd)
{
	if ( !redundant(inst, (char*)reg->regName, val, immd) )
	{
		icode(inst);
		fprintf(fout, "%s, ", reg->regName);
		if ( immd ) fprintf(fout, "#%d\n", val & ((reg->regSize==8)? 0xff: 0xffff));
		else 	    fprintf(fout, "%d\n", val);
	}
}

void STM8S_ASM :: code2(char *inst, STM8S_reg *reg, char *str, char immd)
{
	if ( !redundant(inst, (char*)reg->regName, str, immd) )
	{
		icode(inst);
		fprintf(fout, "%s, ", reg->regName);
		if ( immd ) fprintf(fout, "#");
		fprintf(fout, "%s\n", str);
	}
}

void STM8S_ASM :: code2(char *inst, STM8S_reg *dst, STM8S_reg *src)
{
	if ( !redundant(inst, (char*)dst->regName, (char*)src->regName, false) )
		code2(inst, (char*)dst->regName, (char*)src->regName, 0);
}

void STM8S_ASM :: code2(char *inst, char *dst, STM8S_reg *src)
{
	if ( !redundant(inst, dst, (char*)src->regName, false) )
		code2(inst, dst, (char*)src->regName, 0);
}
#endif
void STM8S_ASM :: codeLbl (char *inst, int lbl)
{
	char buf[32];
	sprintf(buf, "_$L%d", lbl);
	code1(inst, buf, 0);
	outputBuffered = false;
}

bool STM8S_ASM :: redundant(char *inst, char *opr1, char *opr2, bool immd)
{
	if ( immd )	return outputBuffered = false;

	if ( outputBuffered && inst && opr1 && opr2 &&
		 (strcmp(inst, _LD) == 0 || strcmp(inst, _LDW) == 0) )
	{
		if ( strcmp(instBuf, inst) == 0 &&
			((strcmp(oprand1Buf, opr1) == 0 && strcmp(oprand2Buf, opr2) == 0) ||
			 (strcmp(oprand1Buf, opr2) == 0 && strcmp(oprand2Buf, opr1) == 0)) )
			return true;
	}
	strcpy(instBuf,    inst? inst: "");
	strcpy(oprand1Buf, opr1? opr1: "");
	strcpy(oprand2Buf, opr2? opr2: "");
	outputBuffered = true;
	return false;
}

bool STM8S_ASM :: redundant(char *inst, char *opr1, int opr2, bool immd)
{
	char buff[32];

	if ( immd )	return outputBuffered = false;

	sprintf(buff, "%d", opr2);
	if ( outputBuffered && inst && opr1 &&
		 (strcmp(inst, _LD) == 0 || strcmp(inst, _LDW) == 0) )
	{
		if ( strcmp(instBuf, inst) == 0 &&
			((strcmp(oprand1Buf, opr1) == 0 && strcmp(oprand2Buf, buff) == 0) ||
			 (strcmp(oprand1Buf, buff) == 0 && strcmp(oprand2Buf, opr1) == 0)) )
			return true;
	}
	strcpy(instBuf,    inst? inst: "");
	strcpy(oprand1Buf, opr1? opr1: "");
	strcpy(oprand2Buf, buff);
	outputBuffered = true;
	return false;
}

bool STM8S_ASM :: redundant(char *inst, int opr1, char *opr2, bool immd)
{
	char buff[32];

	if ( immd )	return outputBuffered = false;

	sprintf(buff, "%d", opr1);
	if ( outputBuffered && inst && opr2 &&
		 (strcmp(inst, _LD) == 0 || strcmp(inst, _LDW) == 0) )
	{
		if ( strcmp(instBuf, inst) == 0 &&
			((strcmp(oprand1Buf, buff) == 0 && strcmp(oprand2Buf, opr2) == 0) ||
			 (strcmp(oprand1Buf, opr2) == 0 && strcmp(oprand2Buf, buff) == 0)) )
			return true;
	}
	strcpy(instBuf,    inst? inst: "");
	strcpy(oprand2Buf, opr2? opr2: "");
	strcpy(oprand1Buf, buff);
	outputBuffered = true;
	return false;
}

bool STM8S_ASM :: redundant(char *inst, int opr1, int opr2, bool immd)
{
	char buf1[32], buf2[32];
	if ( immd )	return outputBuffered = false;

	sprintf(buf1, "%d", opr1);
	sprintf(buf2, "%d", opr2);
	if ( outputBuffered && inst &&
		 (strcmp(inst, _LD) == 0 || strcmp(inst, _LDW) == 0) )
	{
		if ( strcmp(instBuf, inst) == 0 &&
			((strcmp(oprand1Buf, buf1) == 0 && strcmp(oprand2Buf, buf2) == 0) ||
			 (strcmp(oprand1Buf, buf2) == 0 && strcmp(oprand2Buf, buf1) == 0)) )
			return true;
	}
	strcpy(instBuf,    inst? inst: "");
	strcpy(oprand1Buf, buf1);
	strcpy(oprand2Buf, buf2);
	outputBuffered = true;
	return false;
}