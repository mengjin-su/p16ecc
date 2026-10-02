#ifndef _DNODE_H
#define _DNODE_H
extern "C" {
#include "common.h"
}

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
		void display(int spaces);
};

#endif