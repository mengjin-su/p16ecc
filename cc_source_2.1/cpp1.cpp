#include <stdio.h>
#include <string.h>
#include <string>
#include "cpp1.h"

typedef struct File_t_ {
	std::string		name;
	FILE			*file_in;
	int  			line_no;
	bool            sys_file;
	struct File_t_  *next;
} File_t;

static File_t *fileStack = NULL;
static FILE   *output = NULL;
static char   includeType = 0;
static char   lineBuffer[4096];
static std::string sysPath = "";

static int  parse(File_t *file);
static int  readLine(FILE *fin, char *buffer);
static char *skipSP(char *line);

#define IS_SLASH(c)		((c) == '/' || (c) == '\\')
#define IS_SEPERATOR(c)	((c) == '=' || (c) == ';' || (c) == ':')

//////////////////////////////////////////////////////////////////////////
int cpp1(char *env[], char *file_in, char *file_out, bool sys_file)
{
	if ( file_out )
		output = fopen(file_out, "w");

	if ( env )
	{
		sysPath = "../include/";
		for (; env && *env; env++) 	// search for "PATH" setting
			if ( strncasecmp(*env, "Path=", 5) == 0 )
			{
				char *p = strstr(*env, "\\iCC16e");
				if ( p == NULL ) p = strstr(*env, "/iCC16e");
				if ( p && IS_SLASH(p[7]) && memcmp(p+8, "bin", 3) == 0 )
				{
					int i = 0;
					while ( !IS_SEPERATOR(p[i-1]) ) i--;
					sysPath = &p[i];
					sysPath = sysPath.substr(0, 7-i) + "/include/";
				}
			}
	}

	for (File_t *fp = fileStack; fp; fp = fp->next)
		if ( fp->name == file_in && fp->sys_file == sys_file )
			return 0;

	File_t *file = new File_t;
	file->file_in= fopen(file_in, "r");

	if ( file->file_in == NULL )
	{
		printf("can't open file '%s'!\n", file_in);
		return -1;
	}

	file->name = file_in;
	file->line_no = 0;
	file->sys_file = sys_file;

	file->next = fileStack;
	fileStack = file;

	int rtcode = parse(file);

	fclose(file->file_in);
	delete file;
	file = fileStack = fileStack->next;

	if ( fileStack == NULL )
		fclose(output);

	return rtcode;
}

static int parse(File_t *file)
{
	int line_mark = 1;
	for (;;)
	{
		int length = readLine(file->file_in, lineBuffer);
		if ( length <= 0 ) return 0;
		file->line_no++;

		char *p = skipSP(lineBuffer), *p1;
		if ( memcmp(p, "#include", 8) == 0 )
		{
			int rtcode;
			std::string file_path = sysPath;

			p = skipSP(p + 8);
			switch ( includeType = *p++ )
			{
				case '"':	// user source file
					if ( (p1 = strchr(p, '"')) != NULL )
					{
						*p1 = '\0';
						rtcode = cpp1(NULL, p, NULL, 0);
						if ( rtcode < 0 ) return rtcode;
						break;
					}
					return -1;
				case '<':	// system source file
					if ( (p1 = strchr(p, '>')) != NULL )
					{
						*p1 = '\0';
						file_path += p;
						p = (char*)file_path.c_str();
						rtcode = cpp1(NULL, p, NULL, 1);
						if ( rtcode < 0 ) return rtcode;
						break;
					}
					return -1;
			}
			line_mark = 1;
		}
		else
		{
			if ( line_mark )
			{
				if ( includeType == '<' )
					fprintf(output, "#LINE %d %s\n", file->line_no, file->name.c_str());
				else
					fprintf(output, "#line %d %s\n", file->line_no, file->name.c_str());
			}

			line_mark = 0;
			fprintf(output, "%s", lineBuffer);
		}
	}
}

static int readLine(FILE *fin, char *buffer)
{
	int length = 0;
	while ( fin )
	{
		char c;
		fscanf(fin, "%c", &c);

		if ( feof(fin) ) break;

		if ( c != '\r' )
		{
			buffer[length++] = c;
			if ( c == '\n' ) break;
		}
	}

	if ( length > 0 && buffer[length-1] != '\n' )
		buffer[length++] = '\n';

	buffer[length] = 0;
	return length;
}

static char *skipSP(char *p)
{
	while ( *p == ' ' || *p == '\t' ) p++;
	return p;
}
