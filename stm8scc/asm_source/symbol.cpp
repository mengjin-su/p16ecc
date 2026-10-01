#include <stdio.h>
#include <string.h>
#include <malloc.h>
extern "C" {
#include "define.h"
}
#include "symbol.h"
#include "seg.h"

symbol_t *symbol_list = NULL;

/* ---------------------------------------------------------------------------- */
symbol_t *search_symbol(line_t *line, char *symbol)
{
    symbol_t *p;

    for (p = symbol_list; p != NULL; p = p->next)
    {
        int match;

        match = !strcmp(p->name, symbol);

        if ( match )
        {
            if ( line == NULL )
                return p;

            if ( p->global || p->system || !strcasecmp(p->owner, line->fname) )
                return p;
        }
    }

    return NULL;
}

/* ---------------------------------------------------------------------------- */
symbol_t *add_symbol(Cseg *seg, line_t *line, char *symbol, item_t *iptr, int attr)
{
    symbol_t *p = search_symbol (line, symbol);
    bool in_list = false;

    if ( p != NULL )
    {
        if ( line != NULL && p->lineno == line->lineno )
        {
			freeItem (iptr);
            return p;
		}

		printf("%s : ", symbol);
        my_yyerror(line, "Symbol redefined!");

		freeItem(iptr);
        return NULL;
    }

	if ( !in_list )
	{
	    p = (symbol_t *)MALLOC(sizeof(symbol_t));
	    p->name = dupStr (symbol);

	    if ( symbol_list == NULL )
	    {
	        symbol_list = p;
	    }
	    else
	    {
	        symbol_t *tmp = symbol_list;

	        while ( tmp->next != NULL )
	            tmp = tmp->next;

	        tmp->next = p;
	    }
	}

    p->item = iptr;
    p->attr = attr;

    if ( line != NULL )
    {
        p->owner  = line->fname;
        p->global = line->global_lbl;
        p->lineno = line->lineno;
    }

    p->seg = seg;
    return p;
}

/* ---------------------------------------------------------------------------- */
void del_symbol_list (void)
{
    while ( symbol_list != NULL )
    {
        symbol_t *p = symbol_list->next;

        free (symbol_list->name);
        free (symbol_list);

        symbol_list = p;
    }
}

/* ---------------------------------------------------------------------------- */
void disp_symbol (void)
{
	symbol_t *p = symbol_list;
	while ( p != NULL )
	{
		printf ("%s [%c]\n", p->name, p->isABS? 'A': 'N');
		p= p->next;
	}
}
