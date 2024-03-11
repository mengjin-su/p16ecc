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
            fprintf(file, "constant node: %ld ", np->con.value);
            display(np->con.attr, verbal_asm? 0: '\n', file);
            break;

        case NODE_ID:
            headMark(level, file);
            fprintf(file, "id node: %s ", np->id.name);
            display(np->id.attr, verbal_asm? 0: '\n', file);
			display(np->id.dim, level+1, file);
			if ( np->id.fp_decl && np->id.parp )
			{
				headMark(level+1, file);
                fprintf(file, "fptr parameters:");
                if ( !verbal_asm ) fprintf(file, "\n");
				display(np->id.parp, level+1, file);
			}
			if ( np->id.init )
			{
				headMark(level+1, file);
				fprintf(file, "init...:");
				if ( !verbal_asm ) fprintf(file, "\n");
				display(np->id.init, level+1, file);
			}
            break;

        case NODE_STR:
            headMark(level, file);
            fprintf(file, "string node: %s", np->str.str);
            if ( !verbal_asm ) fprintf(file, "\n");
            break;

        case NODE_OPR:
            headMark(level, file);
            if ( np->opr.oper < 127 )
                fprintf(file, "opr node {%d}: %c ", np->opr.nops, np->opr.oper);
            else
                fprintf(file, "opr node {%d}: %s ", np->opr.nops, oprName(np->opr.oper));
			display(np->opr.attr, verbal_asm? 0: '\n', file);

            for (int i = 0; i < np->opr.nops; i++)
                display(np->opr.op[i], level+1, file);
            break;

        case NODE_LIST:
            headMark (level, file);
            fprintf (file, "list node {%d}:", np->list.nops);
            if ( !verbal_asm ) fprintf(file, "\n");

            for (int i = 0; i < np->list.nops; i++)
                display(np->list.ptr[i], level+1, file);
            break;
    }
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
		case FUNC_HEAD:		return (char*)"FUNC_HEAD";
		case FUNC_DECL: 	return (char*)"FUNC_DECL";
		case DATA_DECL: 	return (char*)"DATA_DECL";
	}
	sprintf (buf, "(oper. token: %d)", token);
	return buf;
}

void display(attrib *ap, int level, char endln, FILE *file)
{
	if ( file == NULL ) file = stdout;
	if ( ap )//&& ap->type )
	{
        headMark(level, file);
		fprintf(file, "{");

		if ( ap->dataBank )
		{
			if ( ap->dataBank == CONST )
                fprintf(file, "C:");
			else if ( ap->dataBank == LINEAR )
                fprintf(file, "L:");
		}

		if ( ap->isNeg )	  fprintf(file, "- ");
		if ( ap->isVolatile ) fprintf(file, "volatile ");
		if ( ap->isTypedef )  fprintf(file, "typedef ");
		if ( ap->isStatic )   fprintf(file, "static ");
		if ( ap->isExtern )   fprintf(file, "extern ");
		if ( ap->isUnsigned ) fprintf(file, "unsigned ");
		if ( ap->typeName )   fprintf(file, "(%s) ", ap->typeName);

		switch ( ap->type )
		{
			case VOID:			fprintf(file, "void");		break;
			case SBIT:  		fprintf(file, "sbit");  	break;
			case CHAR:  		fprintf(file, "char");    	break;
			case INT:   		fprintf(file, "int");     	break;
			case FPTR:  		fprintf(file, "Fptr");    	break;
			case SHORT: 		fprintf(file, "short");   	break;
			case LONG:  		fprintf(file, "long");    	break;
			case STRUCT:		fprintf(file, "STRUCT"); 	break;
			case UNION: 		fprintf(file, "UNION");  	break;
			case TYPEDEF_NAME:	fprintf(file, "TYPE_NAME");	break;
		}

		for(int i = 0; ap->ptrVect && i < ap->ptrVect[0]; i++)
			if ( ap->ptrVect[i+1] == CONST ) fprintf(file, " C*");
			else							 fprintf(file, " *");

		if ( ap->atAddr )
			display((node *)ap->atAddr, level+1, file);

		if ( ap->dimVect )
		{
			for (int i = 0; i < ap->dimVect[0]; i++)
				fprintf(file, "[%d]", ap->dimVect[i+1]);
		}

		fprintf(file, "}");
		if ( endln ) fprintf(file, "%c", endln);

		if ( endln && (ap->type == STRUCT || ap->type == UNION) )
			display((node*)ap->newData, level+1, file);
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

void dispSrc(node *np, int level, FILE *file)
{
	if ( np->id.src )
	{
		if ( file == NULL ) file = stdout;
		headMark(level, file);
		fprintf(file, "%s #%d: %s\n", np->id.src->fileName, np->id.src->lineNum, np->id.src->srcCode);
	}
}
