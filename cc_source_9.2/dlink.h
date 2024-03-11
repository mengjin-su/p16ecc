#ifndef _DLINK_H
#define _DLINK_H

class Dnode {
	public:
		char   *name;	// data name
		attrib *attr;	// data attributes
		int    index;	// id sequence index
		char   *func;	// owner(funtion) pointer
		int    atAddr;

		int	   elipsis:1;
		node   *parp;	// function pointer's parameters
		Dnode  *next;

	public:
		Dnode (char *_name, attrib *_attr, int _index);
		~Dnode();

		char *nameStr(void);
		void  nameUpdate(char *_name);
		int	  size(void);

		void dimUpdate(node *np);	// dim demension update
		bool dimCheck(node *np);	// dim dimension check
		bool fptrCheck(node *np);	// function pointer check
};

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
		void  addChild(Dlink *lk);	// add a child link

		int   dataCount(void);
		void  add(Dnode *dp);
		Dnode *add(node *np, attrib *attr, int index);
		Dnode *search(char *name, int search_mode);
		Dnode *get(int index);
		bool  nameCheck(char *name, int end_index);
};


void delDlinks(Dlink *dlink);

#endif
