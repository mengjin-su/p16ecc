#include <stdio.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "display.h"

static bool verbal_asm = false;
static void headMark(int level, FILE *file);
static char *oprName(int token);


void display(node *np, int level, FILE *file)
{
    if ( np == NULL ) return;
	if ( file == NULL ) file = stdout;

	dispSrc(np, level, file);
    switch ( np->type )
    {
        case NODE_CON:
            headMark(level, file);
            fprintf(file, "constant node: %ld\n", np->con.value);
            break;

        case NODE_ID:
            headMark(level, file);
            fprintf(file, "id node: %s ", np->id.name);
            display(np->id.attr, '\n', file);
            break;

        case NODE_STR:
            headMark(level, file);
            fprintf(file, "string node: %s\n", np->str.str);
            break;

        case NODE_OPR:
            headMark(level, file);
            if ( np->opr.oper < 127 )
                fprintf(file, "opr node {%d}: %c ", np->opr.nops, np->opr.oper);
            else
                fprintf(file, "opr node {%d}: %s ", np->opr.nops, oprName(np->opr.oper));
			display(np->opr.attr, '\n', file);

            for (int i = 0; i < np->opr.nops; i++)
                display(np->opr.op[i], level+1, file);
            break;

        case NODE_LIST:
            headMark (level, file);
            fprintf (file, "list node {%d}:\n", np->list.nops);

            for (int i = 0; i < np->list.nops; i++)
                display(np->list.ptr[i], level+1, file);
            break;
    }
}

void dispSrc(node *np, int level, FILE *file)
{
	if ( np->id.src )
	{
		if ( file == NULL ) file = stdout;
		headMark(level, file);
		fprintf(file, "SOURCE - ");
		fprintf(file, "%s #%d: %s\n", np->id.src->fileName, np->id.src->lineNum, np->id.src->srcCode);
	}	
}

void display(attrib *ap, int level, char endln, FILE *file)
{
	if ( file == NULL ) file = stdout;
	if ( ap )
	{
        headMark(level, file);
		fprintf(file, "{");

		switch ( ap->type )
		{
			case VOID:			fprintf(file, "void");		break;
			case CHAR:  		fprintf(file, "char");    	break;
			case INT:   		fprintf(file, "int");     	break;
		}

		fprintf(file, "}");
		if ( endln ) fprintf(file, "%c", endln);
	}
}

void display(attrib *ap, char ln, FILE *file)
{
	if ( file == NULL ) file = stdout;
	if ( ap ) display(ap, 0, 0, file);
	if ( ln ) fprintf (file, "%c", ln);
}

void display(attrib *ap, FILE *file)
{
	display(ap, 0, file);
}

static void headMark(int level, FILE *file)
{
	while ( level-- ) fprintf(file, " ");
}

static char *oprName(int token)
{
	static char buf[256];
    switch ( token )
    {
		case FUNC_DEF:	return (char*)"FUNC_DEF";
		case FUNC_DEC:	return (char*)"FUNC_DEC";
		case FUNC_DATA:	return (char*)"FUNC_DATA";
	}
	sprintf (buf, "(oper. token: %d)", token);
	return buf;
}
