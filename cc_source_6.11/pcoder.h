#ifndef _PCODER_H
#define _PCODER_H
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
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
 		JumpStack()		{ index = 0; }
 		void reset()	{ index = 0; }
 		void push(int lbl) { if (index < 128) FILO[index++] = lbl; }
 		int  pop() 		{ return (index > 0)? FILO[--index]: 0; }
 		int  top()		{ return (index > 0)? FILO[index-1]: 0; }
		int  depth()	{ return index; }
};

#define INTERRUPT_ISR_NAME 	((char*)"_$interrupt")
#define PUT_LBL(lbl)		addPnode(&mainPcode, new Pnode(LABEL, lblItem(lbl)))

extern Dlink *dataLink;	// data link
extern Flink *funcLink;	// func link;

class Pcoder {
	public:
		Pcoder();
		~Pcoder();
		void run(node *np);

		void  takeSrc(node *np);
		Pnode *mainPcode;
		Pnode *initPcode;
		Pnode *constPcode;
		src_t *src;
		int   errorCount;
		int   warningCount;

		Dlink *curDlink;	// current data link
		Fnode *curFnode;	// current func node

		int	  labelSeed;
		int   getLbl(void) { return ++labelSeed; }
		Item  *iStack;
		Const *constGroup;

		JumpStack continueStack;
		JumpStack breakStack;

		#define MAX_TEMP_INDEX	(16*32)
		unsigned int tempIndexMask[16];

	private:
		Item *makeTemp(void);
		Item *makeTemp(attrib *ap);
        Item *makeTemp(int size);
		void releaseTemp(Item *ip);
		bool mergeConst(int opcode, Item *ip0, Item *ip1);

		void warningPrint(const char *msg, char *opt_msg);
		void errPrint(const char *msg, char *opt_msg);
		void errPrint(const char *msg) { errPrint(msg, NULL); }
		void setLbl(int lbl) {
			addPnode(&mainPcode, new Pnode(LABEL, lblItem(lbl)));
		}
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

		void romDataInit(char *dname, attrib *attr, node *init, Dnode *dp, int mem_addr);
		void ramDataInit(char *dname, attrib *attr, node *init, Dnode *dp, int mem_addr);

		Item *makeOffset(Item *ip, int scale);
		void ptrBiasing(int code, Item *ip0, Item *ip1);

		void logicBranch(node *np, int t_lbl, int f_lbl, int next_lbl);
		void logicBranch(Item *ip, int t_lbl, int f_lbl, int next_lbl);
		void compareBranch(int op, Item *ip0, Item *ip1, int t_lbl, int f_lbl, int next_lbl);
		void compareBranch(int op, node *np0, node *np1, int t_lbl, int f_lbl, int next_lbl);
		void switchListProc(Item *ip, node *list);
		void passParameter(attrib *attr, node *par, int par_index, int par_count);
		bool buildInAsm(node *np);

		void run1(oprNode_t *np);
		void run2(oprNode_t *np);
		void run3(oprNode_t *np);
		void run4(oprNode_t *np);
		void run5(oprNode_t *np);
		void run6(oprNode_t *np);
		void run7(oprNode_t *np);
		void run8(oprNode_t *np);
};

typedef struct {
	const char *name;
	char 	   *inst;
	int			pars;
} asmCode;

#endif
