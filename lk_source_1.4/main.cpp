#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
extern "C" {
#include "common.h"
}
#include "path.h"
#include "segment.h"
#include "symbol.h"
#include "memory.h"
#include "p16link.h"

#define VERSION     "v0.1.3"

static char *outputFile  = (char*)"_OUTPUT_";	// default output
static Symbol *objList = NULL;					// list of .obj files
static Symbol *libList = NULL;					// list of .lib files

static void link0(char *filename, bool lib_f=false);
static char *getLibFile(char *libfile);
static void cleanup(void);

int     useBSR6 = 0;
Memory *dataMem = NULL;
Memory *codeMem = NULL;
Symbol *fileList= NULL;
P16link*p16link = NULL;
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
        {
			if ( strcmp(argv[i], "-o") == 0 )
			{
				if ( ++i < argc )
					outputFile = argv[i];	// assign the output file name
			}
			else if ( strcmp(argv[i], "-X") == 0 )
			{
				useBSR6 = 1;				// set super PIC16xxxx target
			}
			continue;
        }

		if ( !start_parsing )
		{
			dataMem = new Memory(DATA_MEMORY);
			codeMem = new Memory(CODE_MEMORY);
			libList = new Symbol((char*)"crt0");
			start_parsing++;
		}

		link0(argv[i]);   // parse source file
    }

	if ( start_parsing )
	{
		for (Symbol *sp = libList; sp; sp = sp->next)
		{
			char *ssp = getLibFile(sp->name);
			if ( ssp ) link0(ssp, 1);			
		}
	
		p16link = new P16link(dataMem, codeMem);
		p16link->scanLibInclusion();	// lib code inclusion
		p16link->scanFuncCalls();		// create func call matrix

		p16link->print();	
		cleanup();
	}

	delete path;
    return errorCnt;
}

////////////////////////////////////////////////////////////////////////
/*
	parse the input source(.obj) file(s), and generate the raw data in
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
					std::string str = ip0->data.str;
					int pos = str.find_last_of("/");
					if ( pos == std::string::npos ) pos = str.find_last_of("\\");
					if ( pos != std::string::npos )	str = str.substr(pos+1);
	
					pos = str.find("."); 
					if ( pos != std::string::npos )	str = str.substr(0, pos);
	
					if ( !searchSymbol(libList, (char*)str.c_str()) )
						addSymbol(&libList, new Symbol((char*)str.c_str()));
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
					seg 	   = new Segment(filename);
					seg->isLIB = lib_f;
					add_seg    = true;
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
				addSegment(seg, lib_f? 1: 0);
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
		sprintf(buf, "../lib/%s.lib", libfile);

	FILE *fd = fopen(buf, "r");

	if ( fd == NULL )
		return NULL;

	fclose(fd);
	return buf;
}

static void cleanup(void)
{
	deleteSegments(codeSegGroup);
	deleteSegments(dataSegGroup);
	deleteSegments(fuseSegGroup);
	deleteSegments(miscSegGroup);
	deleteSegments(libSegGroup);
	deleteSymbols(&libList);
	delete dataMem;
	delete codeMem;	
	delete p16link;	
}
