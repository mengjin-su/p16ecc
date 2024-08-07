#ifndef _ASM_OPT_H
#define _ASM_OPT_H

#include <string>

typedef struct {
	bool insertTAB;
	std::string inst, opr1, opr2;
	bool nullLine(void) {
		return (inst.c_str()[0] == '\n' ||
		  	    inst.c_str()[0] == ';'   );
	}
	bool isInst(char *_inst, char *des=NULL)
	{
		if ( inst != _inst ) return false;
		return (des == NULL || opr2 == des);
	}
} AsmLine;

#define ASM_BUFFER_SIZE		32
#define NEXT_INDEX(pt)		((pt + 1) & (ASM_BUFFER_SIZE-1))
#define LAST_INDEX(pt)		((pt - 1) & (ASM_BUFFER_SIZE-1))
#define SKIP_BSEL			true

class P16E_asmOPT {

	public:
		P16E_asmOPT(FILE *fileout);
		~P16E_asmOPT() { flush(); }
		void output(char *s);
		void label(char *lbl, bool c=false, char *opr=NULL);
		void code(char *inst);
		void code(char *inst, char *opr1, char *opr2=NULL);
		void code(char *inst, int opr1, char *opr2=NULL);
		void code(char *inst, char *opr1, int opr2);
		void code(char *inst, int opr1, int opr2);

	private:
		FILE *fout;
		AsmLine outputBuffer[ASM_BUFFER_SIZE];
		int outputHead;
		int outputTail;
		bool append_str;

		void flush(void);
		void flush(int flush_lines);
		bool isConst(std::string str, int *n);
		bool isINDF0(std::string str);
		bool isINDF1(std::string str);
		bool isTempVar(std::string s);
		bool isWREG(std::string r);
		bool isVarPair(std::string s1, std::string s2);
		bool isTempPair(std::string s1, std::string s2);
		bool isAddrPair(std::string s1, std::string s2);
		bool isUpdate_WandZ(AsmLine *p);
		bool renewWREG(AsmLine *p);
		bool sameOperand(AsmLine *p1, AsmLine *p2);

		bool bufferFull(void);
		int  bufferDepth(void);

		AsmLine *nextLine(int idx, int *, bool skip_bsel=false);
		void removeLine(int idx);

		int optimize(void);
		int case1(void);
		int case2(void);
		int case3(void);
		int case4(void);
		int case5(void);
		int case6(void);
		int case7(void);
		int case8(void);
		int case9(void);
};

#endif