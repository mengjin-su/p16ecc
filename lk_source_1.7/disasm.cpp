#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "disasm.h"
extern "C" {
#include "common.h"
#include "lnk.h"
}

static char buf[4096];
static char *regName(unsigned int faddr);

/////////////////////////////////////////////////////////////////
const P16inst_t P16instTbl[] = {

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
	{CLRWDT,	0x0064,	0},
	{GOTO,		0x2800,	P16_LIT11_I},
	{IORLW,		0x3800, P16_LIT8_I},
	{MOVLW,		0x3000, P16_LIT8_I},
	{RETFIE,	0x0009, 0},
	{RETLW,		0x3400, P16_LIT8_I},
	{RETURN,	0x0008, 0},
	{SUBLW,		0x3c00, P16_LIT8_I},
	{XORLW,		0x3a00, P16_LIT8_I},
	{RESET,		0x0001,	0},
	{CALLW,		0x000a,	0},
	{BRW,		0x000b,	0},
	{MOVIW_OP,	0x0010,	P16_MOV_OP_I},
	{MOVWI_OP,	0x0018,	P16_MOV_OP_I},
	{MOVIW_OFF,	0x3f00,	P16_MOV_OFF_I},
	{MOVWI_OFF,	0x3f80,	P16_MOV_OFF_I},
	{ADDWFC,	0x3d00, P16_FD_I},
	{SUBWFB,	0x3b00, P16_FD_I},
	{MOVLB,		0x0020, P16_LIT5_I},
	{MOVLB,		0x0140, P16_LIT6_I},
	{ADDFSR,	0x3100, P16_FSR_OP_I},
	{MOVLP,		0x3180, P16_LIT7_I},
	{LSLF,		0x3500, P16_FD_I},
	{LSRF,		0x3600, P16_FD_I},
	{ASRF,		0x3700, P16_FD_I},
	{BRA,		0x3200, P16_LIT9_I},	/* relative addressing */
	{NULL, 		0, 0}
};

char *p16disasm(int addr, int word)
{
	const P16inst_t *instPtr = NULL;
	int mask = 0xffff, v;
	for (int i = 0; i < 12 && !instPtr; i++)
	{
		const P16inst_t *inst = P16instTbl;
		for (; inst->symbl; inst++)
		{
			if ( (word & mask) == inst->code )
			{
				instPtr = inst;
				break;
			}
		}
		mask <<= 1;
	}
	buf[0] = 0;
	if ( instPtr )
	{
		sprintf(buf, "%s", instPtr->symbl);
		char *p = &buf[strlen(buf)];
		char *reg = regName(word & 0x7f);
		
		switch ( instPtr->type )
		{
			case 0:
				break;
				
			case P16_FD_I:
				if ( word & 0x80 )
					sprintf(p, "\t%s, F", reg);
				else	
					sprintf(p, "\t%s, W", reg);
				break;

			case P16_F_I:
				sprintf(p, "\t%s", reg);
				break;
				
			case P16_BIT_I:
				sprintf(p, "\t%s, %d", reg, (word >> 7)&7);
				break;
				
			case P16_LIT8_I:
				sprintf(p, "\t0x%02X", word & 0xff);
				break;
				
			case P16_LIT11_I:
				sprintf(p, "\t0x%03X", word & 0x7ff);
				break;
				
			case P16_LIT5_I:	
				sprintf(p, "\t0x%02X", word & 0x1f);
				break;
				
			case P16_LIT6_I:	
				sprintf(p, "\t0x%02X", word & 0x3f);
				break;

			case P16_LIT7_I:	
				sprintf(p, "\t0x%02X", word & 0x7f);
				break;
				
			case P16_LIT9_I:	
				v = word & 0xff;
				if ( word & 0x100 ) v |= ~0xff;
				sprintf(p, "\t0x%04X", addr + v + 1);
				break;	
				
			case P16_FSR_OP_I:
				if ( word & 0x40 ) strcat(buf, "\tFSR1, ");
				else			   strcat(buf, "\tFSR0, ");
				v = word & 0x1f;
				if ( word & 0x20 ) v |= ~0x1f;
				sprintf(&buf[strlen(buf)], "%d", v);
				break;
				
			case P16_MOV_OP_I:
				strcat(buf, "\t[");
				switch ( word & 3 )
				{
					case 0:	strcat(buf, "++");	break;
					case 1: strcat(buf, "--");	break;
				}
				if ( word & 4 ) strcat(buf, "FSR1");
				else		    strcat(buf, "FSR0");
				switch ( word & 3 )
				{
					case 2:	strcat(buf, "++");	break;
					case 3: strcat(buf, "--");	break;
				}
				strcat(buf, "]");
				break;
				
			case P16_MOV_OFF_I:
				v = word & 0x1f;
				if ( word & 0x20 ) v |= ~0x1f;
				sprintf(p, "\t%d[", v);
				if ( word & 0x40 ) strcat(buf, "FSR1]");
				else			   strcat(buf, "FSR0]");	
				break;
		}
	}
	return buf;
}

const static char *coreRegister[12] = {
	"INDF0", "INDF1", "PCL", "STATUS",
	"FSR0L", "FSR0H", "FSR1L", "FSR1H",
	"BSR", "WREG", "PCLATH", "INTCON",
};

static char *regName(unsigned int faddr)
{
	static char buffer[32];
	
	if ( faddr < 0x0c )	// core register addr 
		return (char*)coreRegister[faddr];
		
	sprintf(buffer, "0x%02X", faddr);
	return buffer;
}