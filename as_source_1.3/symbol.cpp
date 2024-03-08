#include <string.h>
#include "symbol.h"

Symbol :: Symbol(char *s)
{
	memset(this, 0, sizeof(Symbol));
	name = new char[strlen(s)+1];
	strcpy(name, s);
}

Symbol :: Symbol(char *s, item_t *ip)
{
	memset(this, 0, sizeof(Symbol));
	name = new char[strlen(s)+1];
	strcpy(name, s);
	item = cloneItem(ip);
}

Symbol :: ~Symbol()
{
	delete name;
	freeItem(item);
}

/////////////////////////////////////////////
Symbol *addSymbol(Symbol **list, Symbol *s)
{
	if ( *list == NULL )
		*list = s;
	else
	{
		Symbol *l = *list;
		while ( l->next ) l = l->next;
		l->next = s;
	}
	return s;
}

void delSymbol(Symbol **list)
{
	if ( list && *list )
	{
		for(Symbol *sp = *list; sp != NULL;)
		{
			Symbol *next = sp->next;
			delete sp;
			sp = next;
		}
		*list = NULL;
	}
}

Symbol *searchSymbol(Symbol *list, char *s)
{
	Symbol *sp = NULL;
	while ( list )
	{
		if ( strcmp(list->name, s) == 0 )
			sp = list;
		
		list = list->next;
	}
	return sp;
}