#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" {
#include "common.h"
}
#include "path.h"
#include "segment.h"
#include "symbol.h"
#include "memory.h"

#define VERSION     "v0.1.2"

static char *outputFile  = (char*)"_OUTPUT_";	// default output
static Symbol *objList = NULL;					// list of .obj files
static Symbol *libList = NULL;					// list of .lib files

static void link0(char *filename, bool lib_f=false);
static char *getLibFile(char *libfile);
static void addSegment(Segment *seg);

int    useBSR6  = 0;
Memory *dataMem = NULL;
Memory *codeMem = NULL;
Symbol *fileList= NULL;
char   *libPath;

////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[], char *env[])
{
    int start_parsing = 0;
	Path *path = new Path(env);
	libPath    = path->get();

   	printf ("PIC16E linker, %s\n", VERSION);

    for (int i = 1; i < argc; i++)
    {
        if ( argv[i][0] == '-' )
        {	// assign the output file name
			if ( strcmp(argv[i], "-o") == 0 )
			{
				if ( ++i < argc )
					outputFile = argv[i];
			}
			else if ( strcmp(argv[i], "-X") == 0 )
			{
				useBSR6 = 1;
			}
			continue;
        }

		if ( !start_parsing )
		{
			dataMem   = new Memory(DATA_MEMORY);
			codeMem   = new Memory(CODE_MEMORY);
			start_parsing++;
		}

		link0(argv[i]);   // parse source file
    }

	if ( start_parsing )
	{
		deleteSegments(codeSegGroup);
		deleteSegments(dataSegGroup);
		deleteSegments(fuseSegGroup);
		deleteSegments(miscSegGroup);
		deleteSymbols(&libList);
		delete dataMem;
		delete codeMem;
	}

	delete path;
    return errorCnt;
}

////////////////////////////////////////////////////////////////////////
/*
	parse the input source file, and generate the raw file in
	'line_t' format.
*/
////////////////////////////////////////////////////////////////////////
static void link0(char *filename, bool lib_f)
{
	if ( searchSymbol(fileList, filename) )
		return;

	addSymbol(&fileList, new Symbol(filename));

    curFile = filename;
    printf("linking '%s' ...\n", filename);

	// using Lex and Bison to parse the source ...
	errorCnt = 0;
    line_t *lp = _main(filename);
	if ( errorCnt > 0 )	exit(1);

	Segment *seg = NULL;
	while ( lp )
	{
		bool save_line = false;
		bool add_seg   = false;
		item_t *ip0, *ip1;

		switch ( lp->type )	// input line type?
		{
			case 'P':	// processor
				ip0 = itemPtr(lp->items, 1);
				ip1 = itemPtr(lp->items, 2);
				if ( ip0 ) dataMem->resize(ip0->data.val);
				if ( ip1 ) codeMem->resize(ip1->data.val);
				break;

			case 'I':	// include lib file
				ip0 = itemPtr(lp->items, 0);
				if ( ip0 && ip0->type == TYPE_STRING )
				{
					if ( !searchSymbol(libList, ip0->data.str) )
						addSymbol(&libList, new Symbol(ip0->data.str));
				}
				break;

			case 'N':	// RAM blank
			case 'M':	// ROM blank
				ip0 = itemPtr(lp->items, 0);	// 1st item in line
				ip1 = itemPtr(lp->items, 1);	// 2nd item in line
				if ( ip0 && ip0->type == TYPE_VALUE &&
					 ip1 && ip1->type == TYPE_VALUE )
				{
					if ( lp->type == 'N' ) codeMem->blank(ip0->data.val, ip1->data.val);
					if ( lp->type == 'M' ) dataMem->blank(ip0->data.val, ip1->data.val);
				}
				break;
			case 0:
				break;

			case 'S':
				seg = NULL;
			default:
				if ( seg == NULL )
				{
					seg = new Segment(filename);
					seg->isLIB = lib_f;
					add_seg = true;
				}
				save_line = true;
				break;
		}

		line_t *next = lp->next;
		lp->next = NULL;

		if ( save_line )
		{
			seg->addLine(lp);	// add the line into segment...
			if ( add_seg )
			{
				seg->init();
				addSegment(seg);
			}
		}
		else
			freeLine(lp);

		lp = next;
	}
}

//////////////////////////////////////////////
static char *getLibFile(char *libfile)
{
	static char buf[4096];

	if ( libPath != NULL )
		sprintf(buf, "%slib/%s.lib", libPath, libfile);
	else
		sprintf(buf, "%s.lib", libfile);

	FILE *fd = fopen(buf, "r");

	if ( fd == NULL )
		return NULL;

	fclose(fd);
	return buf;
}

static void addSegment(Segment *seg)
{
	Segment **listp;
	int type = seg->type();

	if ( seg->isLIB )
		listp = &libSegGroup;
	else
		switch ( type )
		{
			case CONST_SEGMENT:
			case CODE_SEGMENT:	listp = &codeSegGroup;	break;
			case DATA_SEGMENT:	listp = &dataSegGroup;	break;
			case FUSE_SEGMENT:	listp = &fuseSegGroup;	break;
			default:			listp = &miscSegGroup;	break;
		}

	Segment *sp = *listp, *last = NULL;
	if ( sp == NULL )
	{
		*listp = seg;
		return;
	}

	for (; sp; last = sp, sp = sp->next)
	{
		if ( type == MISC_SEGMENT )
			continue;

		bool insert = false;
		int cmp_result = strcmp(seg->name(), sp->name());

		if ( cmp_result > 0 ) continue;
		if ( cmp_result < 0 ) insert = true;
		if ( cmp_result == 0 && sp->isBEG ) continue;

		if ( insert || sp->isEND || seg->isBEG )
		{
			seg->next = sp;
			if ( last )	last->next = seg;
			else *listp = seg;
			return;
		}
	}

	last->next = seg;
}
