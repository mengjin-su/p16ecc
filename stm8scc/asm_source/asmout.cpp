#include <stdio.h>
#include <string.h>
#include "asmout.h"

#define MAX_OBJ_OUTPUT_WIDTH    80
#define MAX_LST_OUTPUT_WIDTH    24
#define ADDR_FIELD_WIDTH		7

static void fillUpAddrField(int *col, FILE *fout, int field_width);


asm_obj :: asm_obj(const ST8_inst_t *p)
{
	cnt = 0;

	if ( p->code_size > 1 )
		add(p->code >> 8, 'B');

	add(p->code & 0xff, 'B');
}

asm_obj :: asm_obj(int val, char t)
{
	cnt = 0;
	add(val, t);
}

asm_obj :: ~asm_obj()
{
	for (int i = 0; i < cnt; i++)
	{
		if ( !items[i].const_val )
			delete [] items[i].s;
	}
}

void asm_obj :: add(int val, char t)
{
	items[cnt].type = t;
	items[cnt].v  = val;
	items[cnt].const_val = 1;
	cnt++;
}

void asm_obj :: add(char *s, char t)
{
	items[cnt].type = t;
	items[cnt].s  = new char[strlen(s)+1];
	strcpy(items[cnt].s, s);
	items[cnt].const_val = 0;
	cnt++;
}

