#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include "../item.h"
#include "pic16e_asm.h"
#include "pic16e_reg.h"
#include "pic16e_inst.h"
#include "pic16e_asm_opt.h"

int P16E_asmOPT :: optimize(void)
{
	int optimized = 0, n;
	while ( outputHead != outputTail )
	{
		if ( (n = case1()) ) { optimized += n; continue; }
		if ( (n = case2()) ) { optimized += n; continue; }
		if ( (n = case3()) ) { optimized += n; continue; }
		if ( (n = case4()) ) { optimized += n; continue; }
		if ( (n = case5()) ) { optimized += n; continue; }
		if ( (n = case6()) ) { optimized += n; continue; }
		if ( (n = case7()) ) { optimized += n; continue; }
		if ( (n = case8()) ) { optimized += n; continue; }
		if ( (n = case9()) ) { optimized += n; continue; }
		break;
	}

	return optimized;
}

int P16E_asmOPT :: case1(void)
{
	if ( bufferDepth() >= 2 )
	{
		int idx1, idx2;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx1);

		if ( isUpdate_WandZ(p0) && bufferDepth() >= 3 )
		{
			AsmLine *p2 = nextLine(idx1, &idx2);
			if ( p1->isInst(_MOVWF) && p2->isInst(_MOVF, _W_) &&
				 sameOperand(p1, p2) 							)
		 	{
				 removeLine(idx2);
				 return 1;
			}
		}

		if ( p0->isInst(_MOVWF) && p1->isInst(_MOVWF) &&
			 sameOperand(p0, p1) 						)
		{
			removeLine(idx1);
			return 1;
		}

		if ( p0->isInst(_MOVF, _W_) &&
		     p1->isInst(_MOVWF) && sameOperand(p0, p1) )
		{
			removeLine(idx1);
			return 1;
		}
	}
	return 0;
}

int P16E_asmOPT :: case2(void)
{
	if ( bufferDepth() >= 1 )
	{
		AsmLine *p0 = &outputBuffer[outputHead];

		if ( (p0->isInst(_BSEL) || p0->isInst(_PSEL)) &&
			 p0->opr1 == p0->opr2 					  )
		{
			removeLine(outputHead);
			return 1;
		}
	}
	return 0;
}

int P16E_asmOPT :: case3(void)
{
	if ( bufferDepth() >= 2 )
	{
		int idx, n, m;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx);

		if ( p0->isInst(_ADDFSR) && isINDF0(p0->opr1) &&
			 p1->isInst(_ADDFSR) && isINDF0(p1->opr1) &&
			 isConst(p0->opr2, &n) && isConst(p1->opr2, &m) )
		{
			if ( (n + m) >= -32 && (n + m) <= 31 )
			{
				int c = 1;
				char buff[16];
				sprintf(buff, "%d", n + m);
				p0->opr2 = buff;
				removeLine(idx);

				if ( (n + m) == 0 )
				{
					removeLine(outputHead);
					c++;
				}
				return c;
			}
		}

		if ( p0->isInst(_ADDFSR) && isINDF1(p0->opr1) &&
			 p1->isInst(_ADDFSR) && isINDF1(p1->opr1) &&
			 isConst(p0->opr2, &n) && isConst(p1->opr2, &m) )
		{
			if ( (n + m) >= -32 && (n + m) <= 31 )
			{
				int c = 1;
				char buff[16];
				sprintf(buff, "%d", n + m);
				p0->opr2 = buff;
				removeLine(idx);

				if ( (n + m) == 0 )
				{
					removeLine(outputHead);
					c++;
				}
				return c;
			}
		}

	}
	return 0;
}

int P16E_asmOPT :: case4(void)
{
	if ( bufferDepth() >= 4 )
	{
		int idx1, idx2, idx3;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx1);
					  nextLine(idx1, &idx2);
		AsmLine *p3 = nextLine(idx2, &idx3);

		if ( (p0->isInst(_BTFSS) || p0->isInst(_BTFSC)) &&
			 p1->isInst(_GOTO)	 						 )
		{
			std::string lbl = p1->opr1 + ":";
			if ( lbl == p3->inst )
			{
				p0->inst = p0->isInst(_BTFSS)? _BTFSC: _BTFSS;
				removeLine(idx1);
				return 1;
			}
		}
	}
	return 0;
}

int P16E_asmOPT :: case5(void)
{
	if ( bufferDepth() >= 3 )
	{
		int idx1, idx2, n;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx1);
		AsmLine *p2 = nextLine(idx1, &idx2);

		if ( (p0->isInst(_MOVLW) && isConst(p0->opr1, &n) && n == 0) ||
			 p0->isInst(_CLRW) 										 )
		{
			if ( p1->isInst(_MOVWF) && renewWREG(p2) )
			{
				p1->inst = _CLRF;
				removeLine(outputHead);
				return 1;
			}
		}
	}
	return 0;
}

