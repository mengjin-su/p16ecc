#ifndef _OPTION_H
#define _OPTION_H

#include "nlist.h"

class Option {
	public:
		Option();
		~Option();

		bool  add(char *s);
		Nnode *get(int type, int index, char *id = NULL);

	public:
		int   level;
		bool  debug;
		char *mcuFile;
		bool  extendedMode;

	private:
		Nnode *nnpList;
		node  *makeNode(char *s);
		void   addNode(Nnode *np);
};

extern Option *option;

#define EXT_MODE	(option->extendedMode)

#endif
