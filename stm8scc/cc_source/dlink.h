#ifndef _DLINK_H
#define _DLINK_H
extern "C" {
#include "common.h"
}
#include "dnode.h"

enum {LOCAL_SEARCH, WHOLE_SEARCH};	// how to search Dlink
enum {PAR_DLINK, VAR_DLINK};		// type of Dlink

class Dlink {
	public:
		int   dtype;
		Dnode *dlist;
		Dlink *parent, *next, *child;
		Dnode *search(char *name);

	public:
		Dlink(int type);
		~Dlink();

		void  add(Dlink *lk);		// add a sibling link
		void  addChild (Dlink *lk);	// add a child link

		int   dataCount(void);
		void  add(Dnode *dp);
		Dnode *add(node *np, attrib *attr, int index);
		Dnode *search(char *name, int search_mode);
		Dnode *get(int index);
		bool  nameCheck(char *name, int end_index);
		void  display(int spaces);
};


void delDlinks(Dlink *dlink);

#endif