int P16E_asmOPT :: case6(void)
{
	if ( bufferDepth() >= 8 )
	{
		int idx1, idx2, idx3, idx4, idx5, idx6, idx7;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx1, SKIP_BSEL);
		AsmLine *p2 = nextLine(idx1, &idx2, SKIP_BSEL);
		AsmLine *p3 = nextLine(idx2, &idx3, SKIP_BSEL);
		AsmLine *p4 = nextLine(idx3, &idx4, SKIP_BSEL);
		AsmLine *p5 = nextLine(idx4, &idx5, SKIP_BSEL);
		AsmLine *p6 = nextLine(idx5, &idx6, SKIP_BSEL);
		AsmLine *p7 = nextLine(idx6, &idx7, SKIP_BSEL);

		if ( p0->isInst(_MOVF, _W_) 	&&
		     p1->isInst(_ADDWF, _F_) 	&&
		     p2->isInst(_MOVF, _W_) 	&&
		     p3->isInst(_ADDWFC, _F_) 	&&
		     p4->isInst(_MOVF, _W_) 	&&
		     p5->isInst(_MOVWF) && p5->opr1 == FSR0L &&
   		     p6->isInst(_MOVF, _W_) 	&&
		     p7->isInst(_MOVWF) && p7->opr1 == FSR0H )
		{
			if ( isVarPair(p0->opr1, p2->opr1) &&
				 isTempPair(p1->opr1, p3->opr1) &&
				 sameOperand(p1, p4) && sameOperand(p3, p6) )
			{
				p1->opr2 = _W_;
				p3->opr2 = _W_;

				AsmLine t = *p5;
				*p5 = *p7;
				*p4 = *p3;
				*p3 = *p2;
				*p2 = t;
				removeLine(idx7);
				removeLine(idx6);
				return 2;
			}
		}

		if ( p0->isInst(_MOVLW ) 		&&
			 p1->isInst(_ADDWF, _F_) 	&&
			 p2->isInst(_MOVLW)  		&&
			 p3->isInst(_ADDWFC, _F_) 	&&
			 p4->isInst(_MOVF, _W_) 	&&
			 p5->isInst(_MOVWF)  		&&
			 p6->isInst(_MOVF, _W_) 	&&
			 p7->isInst(_MOVWF) 		)
		{
			if ( isAddrPair(p0->opr1, p2->opr1) &&
		 	  	 isTempPair(p1->opr1, p3->opr1) &&
			  	 sameOperand(p1, p4) && sameOperand(p3, p6) &&
			   	 p5->opr1 == FSR0L && p7->opr1 == FSR0H )
		 	{
				p1->opr2 = _W_;
				p3->opr2 = _W_;

				AsmLine t = *p5;
				*p5 = *p7;
				*p4 = *p3;
				*p3 = *p2;
				*p2 = t;
				removeLine(idx7);
				removeLine(idx6);
				return 2;
			}
		}

		if ( p0->isInst(_MOVLW)  		&&
			 p1->isInst(_ADDWF, _F_) 	&&
			 p2->isInst(_MOVLW)  		&&
			 p3->isInst(_ADDWFC, _F_)	&&
			 p4->isInst(_MOVF, _W_) 	&&
			 p5->isInst(_MOVWI) && p5->opr1 == "--INDF1" &&
			 p6->isInst(_MOVF, _W_) 	&&
			 p7->isInst(_MOVWI) && p7->opr1 == "--INDF1" )
		 {
			 if ( isAddrPair(p0->opr1, p2->opr1) &&
			 	  isTempPair(p1->opr1, p3->opr1) &&
			 	  sameOperand(p1, p4) && sameOperand(p3, p6) )
			{
				p1->opr2 = _W_;
				p3->opr2 = _W_;

				AsmLine t = *p5;
				*p5 = *p7;
				*p4 = *p3;
				*p3 = *p2;
				*p2 = t;
				removeLine(idx7);
				removeLine(idx6);
				return 2;
			}
		}
	}
	return 0;
}

