#ifndef _SYMBOL_H
#define _SYMBOL_H

extern "C" {
#include "common.h"
}

#include "seg.h"

/* ---------------------------------------------------- */
typedef struct symbol {
    char		  *name;
    char		  *owner;
    item_t		  *item;
    int			   lineno;
	int			   type;
	Cseg		  *seg;
	int			   attr;
	int			   index;
	int			   global : 1;
	int			   system : 1;
	int			   isABS  : 1;
    struct symbol *next;
} symbol_t;

extern symbol_t *symbol_list;

symbol_t *search_symbol(line_t *line, char *symbol);
symbol_t *add_symbol(Cseg *seg, line_t *line, char *symbol, item_t *iptr, int attr);
void del_symbol_list(void);
void disp_symbol(void);

#endif
