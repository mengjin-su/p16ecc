#ifndef _PRESCAN_H
#define _PRESCAN_H
#include "common.h"

class PreScan
{
	public:
		PreScan(Nlist *_nlist);
		node *scan(node *np);

	private:
		Nlist *nlist;
		int   errCount;		
		int   tagSeed;
		src_t *src;
		int   enumNeg;	// min item size in an enum set
		int   enumPos;	// max item size in an enum set
		
		node *scan(idNode_t  *ip);
		node *scan(oprNode_t *op);
		int   mergeCon(int n1, int n2, int oper);
		node *replacePar (node *np, node *name_par, node *real_par);
		void  errPrint(const char *msg, char *extra=NULL);
		char *tagLabel(void);
		void  attrRestore(attrib **ap, bool update_flags=false);
		bool  StUnNameCheck(node *np, char *name);
		
};

#endif