#include <stdio.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "asm.h"
}
#include "p16inst.h"
#include "symbol.h"

/////////////////////////////////////////////////////////////////
const P16inst_t P16_inst_code_tbl[] = {
	{ADDWF, 	0x0700,	P16_FD_I},
	{ANDWF,		0x0500, P16_FD_I},
	{CLRF,		0x0180, P16_F_I},
	{CLRW,		0x0100, 0},
	{COMF,		0x0900, P16_FD_I},
	{DECF,		0x0300, P16_FD_I},
	{DECFSZ,	0x0b00, P16_FD_I},
	{INCF,		0x0a00, P16_FD_I},
	{INCFSZ,	0x0f00, P16_FD_I},
	{IORWF,		0x0400, P16_FD_I},
	{MOVF,		0x0800, P16_FD_I},
	{MOVWF,		0x0080, P16_F_I},
	{NOP,		0x0000,	0},
	{RLF,		0x0d00, P16_FD_I},
	{RRF,		0x0c00, P16_FD_I},
	{SUBWF,		0x0200, P16_FD_I},
	{SWAPF,		0x0e00, P16_FD_I},
	{XORWF,		0x0600, P16_FD_I},

	{BCF,		0x1000, P16_BIT_I},
	{BSF,		0x1400, P16_BIT_I},
	{BTFSC,		0x1800, P16_BIT_I},
	{BTFSS,		0x1c00, P16_BIT_I},

	{ADDLW,		0x3e00, P16_LIT8_I},
	{ANDLW,		0x3900, P16_LIT8_I},
	{CALL,		0x2000,	P16_LIT11_I},
	{GOTO,		0x2800,	P16_LIT11_I},
	{IORLW,		0x3800, P16_LIT8_I},
	{MOVLW,		0x3000, P16_LIT8_I},

	{RETFIE,	0x0009, 0},
	{RETLW,		0x3400, P16_LIT8_I},
	{RETURN,	0x0008, 0},
	{CLRWDT,	0x0064, 0},
	{SLEEP,		0x0063, 0},
	{SUBLW,		0x3c00, P16_LIT8_I},
	{XORLW,		0x3a00, P16_LIT8_I},

	// following are the enhanced instructions
	{RESET,		0x0001,	0},			
	{CALLW,		0x000a,	0},
	{BRW,		0x000b,	0},
	{ADDWFC,	0x3d00, P16_FD_I},
	{SUBWFB,	0x3b00, P16_FD_I},
	{MOVLB,		0x0020, P16_LIT5_I},
	{ADDFSR,	0x3100, P16_FSR_OP_I},
	{MOVLP,		0x3180, P16_F_I},
	{LSLF,		0x3500, P16_FD_I},
	{LSRF,		0x3600, P16_FD_I},
	{ASRF,		0x3700, P16_FD_I},
	{BRA,		0x3200, P16_LIT9_I},
	{MOVIW_OP,	0x0010,	P16_MOV_OP_I},
	{MOVWI_OP,	0x0018,	P16_MOV_OP_I},
	{MOVIW_OFF,	0x3f00,	P16_MOV_OFF_I},
	{MOVWI_OFF,	0x3f80,	P16_MOV_OFF_I},
	{0, 0, 0}
};

//////////////////////////////////////////////////////////////////////
P16inst_t *P16inst (int inst)
{
	const P16inst_t *p = P16_inst_code_tbl;
	while ( p->token != 0 )
	{
		if ( p->token == inst )
			return (P16inst_t *)p;

		p++;
	}
	return NULL;
}
