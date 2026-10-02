#ifndef _PNODE_H
#define _PNODE_H

#include "item.h"

enum {
	P_FUNC_BEG=1024, P_FUNC_END, P_SRC_CODE, P_INC, P_DEC,	// 1024...
	P_JZ, P_JNZ, P_JEQ, P_JNE, P_JGT, P_JGE, P_JLT, P_JLE,	// 1029...
	P_CAST, P_ARG_PASS, P_ARG_PASS1, P_CALL, P_JBZ, P_JBNZ,	// 1038...
	P_JZ_INC, P_JZ_DEC, P_JNZ_INC, P_JNZ_DEC,				// 1044...
	P_MOV, P_DJNZ, P_IJNZ,									// 1048...
	P_MOV_INC, P_MOV_DEC, P_BRANCH,							// 1051...
	P_SEGMENT, P_FILL, P_COPY, P_ASMFUNC,					// 1054...
	P_DEC_STACK, P_INC_STACK,
};

/////////////////////////////////////////////////////////////////////////
class Pnode {

	public:
    	int      type;
    	Item	*items[3];
    	Pnode   *last;
    	Pnode   *next;

    public:
    	Pnode(int type);
    	Pnode(int type, Item *ip0);
    	~Pnode();

    	void updateItem(int index, Item *ip);
		void updateName(int index, char *str);
    	void insert(Pnode *pp);
    	Pnode *end(void);
};

void addPnode(Pnode **list, Pnode *pnp);
void delPnodes(Pnode **list);

#endif