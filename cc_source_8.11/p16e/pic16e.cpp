#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "../common.h"
extern "C" {
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "../const.h"
//#include "../option.h"
#include "pic16e.h"
#include "pic16e_reg.h"
#include "pic16e_inst.h"
#include "pic16e_asm.h"

P16E_REG8 	*regWREG;
P16E_REG8 	*regBSR;
P16E_REG8 	*regPCLATH;
P16E_FSR0	*regFSR0;

PIC16E :: PIC16E(char *out_file, Nlist *_nlist, Pcoder *_pcoder)
{
	memset(this, 0, sizeof(PIC16E));	// clean up the class data

	if( out_file ) fout = fopen(out_file, "w");
	if ( fout == NULL )	fout = stdout;
	asm16e = new P16E_ASM(fout);
	nlist  = _nlist;
	pcoder = _pcoder;

	regWREG   = new P16E_REG8(WREG,   asm16e);
	regBSR 	  = new P16E_REG8(BSR,    asm16e);
	regPCLATH = new P16E_REG8(PCLATH, asm16e);
	regFSR0   = new P16E_FSR0(asm16e);
	accSave   = 4;
	isrStackSet = true;
}

PIC16E :: ~PIC16E()
{
	fclose(fout);
	delete asm16e;
	delete regWREG;
	delete regBSR;
	delete regPCLATH;
	delete regFSR0;
}

////////////////////////////////////////////////////////////////////////////////////////
void PIC16E :: run(void)
{
	time_t t = time (&t);	// current time
	char *buf = STRBUF();	// string buffer
	Nnode *nnp = NULL;
	int ram_size = 0;
	int stack_addr = 0;

    sprintf(buf, ";**********************************************************\n"
	             ";  Microchip Enhanced PIC16F1xxx C Compiler (CC16E), %s\n"
				 ";  %s"
				 ";**********************************************************\n",
				 VERSION, ctime (&t));
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
			ASM_CODE(_INVOKE, s);
		}
	}
	delName(&sysIncludeList); ASM_OUTP("\n");

	sprintf(buf, "\"pic16e\"");
	if ( nlist )	// search Name List...
	{
		// device RAM name
		nnp = nlist->search((char*)"__DEVICE", DEFINE);
		if ( nnp && nnp->np[0] && nnp->np[0]->type == NODE_STR )
			sprintf(buf, "\"%s\"", nnp->np[0]->str.str);

		// device RAM size
		nnp = nlist->search((char*)"__SRAM_SIZE", DEFINE);
		if ( nnp && nnp->np[0] && nnp->np[0]->type == NODE_CON )
		{
			ram_size = nnp->np[0]->con.value;
			sprintf(&buf[strlen(buf)], ", %d", ram_size);
			stack_addr = 0x2000 + ram_size - 16;
		}

		// device FLASH size
		nnp = nlist->search((char*)"__FLASH_SIZE", DEFINE);
		if ( nnp && nnp->np[0] && nnp->np[0]->type == NODE_CON )
		{
			int flash_size = nnp->np[0]->con.value;
			sprintf(&buf[strlen(buf)], ", %d", flash_size);
		}

		nnp = nlist->search((char*)"__STACK_INIT_ADDR", DEFINE);
		if ( nnp && nnp->np[0] && nnp->np[0]->type == NODE_CON )
			stack_addr = nnp->np[0]->con.value;
	}
	ASM_CODE(_DEVICE, buf);
	ASM_OUTP("\n");

	if ( stack_addr > 0 )
	{
		sprintf(buf, "%s\t0x%X\t; stack init. value\n", _RS, stack_addr);
		ASM_LABL((char*)"_$$", true, buf);
	}

	// allocate memory for fixed address & public data
	if ( outputData(dataLink) )	ASM_OUTP("\n");

	// generate main program ASM code
	outputASM0(pcoder->mainPcode);
	ASM_OUTP("\n");
	ASM_CODE(_END);
}

void PIC16E :: errPrint(const char *msg)
{
	if ( srcCode )
		printf("%s - %s\n", msg, srcCode);
	else
		printf ("%s\n", msg);
	errors++;
}

