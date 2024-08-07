#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "const.h"

Const_t :: Const_t(char *s, int indx)
{
	str   = dupStr(s);
	index = indx;
	next  = NULL;
}

Const_t :: ~Const_t()
{
	free(str);
}

char *Const_t :: strName(void)
{
	static char buf[16];
	sprintf(buf, "_$CS%d", index);
	return buf;
}

////////////////////////////////////////////////////
Const :: Const()
{
	list = NULL;
	index = 0;
}

Const :: ~Const()
{
	while ( list )
	{
		Const_t *next = list->next;
		delete list;
		list = next;
	}
}

char *Const :: add(char *str)
{
	Const_t *p = list;
	bool exist = false;

	for (; p; p = p->next)
	{
		if ( strcmp(p->str, str) == 0 )
		{
			exist = true;
			break;
		}
	}

	if ( !exist )
	{
		p = new Const_t(str, ++index);
		if ( list == NULL )
			list = p;
		else
		{
			Const_t *tmp = list;
			while ( tmp->next ) tmp = tmp->next;
			tmp->next = p;
		}
	}
	return p->strName();
}
