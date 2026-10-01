#ifndef _ST8INST_H
#define _ST8INST_H

enum {
	ST8_REGA = 1,
	ST8_REGX,
	ST8_REGY,
	ST8_REGSP,
	ST8_REGXH,
	ST8_REGXL,
	ST8_REGYH,
	ST8_REGYL,
	ST8_REGCC,
	ST8_EXP,		// mem
	ST8_EXP0,		// *mem
	ST8_INDIR,		// [mem]
	ST8_INDIR0,		// [*mem]
	ST8_INDEX_X,	// (X)
	ST8_INDEX_Y,	// (Y)
	ST8_INDIR_X,	// ([mem], X)
	ST8_INDIR_Y,	// ([mem], Y)
	ST8_INDIR0_X,	// ([*mem], X)
	ST8_INDIR0_Y,	// ([*mem], Y)
	ST8_OFFSET_X,	// (n, X)
	ST8_OFFSET_Y,	// (n, Y)
	ST8_OFFSET0_X,	// (n, X)
	ST8_OFFSET0_Y,	// (n, Y)
	ST8_OFFSET0_SP,	// (n, SP)
	ST8_IMMD,		// #exp
};

typedef struct {
	unsigned int symbl;			// instruction token
	unsigned int code_size;		// bytes of opcode
	unsigned int code;			// instruction code
	unsigned int inst_size;		// whole instruction size (bytes)
	unsigned int operands;		// operand types
} ST8_inst_t;

int  operCnt(const ST8_inst_t *);
int  pickOpr(const ST8_inst_t *, int i);
bool operandIsReg(const ST8_inst_t *p, int i);

extern const ST8_inst_t ST8_instCodeTbl[];

#endif
