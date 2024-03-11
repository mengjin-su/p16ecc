#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include "common.h"
extern "C" {
#include "cc.h"
int aConstant(char *s, int type);
}
#include "option.h"

#define HELP_PROMPT	"\n'cc16e' Command Line Format:\n\n\tcc16e [option(s)] file1.c file2.c ...\n\n"
#define TEMP_FILE	(char*)"~mcu.c"
#define MCU_FILE	TEMP_FILE"_"

Option *option;

Option :: Option()
{
	nnpList = NULL;
	level   = 's';	// -Os
	debug   = false;
	mcuFile = NULL;
}

Option :: ~Option()
{
	while ( nnpList )
	{
		Nnode *next = nnpList->next;
		delete nnpList;
		nnpList = next;
	}

	if ( mcuFile )
		remove(mcuFile);
}

bool Option :: add(char *str)
{
	char *p;
	const char *prompt = "unknown/unsupported option";
	int length = strlen(str);

	switch ( str[0] )
	{
		case '?': case 'h':
			printf(HELP_PROMPT);
			return false;

		case 'O':	// optimization level
			if ( length == 2 )
			{
				level = str[1];
				return true;
			}
			break;

		case 'd':
			return (debug = true);

		case 'D':	// #define ...
			if ( str[1] != '=' && length > 2 )
			{
				p = strchr(str, '=');
				if ( p && p[1] )	// search start of ID
				{
					int  id_len = strlen(str) - strlen(p);	// ID length
					char id_buf[id_len];
					memcpy(id_buf, &str[1], id_len); id_buf[id_len] = 0;

					if ( !get('D', 0, id_buf) )				// re-defined?
					{
						addNode(new Nnode('D', id_buf, makeNode(p + 1)));
						return true;
					}
					prompt = "re-defined";
				}
				if ( p == NULL && !get('D', 0, &str[1]) )
				{
					addNode(new Nnode('D', &str[1]));
					return true;
				}
			}
			break;

		case 'M':	// mcu model (indlude <xxx.h>)
			if ( str[1] == '=' && length > 2 && !get('M', 0) )
			{
				FILE *mcu_file = fopen(TEMP_FILE, "w");
				if ( mcu_file )
				{
					fprintf(mcu_file, "#include <%s.h>\n", &str[2]);
					fclose(mcu_file);

					std::string mcu_f = "cpp1 "; mcu_f += TEMP_FILE;
					int rtcode = system(mcu_f.c_str());
					remove(TEMP_FILE);

					if ( rtcode != 0 )
						return false;

					addNode(new Nnode('M', &str[2]));
					mcuFile = MCU_FILE;
					return true;
				}
			}
			break;
	}
	printf("%s - %s\n", prompt, str);
	return false;
}

Nnode *Option :: get(int type, int index, char *id)
{
	for (Nnode *head = nnpList; head; head = head->next)
	{
		if ( head->type == type )
		{
			if ( type == 'D' && id )
			{
				if ( strcmp(id, head->name) == 0 )
					return head;

				continue;
			}

			if ( index-- == 0 )
				return head;
		}
	}
	return NULL;
}

node *Option :: makeNode(char *s)
{
	if ( s[0] == '0' && toupper(s[1]) == 'X' )	return conNode(aConstant(s, 16), INT);
	if ( s[0] == '0' && toupper(s[1]) == 'B' )	return conNode(aConstant(s,  2), INT);
	if ( s[0] == '0' && isdigit(s[1])	     )	return conNode(aConstant(s,  8), INT);
	if ( s[0] == '\'' )							return conNode(aConstant(s,  0), CHAR);
	if ( isdigit(s[0]) )						return conNode(atoi(s), INT);
	return idNode(s);
}

void Option :: addNode(Nnode *np)
{
	if ( nnpList == NULL )
		nnpList = np;
	else
	{
		Nnode *head = nnpList;
		while ( head->next ) head = head->next;
		head->next = np;
	}
}