void asm_obj :: print(char *buf, int index)
{
	if ( index < cnt )
	{
		switch ( items[index].type )
		{
			case 'B':
				if ( items[index].const_val )
					sprintf(buf, " 0x%02X", items[index].v & 0xff);
				else
					sprintf(buf, " %s", items[index].s);
				break;

			case 'W':
				if ( items[index].const_val )
					sprintf(buf, " [0x%04X]", items[index].v & 0xffff);
				else
					sprintf(buf, " [%s]", items[index].s);
				break;

			case 'T':
				if ( items[index].const_val )
					sprintf(buf, " {0x%06X}", items[index].v & 0xffffff);
				else
					sprintf(buf, " {%s}", items[index].s);
				break;

			case 'D':
				if ( items[index].const_val )
					sprintf(buf, " <0x%08X>", items[index].v);
				else
					sprintf(buf, " <%s>", items[index].s);
				break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: flush()
{
	if ( cur_col > 0 && fout != NULL )
	{
		fprintf(fout, "\n");
		cur_col = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(asm_obj *op)
{
	if ( fout == NULL )
		return;

	for (int i = 0; i < op->cnt; i++)
	{
		char buf[200];
		op->print(buf, i);

		int size = strlen(buf);

		if ( cur_col > 0 && (cur_col + size) > MAX_OBJ_OUTPUT_WIDTH )
			flush();

		cur_type = 'B';

		if ( cur_col == 0 )
		{
			fprintf(fout, "%c", cur_type);
			cur_col = 1;
		}

		fprintf(fout, "%s", buf);
		cur_col += size;
	}
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(char type, int val)
{
	char buf[20], cnt = 0;
	int size;	// string length in bytes
	unsigned int value = (unsigned int)val;

	if ( fout == NULL )
		return;

	switch ( type )
	{
		case 'B':	cnt = 1;	size = 5;				break;
		case 'W':	cnt = 2;	size = 10;	type = 'B';	break;
		case 'T':	cnt = 3;	size = 15;	type = 'B';	break;
		case 'D':	cnt = 4;	size = 20;	type = 'B';	break;
		default:    sprintf(buf, " %d", value);
					size = strlen(buf);
	}

	if ( cur_col > 0 && (cur_type != 'B' || (cur_col + size) > MAX_OBJ_OUTPUT_WIDTH) )
		flush();

	cur_type = type;

	if ( cur_col == 0 )
	{
		fprintf(fout, "%c", cur_type);
		cur_col = 1;
	}

	if ( cnt )
	{
		while ( cnt-- )
			fprintf(fout, " 0x%02X", (value >> (cnt*8)) & 0xff);
	}
	else
	{
		fprintf(fout, "%s", buf);
	}

	cur_col += size;
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(char type, char *str)
{
	if ( fout == NULL )
		return;

	int length = strlen(str) + 1;
	char tmp_type = type;

	if ( type == 'W' || type == 'T' || type == 'D' )
    {
		tmp_type = 'B';
        length += 2;
    }

	if ( cur_col > 0 && (cur_type != tmp_type || (cur_col + length) > MAX_OBJ_OUTPUT_WIDTH) )
		flush();

	cur_type = tmp_type;
	if ( cur_col == 0 )
	{
		fprintf(fout, "%c", cur_type);
		cur_col = 1;
	}

	switch ( type )
	{
        case 'W':   fprintf(fout, " [%s]", str);   break;
        case 'T':   fprintf(fout, " {%s}", str);   break;
        case 'D':   fprintf(fout, " <%s>", str);   break;
        case 'I':   fprintf(fout, " \"%s\"", str); break;
        default:    fprintf(fout, " %s", str);     break;

	}

    cur_col += length;
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(char *str)
{
	if ( fout != NULL )
	{
		if ( cur_col > MAX_OBJ_OUTPUT_WIDTH && cur_type == 'B' )
		{
			flush();
			fprintf(fout, "B %s", str);
			cur_col = 1;
		}
		else
			fprintf(fout, " %s", str);

		cur_col += strlen(str) + 1;
	}
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(int val)
{
	if ( fout != NULL )
	{
		char buf[20];
		sprintf(buf, " %d", val);
		fprintf(fout, "%s", buf);
		cur_col += strlen(buf);
	}
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(char type, char *str1, char *str2)
{
	output(type, str1);
	output(str2);
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(char type, char *str1, int val2)
{
	output(type, str1);
	output(val2);
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(char type, int val1, char *str2)
{
	output(type, val1);
	output(str2);
}

///////////////////////////////////////////////////////////////////////////
void Cout_obj :: output(char type, int val1, int val2)
{
	output(type, val1);
	output(val2);
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: flush()
{
	if ( cur_line != NULL && cur_line != prt_line )
	{
		while ( cur_col++ < MAX_LST_OUTPUT_WIDTH )
			fprintf(fout, " ");

		fprintf(fout, " %05d\t%s", cur_line->lineno, cur_line->src);
		prt_line = cur_line;
	}
	else if ( cur_col > 0 )
		fprintf(fout, "\n");

	cur_col  = 0;
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: output(int addr, line_t *line)
{
	if ( cur_line != NULL && cur_line != line )
		flush();

	cur_line = line;
	if ( cur_col > 0 )
		return;

	fprintf(fout, "%05X:", addr & 0xfffff);
	cur_col = ADDR_FIELD_WIDTH - 1;
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: output(int addr, char *buf, line_t *line)
{
	if ( cur_line != NULL && cur_line != line )
		flush();

	fprintf(fout, "%05X:", addr & 0xfffff);

	int i = 0;
	if ( buf )
	{
		for (; buf[i] && i < 16; i++)
			fprintf(fout, "%c", buf[i]);
	}

	while ( i++ < 18 )
		fprintf(fout, " ");

	fprintf(fout, " %05d\t%s", line->lineno, line->src);
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: output(int data, line_t *line, int type)
{
	if ( cur_line != line )
		flush();

	cur_line = line;
	char buf[20];


	switch ( type )
	{
        case 'p':   sprintf(buf, "?");      		break;
        case 'c':
        case 'g':   sprintf(buf, " %04X", data);   	break;
		case 'W':	sprintf(buf, " %04X", data);	break;
		default:	sprintf(buf, " (%d)", data);
	}
	int length = strlen(buf);

	if ( (cur_col+length) >= MAX_LST_OUTPUT_WIDTH )
		flush();

	cur_col += length;
	fillUpAddrField(&cur_col, fout, ADDR_FIELD_WIDTH);

	fprintf(fout, "%s", buf);
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: output(line_t *line)
{
	flush();			// flush out the current line (if any)
	cur_line = line;	// load the this line
	flush();			// and flush out it.
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: output(line_t *line, int addr, int value, int bytes)
{
	if ( cur_line && cur_line != line )
		flush();

	cur_line = line;
	if ( cur_col > 0 )
		return;

	fprintf(fout, "%05X:", addr & 0xfffff);
	cur_col = ADDR_FIELD_WIDTH-1;

	output(value, bytes);
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: output(line_t *line, int addr, int code, char *str, int bytes)
{
	if ( cur_line && cur_line != line )
		flush();

	cur_line = line;
	if ( cur_col > 0 )
		return;

	fprintf(fout, "%05X:", addr & 0xfffff);
	cur_col = ADDR_FIELD_WIDTH-1;

	fprintf(fout, " %02X", code & 0xff);
	cur_col += 3;
	while ( bytes-- )
	{
		fprintf(fout, " ??");
		cur_col += 3;
	}
}

///////////////////////////////////////////////////////////////////////////
void Cout_lst :: output(int value, int bytes)
{
	if ( bytes > 2 )
	{
		fprintf(fout, " %02X", (value >> 16) & 0xff);
		cur_col += 3;
	}

	if ( bytes > 1 )
	{
		fprintf(fout, " %02X", (value >> 8) & 0xff);
		cur_col += 3;
	}

	fprintf(fout, " %02X", value & 0xff);
	cur_col += 3;
}

void Cout_lst :: output(char *str, int bytes)
{
	while ( bytes-- )
	{
		fprintf(fout, " ??");
		cur_col += 3;
	}
}

///////////////////////////////////////////////////////////////////////////
static void fillUpAddrField(int *col, FILE *fout, int field_width)
{
	while ( *col < field_width )
	{
		(*col)++;
		fprintf(fout, " ");
	}
}
