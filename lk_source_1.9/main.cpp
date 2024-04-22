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
#include "memory.h"
#include "symbol.h"
#include "p16link.h"

#define VERSION     "v0.1.9"
#define TEMP_FOLDER	"_tmp_"

Segment	*codeSegGroup= NULL;
Segment	*dataSegGroup= NULL;
Segment	*fuseSegGroup= NULL;
Segment	*miscSegGroup= NULL;
Memory  *dataMem = NULL;
Memory  *codeMem = NULL;
Symbol  *fileList= NULL;

static char *outputFile = (char*)"_OUTPUT_";
static Symbol *libList  = NULL;	// list of .lib files
static void link0(char *filename, bool lib_f=false);
static char *getLibFile(char *libfile);
static void cleanup(void);

int     useBSR6  = 0;
char    *libPath;

////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[], char *env[])
{
    int start_parsing = 0;
	Path path(env);
	libPath = path.get();

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
			dataMem = new Memory(DATA_MEMORY);
			codeMem = new Memory(CODE_MEMORY);
			libList = new Symbol((char*)"crt0");
			addSymbol(&libList,  (char*)"crt1");
			start_parsing++;
		}

		link0(argv[i]);   // parse source file
    }

	if ( start_parsing )
	{
		std::string tmp_folder = TEMP_FOLDER;
		system(("mkdir " + tmp_folder).c_str());	// create a temp folder
		for (Symbol *sp = libList; sp; sp = sp->next)
		{
			char *ssp = getLibFile(sp->name);
			if ( ssp ) link0(ssp, 1);
		}
		system(("rm -r " + tmp_folder).c_str());	// remove temp folder

		P16link p16link(dataMem, codeMem);

		p16link.scanInclusion();
		p16link.assignSegmentsAddress();

		if ( p16link.errorCount == 0 )
		{
			char *output = new char[strlen(outputFile)+10];
			sprintf(output, "%s.hex", outputFile);
			p16link.outputHex(output);
			sprintf(output, "%s.map", outputFile);
			p16link.outputMap(output);
			delete [] output;

			int m_used, m_total;
			m_used = dataMem->memUsed(&m_total);
			printf("RAM used: %d bytes (%.2f%c)\n", m_used, (float)m_used*100/(float)m_total, '%');
			m_used = codeMem->memUsed(&m_total);
			printf("ROM used: %d words (%.2f%c)\n", m_used, (float)m_used*100/(float)m_total, '%');
		}

		cleanup();
	}
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

	addSymbol(&fileList, filename);
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
				ip0 = itemPtr(lp->items, 1);	// ram size
				ip1 = itemPtr(lp->items, 2);	// rom size
				if ( ip0 && ip0->type == TYPE_VALUE ) dataMem->resize(ip0->data.val);
				if ( ip1 && ip1->type == TYPE_VALUE ) codeMem->resize(ip1->data.val);
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
						addSymbol(&libList, (char*)str.c_str());
				}
				break;
			case 'N':	// RAM blank
			case 'M':	// ROM blank
				ip0 = itemPtr(lp->items, 0);	// start address
				ip1 = itemPtr(lp->items, 1);	// length
				if ( ip0 && ip0->type == TYPE_VALUE &&
					 ip1 && ip1->type == TYPE_VALUE )
				{
					if ( lp->type == 'N' ) codeMem->blank(ip0->data.val, ip1->data.val);
					if ( lp->type == 'M' ) dataMem->blank(ip0->data.val, ip1->data.val);
				}
				break;
			case 0:		// null line
				break;

			case 'S':	// SEGMENT line...
				seg = NULL;
			default:
				if ( seg == NULL )
				{	// create a segment...
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
			seg->addLine(lp);		// add the line into segment...
			if ( add_seg )			// new segment, add the segment
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
	std::string file_name;
	std::string cc_cmd = "cc16e ";
	std::string as_cmd = useBSR6? "as16e -X -a ": "as16e -a ";
	std::string cp_cmd = "cp ";
	std::string tmp_path = TEMP_FOLDER;
	char *p0 = libfile, *p1;
	FILE *fd;

	while ( (p1 = strchr(p0, '/'))  ) p0 = p1 + 1;
	while ( (p1 = strchr(p0, '\\')) ) p0 = p1 + 1;

	if ( libPath != NULL )
		sprintf(buf, "%slib/%s", libPath, p0);
	else
		sprintf(buf, "%s", p0);

	file_name = buf; file_name += ".c";
	fd = fopen(file_name.c_str(), "r");	// check if the source exists
	if ( fd == NULL ) return NULL;
	fclose(fd);

	// copy the file to current folder...
//	system((cp_cmd + file_name + " .").c_str());
//	file_name = libfile; file_name += ".c";
	system((cp_cmd + file_name + " " + tmp_path).c_str());
	file_name = tmp_path + "/" + libfile + ".c";

	// compile the source
	system((cc_cmd + file_name).c_str());

	// and assemble it
//	file_name = libfile; file_name += ".asm";
	file_name = tmp_path + "/" + libfile + ".asm";
	system((as_cmd + file_name).c_str());

	// return obj file name...
//	file_name = libfile; file_name += ".lib";
	file_name = tmp_path + "/" + libfile + ".lib";
	fd = fopen(file_name.c_str(), "r");
	if ( fd == NULL ) return NULL;

	fclose (fd);
	sprintf(buf, "%s", file_name.c_str());
	return buf;
}

static void cleanup(void)
{
	deleteSegments(codeSegGroup);
	deleteSegments(dataSegGroup);
	deleteSegments(fuseSegGroup);
	deleteSegments(miscSegGroup);
	delete dataMem;
	delete codeMem;
	deleteSymbols(&fileList);
	deleteSymbols(&libList);
}
