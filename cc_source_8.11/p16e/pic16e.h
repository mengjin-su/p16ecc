#ifndef _PIC16E_H
#define _PIC16E_H
#include "../sizer.h"
#include "pic16e_reg.h"
#include "pic16e_asm.h"

#define VERSION		"v1.00"

#define CONST_ID_ITEM(ip)	(ip->type == ID_ITEM && ip->attr->dataBank == CONST)
#define CONST_ITEM(ip)		(ip->type == CON_ITEM || ip->type == IMMD_ITEM || ip->type == LBL_ITEM)
#define INDF_PP(indf)		((strcmp(indf, INDF0)==0)? "INDF0++": "INDF1++")
#define ZERO_LOC			0x7f
#define FLAG_LOC			0x7e
#define BUFFERED_FSR1L		0x7c
#define BUFFERED_FSR1H		0x7d
#define ISR_FSR1L			0x7a
#define ISR_FSR1H			0x7b

extern P16E_REG8 			*regWREG;
extern P16E_REG8 			*regBSR;
extern P16E_REG8 			*regPCLATH;
extern P16E_FSR0			*regFSR0;

#define ASM_OUTP			asm16e->output
#define ASM_LABL			asm16e->label
#define ASM_CODE			asm16e->code

class PIC16E {
	public:
		PIC16E(char *out_file, Nlist *_nlist, Pcoder *_pcoder);
		~PIC16E();
		void run(void);

	private:
		FILE   		*fout;
		Nlist  		*nlist;
		Pcoder 		*pcoder;
		P16E_ASM 	*asm16e;
		Fnode  		*curFunc;
		char		*srcCode;
		Pnode		*curPnode;
		int 		errors;
		int			accSave;
		bool		isrStackSet;	// set stack pointer request

		// buffers for internal operation use
		#define BUF_COUNT  32
		int 	bufIndex;
		char	buf[BUF_COUNT][4096];
		char	*STRBUF() { return buf[bufIndex++ & (BUF_COUNT-1)]; }

	private:
		void errPrint(const char *);
		int  outputData(Dlink *dlink);
		void outputASM0(Pnode *plist);

		// pic16e0.cpp
		bool useFSR(Item *ip);
		bool useFSR(Item *ip0, Item *ip1);
		bool useFSR(Item *ip0, Item *ip1, Item *ip2);
		char *setFSR(Item *ip, int fsr);
		char *setFSR0(Item *ip);
		char *setFSR1(Item *ip);
		char *itemStr(Item *ip, int offset);
		char *acceItem(Item *ip, int offset, char *indf = NULL);
		void fetch(Item *ip, int offset, char *indf);
		void store(Item *ip, int offset, char *indf);
		Item *storeToACC(Item *ip, int limit);
		bool isWREG(Item *ip);

		// pic16e1.cpp
		void funcBeg(Pnode *pp);
		void funcEnd(void);
		void call(char *func);

		// pic16e2.cpp
		void mov(Item *ip0, Item *ip1);
		void movImmd(Item *ip0, Item *ip1);
		void movIndir(Item *ip0, Item *ip1);
		void movBlock(Item *ip0, Item *ip1);

		void moveToBit(Item *ip0, Item *ip1);
		void moveFromBit(Item *ip0, Item *ip1);
		void moveBetweenBits(Item *ip0, Item *ip1);

		// pic16e3.cpp
		void incValue(Item *ip, int value);
		void compl2(Item *ip0, Item *ip1);
		void compl2(Item *ip0);
		void neg(Item *ip0, Item *ip1);
		void compl1(Item *ip0);
		void compl1(Item *ip0, Item *ip1);
		void notop(Item *ip0);

		// pic16e4.cpp
		void notop(Item *ip0, Item *ip1);
		void addsub(int code, Item *ip0, Item *ip1);
		void andorxor(int code, Item *ip0, int n);
		void andorxor(int code, Item *ip0, Item *ip1);

		// pic16e5.cpp
		void jzjnz(int code, Item *ip0, Item *ip1);
		void jbzjbnz(int code, Item *ip0, Item *ip1, Item *ip2);
		void passarg(Item *ip0, Item *ip1, Item *ip2);
		void mov_incdec(int code, Item *ip0, Item *ip1, Item *ip2);
		void pcall(Item *ip);
		void jzjnz_incdec(int code, Item *ip0, Item *ip1, Item *ip2);
		void addsub(int code, Item *ip0, Item *ip1, Item *ip2);
		void add(Item *ip0, Item *ip1, Item *ip2);
		void sub(Item *ip0, Item *ip1, Item *ip2);

		// pic16e6.cpp
		void jeqjne(int code, Item *ip0, Item *ip1, Item *ip2);
		void jeqjneImmd(int code, Item *ip0, Item *ip1, Item *ip2);
		void jeqjneRomId(int code, Item *ip0, Item *ip1, Item *ip2);
		void jeqjneBits(int code, Item *ip0, Item *ip1, Item *ip2);

		// pic16e7.cpp
		void cmpJump(int code, Item *ip0, Item *ip1, Item *ip2);
		void cmpJumpConst(int code, Item *ip0, Item *ip1, Item *ip2);
		void cmpJump(bool signedCmp, int code, Item *ip2);

		// pic16e8.cpp
		void leftAssign(Item *ip0, Item *ip1);
		void leftAssignConst(Item *ip0, Item *ip1);
		void rightAssign(Item *ip0, Item *ip1, bool sshift);
		void rightAssignConst(Item *ip0, Item *ip1);
		void rightAssignConstSigned(Item *ip0, Item *ip1);

		// pic16e9.cpp
		void leftOpr(Item *ip0, Item *ip1, Item *ip2);
		void leftOpr(Item *ip0, Item *ip1);
		void leftOprIndf1(Item *ip0, Item *ip1);
		void rightOpr(Item *ip0, Item *ip1, Item *ip2);
		void rightOprIndf1(Item *ip0, Item *ip1);
		void rightOpr(Item *ip0, Item *ip1);
		void rightOpr(Item *ip0, Item *ip1, int byte_shift);

		// pic16e10.cpp
		void mulAssign(Item *ip0, Item *ip1);
		void mulAssign8(Item *ip0, Item *ip1);
		void mul(Item *ip0, Item *ip1, Item *ip2);
		void mul8(Item *ip0, Item *ip1, Item *ip2);
		void pushStack(Item *ip0, int size);

		// pic16e11.cpp
		void divmodAssign(int code, Item *ip0, Item *ip1);
		void divmodAssignU(int code, Item *ip0, Item *ip1);
		void divmod(int code, Item *ip0, Item *ip1, Item *ip2);
		void divmodU(int code, Item *ip0, Item *ip1, Item *ip2);
		void pragma(Item *ip0, Item *ip1);

		// pic16e12.cpp
		void outputInit(Pnode *);
		void outputConst(Pnode *);
		void outputString(void);

		// pic16e13.cpp
		void andorxor(int code, Item *ip0, Item *ip1, Item *ip2);
		void andorxor(int code, Item *ip0, Item *ip1, int n);
		void andorxor8(int code, Item *ip0, Item *ip1, Item *ip2);
		void asmfunc(Item *ip0, Item *ip1, Item *ip2);
		void djnz(int code, Item *ip0, Item *ip1);
};

#endif