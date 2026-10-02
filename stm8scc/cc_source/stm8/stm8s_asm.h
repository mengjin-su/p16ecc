#ifndef _STM8S_ASM_H
#define _STM8S_ASM_H

#include <string>
#include "stm8s_reg.h"

typedef struct {
	std::string instBuf, oprand1Buf, oprand2Buf;
} OutputBuffer;

class STM8S_ASM {
	private:
		FILE *fout;

	public:
		STM8S_ASM(FILE *fileout);

		void output(int n);
		void output(char c);
		void output(char *s);
		void output(const char *s);
		void output_ln(char *s=NULL);

		void label(int lbl);
		void label(char *lbl, bool c);
		void label_ln(char *lbl, bool c);

		void code0(char *inst);
		void code1(char *inst, const char *opr);
		void code1(char *inst, char *opr, char immd=0);
		void code1(char *inst, int opr, char immd=0);
		void code2(char *inst, char *opr1, char *opr2, char immd=0);
		void code2(char *inst, char *opr1, int   opr2, char immd=0);
		void code2(char *inst, int   opr1, char *opr2, char immd=0);
		void code2(char *inst, int   opr1, int   opr2, char immd=0);
		void codeLbl(char *inst, int lbl);
		void icode(char *inst);
	private:
		bool outputBuffered;
		char instBuf[64], oprand1Buf[4096], oprand2Buf[4096];
		bool redundant(char *inst, char *opr1, char *opr2, bool immd);
		bool redundant(char *inst, char *opr1, int   opr2, bool immd);
		bool redundant(char *inst, int   opr1, char *opr2, bool immd);
		bool redundant(char *inst, int   opr1, int   opr2, bool immd);
};

#endif