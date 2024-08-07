#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include "../item.h"
#include "pic16e_reg.h"
#include "pic16e_asm.h"
#include "pic16e_inst.h"
#include "pic16e_asm_opt.h"

P16E_asmOPT :: P16E_asmOPT(FILE *fileout)
{
	fout = fileout;
	outputHead = 0;
	outputTail = 0;
	append_str = false;
}

void P16E_asmOPT :: flush(void)
{
	while ( outputHead != outputTail )
		flush(1);
}

void P16E_asmOPT :: flush(int flush_lines)
{
	while ( outputHead != outputTail && flush_lines > 0 )
	{
		int n = optimize();		// run optimization before flushing a line...
		if ( n > 0 )
		{
			flush_lines -= n;
			if ( flush_lines < 0 ) flush_lines = 0;
			continue;
		}

		flush_lines--;
		AsmLine *p = &outputBuffer[outputHead];
		outputHead = NEXT_INDEX(outputHead);

		if ( fout )
		{
			if ( p->insertTAB )
				fprintf(fout, "\t");

			fprintf(fout, "%s", p->inst.c_str());
			if ( p->opr1 != "" )
			{
				fprintf(fout, "\t%s", p->opr1.c_str());
				if ( p->opr2 != "" )
					fprintf(fout, ", %s", p->opr2.c_str());
			}

			if ( !(strchr(p->inst.c_str(), '\n') ||
				   strchr(p->opr1.c_str(), '\n') ||
				   strchr(p->opr2.c_str(), '\n')) )
				fprintf(fout, "\n");		// terminate the line.
		}
	}
}

bool P16E_asmOPT :: isConst(std::string str, int *n)
{
	const char *s = str.c_str();
	bool neg = false;
	if ( s[0] == '-' ) { neg = true; s++; }
	int l = strlen(s);
	if ( l && isdigit(*s) )
	{
		if ( s[0] == '0' && toupper(s[1]) == 'X' )
		{
			for (int i = 2; i < l; i++)
				if ( !isxdigit(s[i]) ) return false;

			sscanf(&s[2], "%x", n);
		}
		else
		{
			for (int i = 0; i < l; i++)
				if ( !isdigit(s[i]) ) return false;

			sscanf(s, "%d", n);
		}
		if ( neg ) *n = -(*n);
		return true;
	}
	return false;
}

bool P16E_asmOPT :: isINDF0(std::string str)
{
	if ( str == INDF0 )
		return true;

	int reg;
	return (isConst(str, &reg) && reg == aINDF0);
}

bool P16E_asmOPT :: isINDF1(std::string str)
{
	if ( str == INDF1 )
		return true;

	int reg;
	return (isConst(str, &reg) && reg == aINDF1);
}

bool P16E_asmOPT :: isWREG(std::string r)
{
	if ( r == WREG )
		return true;

	int reg;
	return (isConst(r, &reg) && reg == aWREG);
}

int P16E_asmOPT :: bufferDepth(void)
{
	int n = 0;
	for (int i = outputHead; i != outputTail; i = NEXT_INDEX(i))
	{
		AsmLine *p = &outputBuffer[i];
		if ( !(p->nullLine() || p->isInst(_BSEL)) ) n++;
	}
	return n;
}

bool P16E_asmOPT :: bufferFull(void)
{
	int length = (outputTail - outputHead) & (ASM_BUFFER_SIZE-1);
	bool full = (length == (ASM_BUFFER_SIZE-1));
	return full;
}

AsmLine *P16E_asmOPT :: nextLine(int idx, int *sel_idx, bool skip_bsel)
{
	idx = NEXT_INDEX(idx);
	while ( idx != outputTail )
	{
		AsmLine *p = &outputBuffer[idx];

		if ( !p->nullLine() )
		{
			if ( skip_bsel && p->isInst(_BSEL) )
			{
				idx = NEXT_INDEX(idx);
				continue;
			}
			*sel_idx = idx;
			return p;
		}
		idx = NEXT_INDEX(idx);
	}
	return NULL;
}

void P16E_asmOPT :: removeLine(int idx)
{
	while ( idx != outputTail )
	{
		int next_idx = NEXT_INDEX(idx);
		outputBuffer[idx] = outputBuffer[next_idx];

		if ( next_idx == outputTail	)
			outputTail = idx;
		else
			idx = next_idx;
	}
}

/////////////////////////////////////////////////////////////////////
bool P16E_asmOPT :: isTempVar(std::string str)
{
	const char *s = str.c_str();
	int l = strlen(s);

	for (int i = l; i--;)
		if ( !isdigit(s[i]) )
		{
			if ( s[i] == '$' && (i+1) < l )
				return true;

			return false;
		}

	return false;
}

bool P16E_asmOPT :: sameOperand(AsmLine *p1, AsmLine *p2)
{
	int n, m;
	std::string s1 = p1->opr1;
	std::string s2 = p2->opr1;
	if ( s1 == s2 ) return true;

	if ( isConst(s1, &n) && isConst(s2, &m) && ((n ^ m) & 0x7F) == 0 )
		return true;

	return false;
}

bool P16E_asmOPT :: isVarPair(std::string var_L, std::string var_H)
{
	std::string var = var_L + "+1";
	return (var == var_H);
}

bool P16E_asmOPT :: isTempPair(std::string tmp_L, std::string tmp_H)
{
	return (isTempVar(tmp_L) && isVarPair(tmp_L, tmp_H));
}

bool P16E_asmOPT :: isAddrPair(std::string addr_L, std::string addr_H)
{
	std::string addr = "(" + addr_L + ")>>8";
	if ( addr == addr_H )
		return true;

	addr = "((" + addr_L + "))>>8";
	return (addr == addr_H);
}

bool P16E_asmOPT :: isUpdate_WandZ(AsmLine *p)
{
	if ( p->isInst(_ANDLW) || p->isInst(_ADDLW) || p->isInst(_SUBLW) ||
		 p->isInst(_IORLW) || p->isInst(_XORLW) )
		return true;

	if ( (p->isInst(_ADDWF) || p->isInst(_SUBWF) || p->isInst(_ADDWFC) || p->isInst(_SUBWFB)||
	 	  p->isInst(_ANDWF) || p->isInst(_IORWF) || p->isInst(_XORWF)  || p->isInst(_ASRF)  ||
	 	  p->isInst(_LSRF)  || p->isInst(_LSLF)  || p->isInst(_MOVF)   || p->isInst(_MOVIW) ||
		  p->isInst(_INCF)  || p->isInst(_DECF)  || p->isInst(_COMF)) && p->opr2 == _W_ 	)
		return true;

	return false;
}

bool P16E_asmOPT :: renewWREG(AsmLine *p)
{
	return (p->isInst(_MOVLW)     || p->isInst(_MOVIW) 	   ||
			p->isInst(_INCF, _W_) || p->isInst(_DECF, _W_) ||
			p->isInst(_COMF, _W_) || p->isInst(_RETURN)    || p->isInst(_MOVF, _W_));
}