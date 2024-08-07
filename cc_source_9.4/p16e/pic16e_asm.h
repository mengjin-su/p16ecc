#ifndef _PIC16E_ASM_H
#define _PIC16E_ASM_H

#include "pic16e_asm_opt.h"

#define _W_		(char*)"W"
#define _F_		(char*)"F"

class P16E_ASM {
	private:
		FILE *fout;
		P16E_asmOPT *asmOpt;
		bool optimizeEnabled;

	public:
		P16E_ASM(FILE *fileout, bool opt);
		~P16E_ASM();

		void output(char *s);
		void output(const char *s)	{ output((char*)s); }
		void label(char *lbl, bool c=false, char *opr=NULL);

		void code(char *inst);
		void code(char *inst, char *opr1, char *opr2=NULL);

		void code(char *inst, const char *opr1, char *opr2=NULL) { code(inst, (char*)opr1, opr2); }
		void code(const char *inst, char *opr1, char *opr2=NULL) { code((char*)inst, opr1, opr2); }

		void code(char *inst, int opr1, char *opr2=NULL);
		void code(const char *inst, int opr) { code((char*)inst, opr); }

		void code(char *inst, char *opr1, int opr2);
		void code(char *inst, int opr1, int opr2);
};

#define EXTEND_WREG		asm16e->code(_ANDLW, 0x80), \
						asm16e->code(_BTFSS, aSTATUS, 2), \
						asm16e->code(_MOVLW, 0xff), \
						regWREG->reset()

#endif