#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
extern "C" {
#include "../common.h"
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../const.h"
#include "../nlist.h"
#include "../prescan.h"
#include "stm8s.h"
#include "stm8s_reg.h"
#include "stm8s_inst.h"
#include "stm8s_asm.h"
#include "../option.h"

STM8S :: STM8S(char *out_file, Nlist *_nlist, Pcoder *_pcoder)
{
	memset(this, 0, sizeof(STM8S));

	if ( out_file ) fout = fopen(out_file, "w");
	if ( fout == NULL ) fout = stdout;
	st8asm = new STM8S_ASM(fout);
	nlist  = _nlist;
	pcoder = _pcoder;

	Reg_A = new STM8S_reg(REG_A, 8);
	Reg_X = new STM8S_reg(REG_X, 16);
	Reg_Y = new STM8S_reg(REG_Y, 16);

	accSave = 4;	// default saving amount = 4 bytes
}

STM8S :: ~STM8S()
{
	fclose(fout);

	delete st8asm;
	delete Reg_A;
	delete Reg_X;
	delete Reg_Y;
}

void STM8S :: run(void)
{
	time_t t = time (&t);	// current time
	char *buf = STRBUF();	// string buffer
	Nnode *nnp = NULL;
	int ram_size = 0;
	int stack_addr = 0;
	PreScan prescan(nlist);

    sprintf(buf, ";**********************************************************\n"
	             ";  STMicroelectronics STM8S C Compiler (ST8CC), %s\n"
				 ";  %s"
				 ";**********************************************************\n",
				 VERSION, ctime(&t));
	ASM_OUTP(buf);

	for (NameList *lp = sysIncludeList; lp; lp = lp->next)
	{
		strcpy(buf, lp->name);
		int len = strlen(buf);
		if ( buf[len-2] == '.' && toupper(buf[len-1]) == 'H' )
		{
			buf[len-2] = '\0';
			char *s = STRBUF();
			sprintf(s, "\"%s\"", buf);
			ASM_CODE1(_INVOKE, s);
		}
	}
	ASM_OUTP('\n');

	sprintf(buf, "\"stm8s\"");
	if ( nlist )	// search Name List...
	{
		// device RAM name
		nnp = nlist->search((char*)"__DEVICE", DEFINE);
		if ( nnp && nnp->np[0] && nnp->np[0]->type == NODE_STR )
			sprintf(buf, "\"%s\"", nnp->np[0]->str.str);

		// device RAM size
		nnp = nlist->search((char*)"__RAM_SIZE", DEFINE);
		if ( nnp && (nnp->np[0] = prescan.scan(nnp->np[0])) && nnp->np[0] && nnp->np[0]->type == NODE_CON )
		{
			ram_size = nnp->np[0]->con.value;
			sprintf(&buf[strlen(buf)], ", %d", ram_size);
			stackAddr = ram_size - 1;
		}

		// device FLASH size
		nnp = nlist->search((char*)"__FLASH_SIZE", DEFINE);
		if ( nnp && (nnp->np[0] = prescan.scan(nnp->np[0])) && nnp->np[0] && nnp->np[0]->type == NODE_CON )
		{
			int flash_size = nnp->np[0]->con.value;
			sprintf(&buf[strlen(buf)], ", %d", flash_size);
//			if ( flash_size > 32768 ) EXT_MODE = true;
		}

		nnp = nlist->search((char*)"__STACK_INIT_ADDR", DEFINE);
		if ( nnp && (nnp->np[0] = prescan.scan(nnp->np[0])) && nnp->np[0] && nnp->np[0]->type == NODE_CON )
			stackAddr = nnp->np[0]->con.value;
	}
	ASM_CODE1(_DEVICE, buf);
	ASM_OUTP('\n');

	if ( stack_addr > 0 )
	{
		sprintf(buf, "_$$::\t%s\t0x%X\t; stack init. value\n", _EQU, stack_addr);
		ASM_OUTP(buf);
	}

	// allocate memory for fixed address data
	if ( outputData(FIXED_DATA_ALLOC, dataLink) )
		ASM_OUTP('\n');

	// allocate memory for bank0 address data
	if ( outputData(PUBLIC_DATA0_ALLOC, dataLink) )
		ASM_OUTP('\n');

	// allocate memory for public data
	if ( outputData(PUBLIC_DATA_ALLOC, dataLink) )
		ASM_OUTP('\n');

	// generate main program ASM code
	outputASM0(pcoder->mainPcode);

	// generate init code
	outputInit(pcoder->initPcode);

	// generate constant code
	outputConst(pcoder->constPcode);

	// output constant strings
	outputString();

	ASM_OUTP('\n');
	st8asm->code0(_END);
}

void STM8S :: errPrint(const char *msg, char *id)
{
	printf("%s", msg);
	if ( id ) printf(" [%s]", id);
	if ( srcCode ) printf(" - %s", srcCode);
	printf ("\n");
	errors++;
}

void STM8S :: outputASM0(Pnode *plist)
{
	for(; plist; plist = plist->next)
	{
		curPnode = plist;
		int pcode = plist->type;
		Item *ip0 = plist->items[0];
		Item *ip1 = plist->items[1];
		Item *ip2 = plist->items[2];

		switch ( pcode )
		{
			case P_SRC_CODE:	// source code insertion
				if ( curFunc )
				{
					ASM_OUTP((char*)"; :: ");
					ASM_OUTP_LN(ip0->val.s);
					srcCode = ip0->val.s;
				}
				break;

			case LABEL:			// label
				ASM_LABL_LN(ip0->val.s, true);
				Reg_A->reset();
				Reg_Y->reset();
				break;

			case P_FUNC_BEG:	// function begin
				curFunc = (Fnode*)ip0->val.p;
				if ( curFunc->endLbl > 0 )	// it's func. definition
					funcBeg(plist);
				break;

			case P_FUNC_END:
				curFunc = (Fnode*)ip0->val.p;
				if ( curFunc->endLbl > 0 )	// it's func. definition
					funcEnd();
				curFunc = NULL;
				break;

			case GOTO:
				ASM_CODE1(__JP, ip0->val.s);
				break;

			case CALL:
				call(ip0->val.s);
				break;

			case '=':	case P_MOV:	case P_ARG_PASS:
				mov(ip0, ip1);
				break;

			case P_ARG_PASS1:
				delete pushStack(ip1, ip0->acceSize());
				break;

			case P_DEC_STACK:
				switch ( ip0->val.i ) {
					case 0:		break;
					case 1:		ASM_CODE1(_PUSH, REG_A);	break;
					case 2:		ASM_CODE1(_PUSHW, REG_X);	break;
					default:	st8asm->code2(_SUB, REG_SP, ip0->val.i, '#');
				}
				frameOffset += ip0->val.i;
				break;

			case P_INC_STACK:
				if ( ip0->val.i > 0 ) popStack(ip0->val.i);
				break;

			case P_INC:	case P_DEC:
				if ( pcode == P_INC )
					incValue(ip0, ip1->val.i);
				else
					incValue(ip0, -ip1->val.i);
				break;

			case NEG_OF:
				neg(ip0, ip1);
				break;

			case '~':
				compl1(ip0, ip1);
				break;

			case '!':
				notop(ip0, ip1);
				break;

			case ADD_ASSIGN: 	add(ip0, ip1);	break;
			case SUB_ASSIGN:	sub(ip0, ip1);	break;

			case AND_ASSIGN: case OR_ASSIGN: case XOR_ASSIGN:
				andorxor(pcode, ip0, ip1);
				break;

			case P_JZ:	case P_JNZ:	// variable test
				jzjnz(pcode, ip0, ip1);
				break;

			case P_JBZ:	case P_JBNZ:// bit test
				jbzjbnz(pcode, ip0, ip1, ip2);
				break;

			case P_CALL:
				pcall(ip0);
				break;

			case P_JZ_INC:	case P_JZ_DEC: 	case P_JNZ_INC:	case P_JNZ_DEC:
				jzjnz_incdec(pcode, ip0, ip1, ip2);
				break;

			case '+':
				add(ip0, ip1, ip2);
				break;

			case '-':
				sub(ip0, ip1, ip2);
				break;

			case AASM:
				st8asm->code0(ip0->val.s);
				break;

			case P_JEQ:	case P_JNE:
				jeqjne(pcode, ip0, ip1, ip2);
				break;

			case P_JLT: case P_JLE: case P_JGT: case P_JGE:
				cmpJump(pcode, ip0, ip1, ip2);
				break;

			case LEFT_ASSIGN:
				leftAssign(ip0, ip1);
				break;

			case RIGHT_ASSIGN:
				rightAssign(ip0, ip1, ip0->acceSign());
				break;

			case LEFT_OP:
				leftOpr(ip0, ip1, ip2);
				break;

			case RIGHT_OP:
				rightOpr(ip0, ip1, ip2);
				break;

			case MUL_ASSIGN:
				mulAssign(ip0, ip1);
				break;

			case DIV_ASSIGN:
			case MOD_ASSIGN:
				divmodAssign(pcode, ip0, ip1);
				break;

			case '*':
				mul(ip0, ip1, ip2);
				break;

			case '/':
			case '%':
				divmod(pcode, ip0, ip1, ip2);
				break;

			case SWITCH:	// fetch SWITCH argument
				casefetch(ip0);
				break;

			case CASE:
				casejump(ip0, ip1, ip2);
				break;

			case '&':
			case '|':
			case '^':
				andorxor(pcode, ip0, ip1, ip2);
				break;

			case PRAGMA:
				pragma(ip0, ip1);
				break;


			case P_DJNZ: // dec, jp on NZ
			case P_IJNZ: // inc, jp on Z
				djnz(pcode, ip0, ip1);
				break;
		}
	}
}

void STM8S :: call(char *f)
{
	if ( EXT_MODE )
		ASM_CODE1(_CALLF, f);
	else
		ASM_CODE1(_CALL, f);
}

int STM8S :: outputData(RAM_DATA type, Dlink *dlink)
{
	if ( dlink == NULL ) return 0;
	int count = 0;
	char *buf = STRBUF();

	for (Dnode *dnode = dlink->dlist; dnode; dnode = dnode->next)
	{
		attrib *attr = dnode->attr;
		int bank = attr->dataBank;
		int size = sizer.size(attr, TOTAL_SIZE);

		if ( attr->isExtern ) continue;

		if ( dnode->atAddr >= 0 && type == FIXED_DATA_ALLOC )
		{
			int addr = dnode->atAddr;
			if ( bank == CONST )
				sprintf(buf, "CODE (ABS, =%d)", addr);
			else
				sprintf(buf, "BANK (ABS, =%d)", addr);
			ASM_CODE1(_SEGMENT, buf);

			ASM_LABL(dnode->nameStr(), !attr->isStatic);
			sprintf(buf, ":\t%s\t%d\n", _RS, size);
			ASM_OUTP(buf);
			count++;
		}

		if ( dnode->atAddr < 0 && type == PUBLIC_DATA0_ALLOC &&
			 !attr->isExtern && bank == BANK0					)
		{
			ASM_CODE1(_SEGMENT, "BANK0 (REL)");
			ASM_LABL(dnode->nameStr(), !attr->isStatic);
			sprintf(buf, ":\t%s\t%d\n", _RS, size);
			ASM_OUTP(buf);
			count++;
		}

		if ( dnode->atAddr < 0 && type == PUBLIC_DATA_ALLOC &&
			 !(attr->isExtern || bank == BANK0 || bank == CONST) )
		{
			ASM_CODE1(_SEGMENT, "BANK (REL)");
			ASM_LABL(dnode->nameStr(), !attr->isStatic);
			sprintf(buf, ":\t%s\t%d\n", _RS, size);
			ASM_OUTP(buf);
			count++;
		}
	}
	return count;
}