int PIC16E :: outputData(Dlink *dlink)
{
	if ( dlink == NULL ) return 0;
	int count = 0;
	char *buf = STRBUF();

	for (Dnode *dnode = dlink->dlist; dnode; dnode = dnode->next)
	{
		attrib *attr = dnode->attr;
		if ( attr == NULL || attr->isExtern ) continue;

		int bank = attr->dataBank;
		if ( bank == CONST || bank == EEPROM ) continue;

		if ( dnode->atAddr >= 0 )	// allocate fixed address variables
		{
			int addr = dnode->atAddr;

			// linear address conversion...
			if ( bank == LINEAR && addr < 0x2000 && (addr & 0x7f) >= 0x20 )
				addr = ((addr >> 7)*80 + ((addr&0x7f) - 0x20)) | 0x2000;

			sprintf(buf, "BANK (ABS, =%d)", addr);
			ASM_CODE(_SEGMENT, buf);
		}
		else						// allocate public variables
		{
			if ( bank == LINEAR )
				ASM_CODE(_SEGMENT, "BANKn (REL)");
			else
				ASM_CODE(_SEGMENT, "BANKi (REL)");
		}

		sprintf(buf, "%s\t%d", _RS, sizer(attr, TOTAL_SIZE));
		ASM_LABL(dnode->nameStr(), !attr->isStatic, buf);
		count++;
	}
	return count;
}

void PIC16E :: outputASM0(Pnode *plist)
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
			case P_FUNC_BEG:	// function begin
				curFunc = (Fnode*)ip0->val.p;
				if ( curFunc->endLbl > 0 )	// it's func. definition
					funcBeg(plist);
				else
					curFunc = NULL;
				break;

			case P_FUNC_END:
				curFunc = (Fnode*)ip0->val.p;
				if ( curFunc->endLbl > 0 )	// it's func. definition
					funcEnd();
				curFunc = NULL;
				break;

			case P_SRC_CODE:	// source code insertion
				if ( curFunc )
				{
					srcCode = ip0->val.s;
					ASM_OUTP("; :: "); ASM_OUTP(srcCode); ASM_OUTP("\n");
				}
				break;

			case LABEL:			// label
				if ( curFunc )
				{
					regPCLATH->load(ip0->val.s);
					regBSR->reset();
					regWREG->reset();
					regFSR0->reset();
				}
				ASM_LABL(ip0->val.s);
				break;

			case CALL:
				call(ip0->val.s);
				regBSR->reset();
				regWREG->reset();
				regFSR0->reset();
				break;

			case GOTO:
				regPCLATH->load(ip0->val.s);
				asm16e->code(_GOTO, ip0->val.s);
				break;

			case '=':	case P_MOV:
				mov(ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case INC_OP:	case DEC_OP:
				incValue(ip0, (pcode == INC_OP)? ip1->val.i: -ip1->val.i);
				regFSR0->reset(ip0);
				break;

			case NEG_OF:
				neg(ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case '~':
				compl1(ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case '!':
				notop(ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case ADD_ASSIGN:
			case SUB_ASSIGN:
				addsub(pcode, ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case AND_ASSIGN:
			case OR_ASSIGN:
			case XOR_ASSIGN:
				andorxor(pcode, ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case P_JZ:	case P_JNZ:	// variable test
				jzjnz(pcode, ip0, ip1);
				break;

			case P_JBZ:	case P_JBNZ:// bit test
				jbzjbnz(pcode, ip0, ip1, ip2);
				break;

			case P_ARG_PASS:
				passarg(ip0, ip1, ip2);
				break;

			case P_CALL:
				pcall(ip0);
				regPCLATH->reset();
				regWREG->reset();
				regBSR->reset();
				regFSR0->reset();
				break;

			case P_JZ_INC:	case P_JZ_DEC: 	case P_JNZ_INC:	case P_JNZ_DEC:
				jzjnz_incdec(pcode, ip0, ip1, ip2);
				break;

			case '+':
				add(ip0, ip1, ip2);
				regFSR0->reset(ip0);
				break;
			case '-':
				sub(ip0, ip1, ip2);
				regFSR0->reset(ip0);
				break;

			case P_JEQ:	case P_JNE:
				jeqjne(pcode, ip0, ip1, ip2);
				break;

			case P_JLT: case P_JLE:	case P_JGT:	case P_JGE:
				cmpJump(pcode, ip0, ip1, ip2);
				break;

			case LEFT_ASSIGN:
				leftAssign(ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case RIGHT_ASSIGN:
				rightAssign(ip0, ip1, ip0->acceSign());
				regFSR0->reset(ip0);
				break;

			case LEFT_OP:
				leftOpr(ip0, ip1, ip2);
				regFSR0->reset(ip0);
				break;
			case RIGHT_OP:
				rightOpr(ip0, ip1, ip2);
				regFSR0->reset(ip0);
				break;

			case MUL_ASSIGN:
				mulAssign(ip0, ip1);
				regFSR0->reset(ip0);
				break;

			case '*':
				mul(ip0, ip1, ip2);
				regFSR0->reset(ip0);
				break;
		}
	}
}
