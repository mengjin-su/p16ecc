#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "asm.h"
}
#include "symbol.h"
#include "fwriter.h"

static bool bankAddressing(int code);

/////////////////////////////////////////////////////////////////
ObjWriter :: ObjWriter(FILE *_fout)
{
	fout = _fout;
	columnNum = 0;
	lineType  = -1;
}

void ObjWriter :: flush(void)
{
	if ( columnNum > 0 && fout )
		fprintf(fout, "\n");

	columnNum = 0;
}

void ObjWriter :: output(char type, int value)
{
	if ( type == 'W' )
		sprintf(buf, " 0x%04X", value);
	else
		sprintf(buf, " %d", value);
	int length = strlen(buf);

	if ( lineType != type || (columnNum + length) > MAX_OBJ_OUTPUT_WIDTH )
		flush();

	if ( columnNum <= 0 )	// new line
	{
		fprintf(fout, "%c", type);
		columnNum = 1;
	}

	fprintf(fout, "%s", buf);
	columnNum += length;
	lineType = type;
}

void ObjWriter :: output(char type, char *str)
{
	int length = strlen(str) + 1;

	if ( lineType != type || (columnNum + length) > MAX_OBJ_OUTPUT_WIDTH )
		flush();

	if ( columnNum <= 0 )	// new line
	{
		fprintf(fout, "%c", type);
		columnNum = 1;
	}

	fprintf(fout, " %s", str);
	columnNum += length;
	lineType = type;
}

void ObjWriter :: output(int val)
{
	sprintf(buf, " %d", val);
	fprintf(fout, "%s", buf);
	columnNum += strlen (buf);
}


void ObjWriter :: output(char *str)
{
	fprintf(fout, " %s", str);
	columnNum += strlen(str) + 1;
}

void ObjWriter :: output(const char *str)
{
	output((char*)str);
}

void ObjWriter :: outputW(int code, char *str, int mask)
{
	sprintf(buf, "0x%04X", code);
	if ( str )
	{
		if ( bankAddressing(code) && mask == 0x7f )
			sprintf(&buf[strlen(buf)], ":%s", str);
		else if ( mask )
			sprintf(&buf[strlen(buf)], "|(%s&%d)", str, mask);
		else
			sprintf(&buf[strlen(buf)], "|%s", str);
	}


	int len = strlen(buf);
	if ( lineType != 'W' || (columnNum+len) >= MAX_OBJ_OUTPUT_WIDTH )
	{
		flush();
		lineType = 'W';
	}

	if ( columnNum == 0 )
	{
		fprintf(fout, "W");
		columnNum++;
	}

	fprintf(fout, " %s", buf);
	columnNum += len+1;
}

void ObjWriter :: outputW(int code, int *val, int mask)
{
	if ( bankAddressing(code) && mask == 0x7f && *val >= 0x2000 )
		*val = (*val - 0x2000)%80 + 0x20;
	
	outputW(code | (*val & mask));
}

void ObjWriter :: outputW(char *str, int mask)
{
	if ( mask )
		sprintf(buf, " (%s&%d)", str, mask);
	else
		sprintf(buf, " %s", str);

	int len = strlen(buf);
	if ( lineType != 'W' || (columnNum+len) >= MAX_OBJ_OUTPUT_WIDTH )
	{
		flush();
		lineType = 'W';
	}

	if ( columnNum == 0 )
	{
		fprintf(fout, "W");
		columnNum++;
	}

	fprintf(fout, "%s", buf);
	columnNum += len;
}

void ObjWriter :: close(void)
{
	fclose(fout);
}

/////////////////////////////////////////////////////////////////
LstWriter :: LstWriter(FILE *_fout)
{
	memset(this, 0, sizeof(LstWriter));
	fout = _fout;
}

void LstWriter :: flush(void)
{
	if ( curLine != NULL && curLine != prtLine )
	{
		while ( columnNum++ < MAX_LST_OUTPUT_WIDTH )
			fprintf (fout, " ");

		fprintf (fout, " %05d\t%s", curLine->lineno, curLine->src);
		prtLine = curLine;
	}
	else if ( columnNum > 0 )
		fprintf (fout, "\n");

	columnNum  = 0;
}

void LstWriter :: output(unsigned int addr, int code)
{
	if ( columnNum > 0 )
		flush();

	fprintf(fout, "%05X: 0x%04X", addr, code);
	columnNum = 13;
}

void LstWriter :: output(unsigned int addr, int code, char type)
{
	output(addr, code);
	if ( type )
		fprintf(fout, "%c", type), columnNum++;
}

void LstWriter :: output(line_t *lp)
{
	while ( columnNum < MAX_LST_OUTPUT_WIDTH )
	{
		fprintf(fout, " ");
		columnNum++;
	}
	fprintf(fout, "% 4d ", lp->lineno);
	if ( lp->src )
		fprintf(fout, "%s", lp->src);
	else
		fprintf(fout, "\n");

	columnNum = 0;
}

void LstWriter :: close(void)
{
	fclose(fout);
}

/////////////////////////////////////////////////////////////////
static bool bankAddressing(int code)
{
	switch ( code & 0x3f80 )
	{
		case 0x0180:	// CLRF
		case 0x0080:	// MOVWF
			return true;
	}

	switch ( code & 0x3c00 )
	{
		case 0x1000:	// BCF
		case 0x1400:	// BSF
		case 0x1800:	// BTFSC
		case 0x1c00:	// BTFSS
			return true;
	}

	switch ( code & 0x3f00 )
	{
		case 0x0700:	// ADDWF
		case 0x0500:	// ANDWF
		case 0x0900:	// COMF
		case 0x0300:	// DECF
		case 0x0b00:	// DECFSZ
		case 0x0a00:	// INCF
		case 0x0f00:	// INCFSZ
		case 0x0400:	// IORWF
		case 0x0800:	// MOVF
		case 0x0d00:	// RLF
		case 0x0c00:	// RRF
		case 0x0200:	// SUBWF
		case 0x0e00:	// SWAPF
		case 0x0600:	// XORWF
		case 0x3d00:	// ADDWFC
		case 0x3b00:	// SUBWFB
		case 0x3500:	// LSLF
		case 0x3600:	// LSRF
		case 0x3700:	// ASRF
			return true;

		default:
			return false;
	}
}