int P16E_asmOPT :: case7(void)
{
	if ( bufferDepth() >= 10 )
	{
		int idx1, idx2, idx3, idx4, idx5, idx6, idx7, idx8, idx9;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx1, SKIP_BSEL);
		AsmLine *p2 = nextLine(idx1, &idx2, SKIP_BSEL);
		AsmLine *p3 = nextLine(idx2, &idx3, SKIP_BSEL);
		AsmLine *p4 = nextLine(idx3, &idx4, SKIP_BSEL);
		AsmLine *p5 = nextLine(idx4, &idx5, SKIP_BSEL);
		AsmLine *p6 = nextLine(idx5, &idx6, SKIP_BSEL);
		AsmLine *p7 = nextLine(idx6, &idx7, SKIP_BSEL);
		AsmLine *p8 = nextLine(idx7, &idx8, SKIP_BSEL);
		AsmLine *p9 = nextLine(idx8, &idx9, SKIP_BSEL);

		if ( p0->isInst(_MOVLW)  		&&
			 p1->isInst(_ADDWF, _W_)   	&&
			 p2->isInst(_MOVWF)  		&&
			 p3->isInst(_MOVLW)  		&&
			 p4->isInst(_ADDWFC, _W_)  	&&
			 p5->isInst(_MOVWF)  		&&
			 p6->isInst(_MOVF, _W_)   	&&
			 p7->isInst(_MOVWF) && p7->opr1 == FSR0L &&
			 p8->isInst(_MOVF, _W_)   	&&
			 p9->isInst(_MOVWF) && p9->opr1 == FSR0H	)
		{
			if ( isAddrPair(p0->opr1, p3->opr1) &&
				 isTempPair(p2->opr1, p5->opr1) &&
			 	 sameOperand(p2, p6) && sameOperand(p5, p8) )
			{
				*p2 = *p7;
				*p5 = *p9;
				removeLine(idx9);
				removeLine(idx8);
				removeLine(idx7);
				removeLine(idx6);
				return 4;
		  	}
		}
	}
	return 0;
}

int P16E_asmOPT :: case8(void)
{
	if ( bufferDepth() >= 2 )
	{
		int idx1, m;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx1, SKIP_BSEL);

		if ( p0->isInst(_ADDFSR) && isINDF0(p0->opr1) &&
			 isConst(p0->opr2, &m) && (m == 1 || m == -1) )
		{
			if ( p1->isInst(_MOVF, _W_) && isINDF0(p1->opr1) )
			{
				p1->inst = _MOVIW;
				p1->opr1 = (m > 0) ? "++INDF0" : "--INDF0";
				p1->opr2 = "";
				removeLine(outputHead);
				return 1;
			}
		}

		if ( p0->isInst(_ADDFSR) && isINDF1(p0->opr1) &&
			 isConst(p0->opr2, &m) && (m == 1 || m == -1) )
		{
			if ( p1->isInst(_MOVF, _W_) && isINDF1(p1->opr1) )
			{
				p1->inst = _MOVIW;
				p1->opr1 = (m > 0)? "++INDF1" : "--INDF1";
				p1->opr2 = "";
				removeLine(outputHead);
				return 1;
			}
		}

		if ( p0->isInst(_ADDFSR) && isINDF0(p0->opr1) &&
			 isConst(p0->opr2, &m) && (m == 1 || m == -1) )
		{
			if ( p1->isInst(_MOVWF) && isINDF0(p1->opr1) )
			{
				p1->inst = _MOVWI;
				p1->opr1 = (m > 0) ? "++INDF0" : "--INDF0";
				p1->opr2 = "";
				removeLine(outputHead);
				return 1;
			}
		}

		if ( p0->isInst(_ADDFSR) && isINDF1(p0->opr1) &&
			 isConst(p0->opr2, &m) && (m == 1 || m == -1) )
		{
			if ( p1->isInst(_MOVWF) && isINDF1(p1->opr1) )
			{
				p1->inst = _MOVWI;
				p1->opr1 = (m > 0) ? "++INDF1" : "--INDF1";
				p1->opr2 = "";
				removeLine(outputHead);
				return 1;
			}
		}
	}
	return 0;
}

#define IS_LITERAL_INST(inst)	(inst == _MOVLW || inst == _ADDLW || inst == _SUBLW || \
								 inst == _IORLW || inst == _ANDLW || inst == _XORLW)
#define IS_OPER_INST(inst)		(inst == _ADDWF || inst == _ANDWF || inst == _SUBWF || \
								 inst == _IORWF || inst == _XORWF || inst == _SWAPF || \
								 inst == _RRF   || inst == _RLF   || \
								 inst == _INCF  || inst == _DECF  || inst == _COMF  || \
								 inst == _ASRF 	|| inst == _LSLF  || inst == _LSRF  || \
								 inst == _MOVF  || inst == _ADDWFC|| inst == _SUBWFB )

int P16E_asmOPT :: case9(void)
{
	if ( bufferDepth() >= 2 )
	{
		int idx1;
		AsmLine *p0 = &outputBuffer[outputHead];
		AsmLine *p1 = nextLine(outputHead, &idx1, SKIP_BSEL);

		if ( p0 && p1 && p1->isInst(_MOVWF) && isWREG(p1->opr1) &&
			 (IS_LITERAL_INST(p0->inst) || (IS_OPER_INST(p0->inst) && p0->opr2 == "W")) )
		{
			removeLine(idx1);
			return 1;
		}
	}
	return 0;
}