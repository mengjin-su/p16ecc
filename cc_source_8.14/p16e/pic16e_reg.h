#ifndef _PIC16E_REG_H
#define _PIC16E_REG_H

#define STATUS  	(char*)"STATUS"
#define INDF0       (char*)"INDF0"
#define INDF1       (char*)"INDF1"
#define FSR0L       (char*)"FSR0L"
#define FSR0H       (char*)"FSR0H"
#define FSR1L       (char*)"FSR1L"
#define FSR1H       (char*)"FSR1H"
#define PCLATH      (char*)"PCLATH"
#define PCL			(char*)"PCL"
#define BSR         (char*)"BSR"
#define WREG        (char*)"WREG"
#define INTCON      (char*)"INTCON"

#define aINDF0      0x0
#define aINDF1      0x1
#define aPCL        0x2
#define aSTATUS     0x3
#define aFSR0L      0x4
#define aFSR0H      0x5
#define aFSR1L      0x6
#define aFSR1H      0x7
#define aBSR        0x8
#define aWREG       0x9
#define aPCLATH		0xa
#define aINTCON     0xb

#define ACC0        (char*)"0x70"	//_$acc0$"
#define ACC1        (char*)"0x71"	//_$acc1$"
#define ACC2        (char*)"0x72"	//_$acc2$"
#define ACC3        (char*)"0x73"	//_$acc3$"

#include <string>
#include "pic16e_asm.h"

///////////////////////////////////////////////////////////////////////
class P16E_REG8 {
	public:
		P16E_REG8(char *_name, P16E_ASM *);

		void reset(void);
		char*reset(char *val);		

		void load(char *inst, char *val);	// WREG only
		void load(char *inst, int   val);	// WREG only
		void load(int   val);
		void load(char *val);
		
		void set(int    val);
		void set(char  *val);
		
	protected:
		P16E_ASM   *pic16e;
		std::string name;
		std::string valStr;	
		std::string instStr;
		void update(char *i, char *s);
};

///////////////////////////////////////////////////////////////////////
class P16E_FSR0 {
	private:
		P16E_ASM *pic16e;
		Item	 *ip;
		int		 offset;

	public:
		P16E_FSR0(P16E_ASM *);
		bool loaded(Item *);
		bool load(int index, Item *, int offset=0);
		void inc(int amount, bool asm_output = false);
		void reset(void);
		void reset(Item *_ip);
};


#endif
