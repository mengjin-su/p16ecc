#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include "pic16e_inst.h"
#include "pic16e_asm_opt.h"


void P16E_asmOPT :: output(char *s)
{
	if ( append_str && outputHead != outputTail )
	{
		int idx = LAST_INDEX(outputTail);

		if ( outputBuffer[idx].opr1 == "" )
			outputBuffer[idx].inst += s;
		else if ( outputBuffer[idx].opr2 == "" )
			outputBuffer[idx].opr1 += s;
		else
			outputBuffer[idx].opr2 += s;
	}
	else
	{
		if ( bufferFull() )	flush(1);

		AsmLine *p = &outputBuffer[outputTail];
		outputTail = NEXT_INDEX(outputTail);

		p->insertTAB = false;
		p->inst = s;
		p->opr1 = "";
		p->opr2 = "";
	}

	append_str = (strchr(s, '\n') == NULL);
}

void P16E_asmOPT :: label(char *lbl, bool c, char *opr)
{
	if ( bufferFull() )	flush(1);

	AsmLine *p = &outputBuffer[outputTail];
	outputTail = NEXT_INDEX(outputTail);

	p->insertTAB = false;
	p->inst = lbl;
	p->inst += ":";
	if ( c ) p->inst += ":";
	p->opr1 = opr? opr: "";
	p->opr2 = "";
	append_str = false;
}

void P16E_asmOPT :: code(char *inst)
{
	if ( bufferFull() )	flush(1);

	AsmLine *p = &outputBuffer[outputTail];
	outputTail = NEXT_INDEX(outputTail);

	p->insertTAB = true;
	p->inst = inst;
	p->opr1 = "";
	p->opr2 = "";
	AsmLine();
	append_str = false;
}

void P16E_asmOPT :: code(char *inst, char *opr1, char *opr2)
{
	if ( bufferFull() )	flush(1);

	AsmLine *p = &outputBuffer[outputTail];
	outputTail = NEXT_INDEX(outputTail);

	p->insertTAB = true;
	p->inst = inst;
	p->opr1 = opr1;
	p->opr2 = opr2? opr2: "";
	append_str = false;
}

void P16E_asmOPT :: code(char *inst, int opr1, char *opr2)
{
	if ( bufferFull() )	flush(1);

	AsmLine *p = &outputBuffer[outputTail];
	outputTail = NEXT_INDEX(outputTail);

	char buff[16];
	p->insertTAB = true;
	p->inst = inst;
	sprintf(buff, "%d", opr1);	p->opr1 = buff;
	p->opr2 = opr2? opr2: "";
	append_str = false;
}

void P16E_asmOPT :: code(char *inst, char *opr1, int opr2)
{
	if ( bufferFull() )	flush(1);

	AsmLine *p = &outputBuffer[outputTail];
	outputTail = NEXT_INDEX(outputTail);

	char buff[16];
	p->insertTAB = true;
	p->inst = inst;
	p->opr1 = opr1;
	sprintf(buff, "%d", opr2);	p->opr2 = buff;
	append_str = false;
}

void P16E_asmOPT :: code(char *inst, int opr1, int opr2)
{
	if ( bufferFull() )	flush(1);

	AsmLine *p = &outputBuffer[outputTail];
	outputTail = NEXT_INDEX(outputTail);

	char buff[16];
	p->insertTAB = true;
	p->inst = inst;
	sprintf(buff, "%d", opr1);	p->opr1 = buff;
	sprintf(buff, "%d", opr2);	p->opr2 = buff;
	append_str = false;
}