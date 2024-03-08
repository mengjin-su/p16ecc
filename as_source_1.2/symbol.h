#ifndef SYMBOL_H
#define SYMBOL_H
extern "C" {
#include "common.h"
#include "asm.h"
}

class Symbol {
	public:
		char		*name;
		int			type;
		bool		global;
		bool		isABS;
		item_t		*item;
		Symbol		*next;
		
	public:
		Symbol(char *s);
		Symbol(char *s, item_t *ip);
		~Symbol();
};


Symbol *addSymbol(Symbol **list, Symbol *s);
Symbol *searchSymbol(Symbol *list, char *s);

#endif
