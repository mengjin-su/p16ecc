#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "memory.h"
#include "segment.h"
#include "symbol.h"
#include "p16link.h"


P16link :: P16link(Memory *ram, Memory *rom)
{
	memset(this, 0, sizeof(P16link));
	dataMem = ram;
	codeMem = rom;
}

P16link :: ~P16link()
{
	deleteSymbols(&symbList);
//	delete fcallHandler;
}

void P16link :: print(FILE *fo)
{
	fout = fo;
	printfSegments(codeSegGroup);	//print(codeSegGroup); 
	printfSegments(dataSegGroup);	//print(dataSegGroup); 
	printfSegments(fuseSegGroup);	//print(fuseSegGroup); 
	printfSegments(miscSegGroup);	//print(miscSegGroup); 
}

void P16link :: print(Segment *sp)
{
	if ( fout == NULL ) fout = stdout;
	for(; sp; sp = sp->next)
	{
		int stype = sp->type();
		switch ( stype )
		{
			case CODE_SEGMENT:
			case DATA_SEGMENT:
			case FUSE_SEGMENT:
			case CONST_SEGMENT:
				fprintf(fout, "%s... %d\n", sp->name(), sp->addr);
				break;

			default:
				fprintf(fout, "Misc...\n");
				break;
		}

		print(sp->lines);
	}
}

void P16link :: print(line_t *lp)
{
	if ( fout == NULL ) fout = stdout;
	for (; lp; lp = lp->next)
	{
		if ( lp->type )
			fprintf(fout, "%c", lp->type);
		else
			fprintf(fout, " ");

		print(lp->items, ' ');
		fprintf(fout, "\n");
	}
}

void P16link :: print(item_t *ip, int c)
{
	if ( fout == NULL ) fout = stdout;
	for (; ip; ip = ip->next)
	{
		if ( c ) fprintf(fout, "%c", c);
		switch ( ip->type )
		{
			case TYPE_SYMBOL:
			case TYPE_STRING:	fprintf(fout, "%s", ip->data.str);	 break;
			case TYPE_VALUE:	fprintf(fout, "0x%x", ip->data.val); break;

			case '+':	case '-':	case '*':	case '/': 	case '%':
			case '|':	case '&':	case '^':	case ':':
				if ( c == 0 ) fprintf(fout, "(");
				print(ip->left);
				fprintf(fout, "%c", ip->type);
				print(ip->right);
				if ( c == 0 ) fprintf(fout, ")");
				break;

			case LSHIFT:
				if ( c == 0 ) fprintf(fout, "(");
				print(ip->left);
				fprintf(fout, "<<");
				print(ip->right);
				if ( c == 0 ) fprintf(fout, ")");
				break;

			case RSHIFT:
				if ( c == 0 ) fprintf(fout, "(");
				print(ip->left);
				fprintf(fout, ">>");
				print(ip->right);
				if ( c == 0 ) fprintf(fout, ")");
				break;
		}
	}
}

