#ifndef _OPTIMIZER_H
#define _OPTIMIZER_H

#include "pnode.h"

#define NEXT(h, i)		next(h, i)
#define ITYPE(p, t)		(p && p->type == t)
#define PTYPE(p, t)		(p && p->type == t)

class Optimizer {

	public:
		int optiCount;
		Pcoder *pcoder;
		Pnode  *head, *funcPtr;
		Item   *ip0, *ip1, *ip2;

	public:
		Optimizer(Pcoder *pcoder);
		void run(void);

	private:
		Pnode *next(Pnode *p, int offset);
		bool updateLbl(char *old_lbl, char *new_lbl, Pnode *p);
		bool updateGoto(char *old_lbl, char *new_lbl, Pnode *p);
		bool unusedLbl(Pnode *pnp, char *lbl, Pnode *p);
		bool unusedTmp(Pnode *pnp, int tmp_idx);
		bool endOfScope(Pnode *pnp);
		bool assignmentCode(int code);
		Pnode *indirReferenced(Pnode *p, Item *ip, int *index);
		void replaceTmp(Pnode *pnp, int tmp_old, int tmp_new);
		bool reduceTmpSize(Pnode *pnp, Item *tmp, int size);
		bool accReferenced(Pnode *pnp);
		bool replaceIndir(Pnode *pnp, Item *ip, int temp_index);
		bool bitSelect(Item *ip, int *n);
		bool bitDeselect(Item *ip, int *n, int size);
		
		bool case1(Pnode *pnp);
		bool case2(Pnode *pnp);
		bool case3(Pnode *pnp);
		bool case4(Pnode *pnp);
		bool case5(Pnode *pnp);
		bool case6(Pnode *pnp);
		bool case7(Pnode *pnp);
		bool case8(Pnode *pnp);
		bool case9(Pnode *pnp);
		bool case10(Pnode *pnp);
		bool case11(Pnode *pnp);
		bool case12(Pnode *pnp);
		bool case13(Pnode *pnp);
		bool case14(Pnode *pnp);		
		bool case15(Pnode *pnp);
		bool case16(Pnode *pnp);
		bool case17(Pnode *pnp);
		bool case18(Pnode *pnp);
		bool case19(Pnode *pnp);
		bool case20(Pnode *pnp);		
		bool case21(Pnode *pnp);			
};

#endif