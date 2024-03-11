#ifndef _PNODE_H
#define _PNODE_H

#include "item.h"

enum {
	P_FUNC_BEG = 1024, P_FUNC_END, P_SRC_CODE,
	P_JZ, P_JNZ, P_JEQ, P_JNE, P_JGT, P_JGE, P_JLT, P_JLE,
	P_CAST, P_ARG_PASS, P_ARG_CLEAR, P_CALL, P_JBZ, P_JBNZ,
	P_JZ_INC, P_JZ_DEC, P_JNZ_INC, P_JNZ_DEC,
	P_MOV, P_DJNZ, P_IJNZ,
	P_MOV_INC, P_MOV_DEC, P_BRANCH,
	P_SEGMENT, P_FILL, P_COPY, P_ASMFUNC,
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