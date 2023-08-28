#ifndef DISASM_H
#define DISASM_H

enum {
	P16_FD_I = 1,
	P16_F_I,
	P16_BIT_I,
	P16_LIT8_I,
	P16_LIT11_I,
	P16_FSR_OP_I,
	P16_MOV_OP_I,
	P16_MOV_OFF_I,
	P16_LIT5_I,
	P16_LIT6_I,
	P16_LIT9_I,
	P16_LIT7_I
};

typedef struct P16_inst {
	char 		  *symbl;
	unsigned short code;
	char  		   type;
} P16inst_t;

// --- P16Fxxx instruction set ---
#define ADDWF		(char*)"addwf"
#define ANDWF		(char*)"andwf"
#define CLRF		(char*)"clrf"
#define COMF		(char*)"comf"
#define DECF		(char*)"decf"
#define DECFSZ		(char*)"decfsz"
#define INCF		(char*)"incf"
#define INCFSZ		(char*)"incfsz"
#define IORWF		(char*)"iorwf"
#define MOVF		(char*)"movf"
#define MOVWF		(char*)"movwf"
#define RLF         (char*)"rlf"
#define RRF         (char*)"rrf"
#define SUBWF		(char*)"subwf"
#define NOP			(char*)"nop"
#define SWAPF		(char*)"swapf"
#define XORWF		(char*)"xorwf"
#define BCF			(char*)"bcf"
#define BSF			(char*)"bsf"
#define BTFSC		(char*)"btfsc"
#define BTFSS		(char*)"btfss"
#define CALL		(char*)"call"
#define CLRWDT		(char*)"clrwdt"
#define GOTO		(char*)"goto"
#define RETFIE		(char*)"retfie"
#define RETLW		(char*)"retlw"
#define RETURN		(char*)"return"
#define ADDLW		(char*)"addlw"
#define ANDLW		(char*)"andlw"
#define IORLW		(char*)"iorlw"
#define MOVLW		(char*)"movlw"
#define SUBLW		(char*)"sublw"
#define XORLW		(char*)"xorlw"
#define CLRW		(char*)"clrw"
#define ADDWFC		(char*)"addwfc"
#define SUBWFB		(char*)"subwfb"
#define LSLF		(char*)"lslf"		// logical shift left
#define LSRF		(char*)"lsrf"		// logical shift right
#define ASRF		(char*)"asrf"		// arithmatic shift right
#define MOVLP		(char*)"movlp"		// load PCLATH
#define MOVLB		(char*)"movlb"		// load BSR
#define BRA			(char*)"bra"
#define BRW			(char*)"brw"
#define CALLW		(char*)"callw"
#define ADDFSR		(char*)"addfsr"
#define MOVIW_OP	(char*)"moviw"
#define MOVWI_OP	(char*)"movwi"
#define MOVIW_OFF	(char*)"moviw"
#define MOVWI_OFF	(char*)"movwi"
#define RESET		(char*)"reset"

char *p16disasm(int addr, int word);

#endif