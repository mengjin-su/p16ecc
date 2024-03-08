#ifndef P16ASM
#define P16ASM
#include "symbol.h"
#include "fwriter.h"

enum {ASM_PASS1, ASM_PASS2};

#define I_TYPE(ip, t)	(ip && ip->type == t)

typedef enum {VAL_EXP, STR_EXP, BAD_EXP} EXP_TYPE;

class P16E_asm {
	
	private:
		char 	*objFile;
		char 	*lstFile;
		Symbol	*symbolList;
		ObjWriter *objWriter;
		LstWriter *lstWriter;
		
	public:	
		int  errorCount;
		
	public:
		P16E_asm(char *fname, const char *ext);
		~P16E_asm();
		void run(line_t *lp, int pass);
		void output(line_t *lp);
		
	private:
		EXP_TYPE parseItem(item_t *ip, char *buf, int *val, int instr);
		void objOutput(char t, item_t *ip);
		void objOutput(item_t *ip);

		item_t *symbolReplace(Symbol *slist, item_t *ip);
		item_t *mergeItems(int op, item_t *ip1, item_t *ip2);
		void   regulateInst(line_t *lp);
};


#endif