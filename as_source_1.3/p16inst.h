#ifndef _P16E_INST_H
#define _P16E_INST_H

// PIC16E instruction formats...
enum {
	P16_FD_I = 1,	// ADDWF x, W
	P16_F_I,		// CLRF
	P16_BIT_I,		// BTFSS x, 0
	P16_LIT5_I,		// MOVLB
	P16_LIT8_I,		// MOVLW
	P16_LIT9_I,		// BRA
	P16_LIT11_I,	// GOTO, CALL
	P16_FSR_OP_I,	// ADDFSR
	P16_MOV_OP_I,	// MOVIW ++FSR0
	P16_MOV_OFF_I,	// MOVIW n[FSR0]
};

typedef struct {
	int token;
	int code;
	int form;
} P16inst_t;

P16inst_t *P16inst(int inst);

#endif
