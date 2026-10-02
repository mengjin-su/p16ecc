#ifndef _PCODER_H
#define _PCODER_H
#include <string.h>
extern "C" {
#include "common.h"
}
#include "dnode.h"
#include "pnode.h"
#include "nlist.h"
#include "dlink.h"
#include "flink.h"
#include "const.h"

class JumpStack {
	private:
    	int FILO[128];
    	int index;

 	public:
 		JumpStack ()	{ index = 0; }
 		void reset ()	{ index = 0; }
 		void push(int lbl) { if (index < 128) FILO[index++] = lbl; }
 		int  pop () 	{ return (index > 0)? FILO[--index]: 0; }
 		int  top ()		{ return (index > 0)? FILO[index-1]: 0; }
		int  depth()	{ return index; }
};

extern Dlink *dataLink;	// data link
extern Flink *funcLink;	// function link;

#define PUT_LBL(lbl)	addPnode(&mainPcode, new Pnode(LABEL, lblItem(lbl)))

class Pcoder {
	public:
		Nlist *nlist;
		Pnode *mainPcode;
		Pnode *initPcode;
		Pnode *constPcode;

		src_t *src;
		int   errCount;
		int   warningCount;
		Dlink *curDlink;	// current data link
		Fnode *curFnode;	// current func node

		int	  labelSeed;
		int   getLbl(void) { return ++labelSeed; }

		#define MAX_TEMP_INDEX	(16*32)
		unsigned int tempIndexMask[16];

		JumpStack continueStack;
		JumpStack breakStack;
		Item  *iStack;
		Const *constGroup;

	private:
		void warningPrint(const char *msg, char *opt_msg);
		void errPrint(const char *msg, char *opt_msg);
		void errPrint(const char *msg) { errPrint(msg, NULL); }

		void PUSH(Item *ip)	{	// push in an Item
			ip->next = iStack;
			iStack = ip;
		}
		void CLEAR(void) {		// clear the stack
			while ( iStack ) {
				Item *inext = iStack->next;
				delete iStack;
				iStack = inext;
			}
		}
		Item *POP(void) {		// pop out an Item
			Item *ip = iStack;
			if ( ip ) iStack = ip->next;
			return ip;
		}
		Item *TOP(void) {		// get the top Item
			return iStack;
		}
		int DEPTH(void) {		// get the depth
			int n = 0;
			for(Item *ip = iStack; ip; ip = ip->next)
				n++;
			return n;
		}
		void DEL(void) {		// delete top Item
			if ( iStack ) delete POP();
		}
		void DEL(int n) {
			while ( n-- ) DEL();
		}

	public:
		Pcoder(Nlist *nlist);
		~Pcoder();

		void run (node *np);
		void run (oprNode_t *np);

		void takeSrc(node *np);
		void romDataInit(char *dname, attrib *attr, node *init, Dnode *dp, int mem_addr);
		void ramDataInit(char *dname, attrib *attr, node *init, Dnode *dp, int mem_addr);

		void run2(oprNode_t *op);
		void run3(oprNode_t *op);
		void run4(oprNode_t *op);
		void run5(oprNode_t *op);
		void run6(oprNode_t *op);
		void run7(oprNode_t *op);
		void run8(oprNode_t *op);
		void run9(oprNode_t *op);

		Item *makeTemp(void);
		Item *makeTemp(attrib *ap);
		Item *makeTemp(int size);
		void releaseTemp(Item *ip);
		Item *makeOffset(Item *ip, int scale);
		void ptrBiasing(int code, Item *ip0, Item *ip1);

		void logicBranch(node *np, int t_lbl, int f_lbl, int next_lbl);
		void logicBranch(Item *ip, int t_lbl, int f_lbl, int next_lbl);
		void compareBranch(int op, Item *ip0, Item *ip1, int t_lbl, int f_lbl, int next_lbl);
		void compareBranch(int op, node *np0, node *np1, int t_lbl, int f_lbl, int next_lbl);
		void switchListProc(Item *ip, node *list);
		int  passParameter(attrib *attr, node *par, int offset, bool ext_par);
		int  passParameter(attrib *attr, node *par);
		bool buildInAsm(node *np);
};

typedef struct {
	const char *name;
	char 	   *inst;
	int			pars;
} asmFunc;

#endif
