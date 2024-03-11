#ifndef SYMBOL_H
#define SYMBOL_H
#include "segment.h"

class Symbol {
	public:
		int			type;
	    char		*name;
		Segment		*segment;
		int			value;	// value of the symbol
		int			size;	// for data
	    Symbol	 	*next;

	public:
		Symbol(char *sname);
		Symbol(char *sname, int stype, Segment *seg);
		~Symbol();
};

void   addSymbol(Symbol **slist, Symbol *sp);
void   addSymbol(Symbol **slist, Symbol *sp, int value);
void   deleteSymbols(Symbol **slist);
Symbol *searchSymbol(Symbol *slist, char *str, Segment *seg=NULL);
Symbol *logSymbol(Symbol **slist, int type, char *name, Segment *, int val);
//void   logSegmentSymbols(Segment *seg, Symbol **sym_list);
void   printSymbol(Symbol *slist);

#endif