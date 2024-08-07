#ifndef _OPTIMIZER_H
#define _OPTIMIZER_H

#include "pnode.h"

#define NEXT(h, i)		next(h, i)
#define ITYPE(ip, t)	(ip && ip->type == t)
#define PTYPE(pp, t)	(pp && pp->type == t)

#define MOVE_OP			(1 << 0)
#define MATH_OP			(1 << 1)
#define LOGIC_OP		(1 << 2)
#define MONO_OP			(1 << 3)
#define SHIFT_OP		(1 << 4)

#define ALL_OP			(MOVE_OP | MATH_OP | LOGIC_OP | MONO_OP | SHIFT_OP)


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
		bool assignmentCode(int code, int types);
		Pnode *indirReferenced(Pnode *p, Item *ip, int *index);
		void replaceTmp(Pnode *pnp, int tmp_old, int tmp_new);
		bool reduceTmpSize(Pnode *pnp, Item *tmp, int size);
		bool accReferenced(Pnode *pnp);
		bool replaceIndir(Pnode *pnp, Item *ip, int temp_index);
		bool bitSelect(Item *ip, int *n);
		bool bitDeselect(Item *ip, int *n, int size);

		bool group1(Pnode *pnp);	// simplify
		bool group2(Pnode *pnp);	// merge
		bool group3(Pnode *pnp);	// jump
		bool group4(Pnode *pnp);	// special constance
		bool group5(Pnode *pnp);	// temporary variable
		bool group6(Pnode *pnp);	// others
};

#endif