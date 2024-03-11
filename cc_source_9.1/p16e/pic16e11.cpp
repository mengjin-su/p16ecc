#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "../common.h"
extern "C" {
#include "../cc.h"
}
#include "../sizer.h"
#include "../pcoder.h"
#include "../display.h"
#include "pic16e.h"
#include "pic16e_asm.h"
#include "pic16e_reg.h"
#include "pic16e_inst.h"

void PIC16E :: divmodAssign(int code, Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	int size1 = ip1->acceSize();
	bool sign0 = ip0->acceSign();
	bool sign1 = ip1->acceSign();
	int size  = (size0 >= size1)? size0: size1;
	int op_flag = 0;

	char *func;
	switch ( size )
	{
		case 1:	func = (char*)"_divmod8";	break;
		case 2:	func = (char*)"_divmod16";	break;
		case 3:	func = (char*)"_divmod24";	break;
		case 4:	func = (char*)"_divmod32";	break;
		default: return;
	}

	if ( sign0 ) op_flag |= 4;				// signed = 1; unsigned = 0
	if ( sign1 ) op_flag |= 2;				// signed = 1; unsigned = 0
	if ( code != DIV_ASSIGN ) op_flag |= 1;	// division = 0; modulation = 1

	pushStack(ip1, size);
	Item *acc = storeToACC(ip0, size);
	ASM_CODE(_MOVLW, op_flag);
	call(func);
	acc->attr->isUnsigned = (sign0 == sign1)? 1: 0;
	mov(ip0, acc);
	delete acc;
}

void PIC16E :: divmod(int code, Item *ip0, Item *ip1, Item *ip2)
{
	int size1 = ip1->acceSize();
	int size2 = ip2->acceSize();
	bool sign1 = ip1->acceSign();
	bool sign2 = ip2->acceSign();
	int size  = (size1 >= size2)? size1: size2;
	int op_flag = 0;

	char *func;
	switch ( size )
	{
		case 1:	func = (char*)"_divmod8";	break;
		case 2:	func = (char*)"_divmod16";	break;
		case 3:	func = (char*)"_divmod24";	break;
		case 4:	func = (char*)"_divmod32";	break;
		default: return;
	}
	if ( sign1 ) op_flag |= 4;			// signed = 1; unsigned = 0
	if ( sign2 ) op_flag |= 2;			// signed = 1; unsigned = 0
	if ( code != '/' ) op_flag |= 1;	// division = 0; modulation = 1

	pushStack(ip2, size);
	Item *acc = storeToACC(ip1, size);
	ASM_CODE(_MOVLW, op_flag);
	call(func);
	acc->attr->isUnsigned = (sign1 == sign2)? 1: 0;
	mov(ip0, acc);
	delete acc;
}

void PIC16E :: pragma(Item *ip0, Item *ip1)
{
	if ( strcmp(ip0->val.s, "acc_save") == 0 )
	{
		if ( ip1 && ip1->type == CON_ITEM )
		{
			accSave = ip1->val.i;
			return;
		}
	}
	if ( strcmp(ip0->val.s, "isr_no_stack") == 0 )
	{
		isrStackSet = false;
		return;
	}
	if ( memcmp(ip0->val.s, "FUSE", 4) == 0 )
	{
		Nnode *nnp = nlist->search(ip0->val.s);

		if ( nnp && nnp->np[0] && nnp->np[0]->type == NODE_CON &&
			 ip1 && ip1->type == CON_ITEM 						)
		{
			char *s = STRBUF();
			ASM_OUTP("\n");
			sprintf(s, "FUSE (ABS, =%ld)", nnp->np[0]->con.value);
			ASM_CODE(_SEGMENT, s);
			ASM_CODE(_DW, ip1->val.i);
			return;
		}
	}
	errPrint("unknown or invalid '#pragma' statement!\n");
}

void PIC16E :: asmfunc(Item *ip0, Item *ip1, Item *ip2)
{
	asmCode *afp = (asmCode *)ip0->val.p;
	int name_len = strlen(afp->name);
	char *suffix = NULL;

	if ( strcmp(&afp->name[name_len-2], "_W") == 0 ) suffix = _W_;
	if ( strcmp(&afp->name[name_len-2], "_F") == 0 ) suffix = _F_;

	char *indf = setFSR0(ip1);
	if ( suffix )
		ASM_CODE(afp->inst, acceItem(ip1, 0, indf), suffix);
	else if ( ip2 )
		ASM_CODE(afp->inst, acceItem(ip1, 0, indf), ip2->val.i);
	else
		ASM_CODE(afp->inst, acceItem(ip1, 0, indf));
}

void PIC16E :: djnz(int code, Item *ip0, Item *ip1)
{
	char *indf0 = setFSR0(ip0);
	char *inst = (code == P_DJNZ)? _DECFSZ: _INCFSZ;
	regPCLATH->load(ip1->val.s);
	ASM_CODE(inst, acceItem(ip0, 0, indf0), _F_);
	ASM_CODE(_GOTO, ip1->val.s);
}