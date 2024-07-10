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

void PIC16E :: mulAssign(Item *ip0, Item *ip1)
{
	int size0 = ip0->acceSize();
	if ( size0 == 1 )
	{
		mulAssign8(ip0, ip1);
		return;
	}
    char *func = (size0 == 2)? (char*)"_mul16indf":
    			 (size0 == 3)? (char*)"_mul24indf":
    						   (char*)"_mul32indf";
	pushStack(ip1, size0);
	setFSR(ip0, 0);
	call(func);
}

void PIC16E :: mulAssign8(Item *ip0, Item *ip1)
{
    if ( ip0->type == ACC_ITEM )
    {
		fetch(ip1, 0, setFSR0(ip1));
		call((char*)"_mul8");
		return;
	}
	Item *acc = NULL;
	if ( CONST_ITEM(ip1) || CONST_ID_ITEM(ip1) )
	{
		acc = storeToACC(ip0, 1);
		fetch(ip1, 0, NULL);
	}
	else
	{
		acc = storeToACC(ip1, 1);
		fetch(ip0, 0, setFSR0(ip0));
	}

	call((char*)"_mul8");
	ASM_CODE(_MOVF, ACC0, _W_);

	if ( useFSR(ip0) )
		ASM_CODE(_MOVWF, INDF0);
	else
		store(ip0, 0, NULL);

	if ( acc ) delete acc;
}

void PIC16E :: mul(Item *ip0, Item *ip1, Item *ip2)
{
	int size0 = ip0->acceSize();
	if ( same(ip0, ip2) )
	{
		mulAssign(ip0, ip1);
		return;
	}
	if ( size0 == 1 || (ip1->acceSize() == 1 && !ip1->acceSign() &&
						ip2->acceSize() == 1 && !ip2->acceSign() ) )
	{
		mul8(ip0, ip1, ip2);
		return;
	}
	if ( ip0->type == ACC_ITEM || ip1->type == ACC_ITEM || ip2->type == ACC_ITEM )
	{
		pushStack((ip2->type == ACC_ITEM)? ip1: ip2, size0);
		Item *acc  = storeToACC((ip2->type == ACC_ITEM)? ip2: ip1, size0);
		char *func = (size0 == 2)? (char*)"_mul16":
					 (size0 == 3)? (char*)"_mul24":
								   (char*)"_mul32";
		call(func);
		mov(ip0, acc);
		delete acc;
		return;
	}

	if ( !related(ip0, ip2) )
	{
		mov(ip0, ip1);
		mulAssign(ip0, ip2);
	}
	else
	{
		mov(ip0, ip2);
		mulAssign(ip0, ip1);
	}
}

void PIC16E :: mul8(Item *ip0, Item *ip1, Item *ip2)
{
	Item *acc = NULL;
	if ( CONST_ITEM(ip1) || CONST_ID_ITEM(ip1) )
	{
		if ( ip2->type != ACC_ITEM )
			acc = storeToACC(ip2, 1);
		fetch(ip1, 0, NULL);
	}
	else if ( CONST_ITEM(ip2) || CONST_ID_ITEM(ip2) )
	{
		if ( ip1->type != ACC_ITEM )
			acc = storeToACC(ip1, 1);
		fetch(ip2, 0, NULL);
	}
	else if ( ip1->type == ACC_ITEM )
	{
		fetch(ip2, 0, setFSR0(ip2));
	}
	else if ( ip2->type == ACC_ITEM )
	{
		fetch(ip1, 0, setFSR0(ip1));
	}
	else
	{
		acc = storeToACC(ip1, 1);
		fetch(ip2, 0, setFSR0(ip2));
	}

	call((char*)"_mul8");

	if ( acc == NULL )
		acc = accItem(newAttr(INT));
	else
		acc->attr->type = INT;

	acc->attr->isUnsigned = 1;
	mov(ip0, acc);
	delete acc;
}

// push an item into stack with desired size
void PIC16E :: pushStack(Item *ip0, int size)
{
	int size0 = ip0->acceSize();
	char *indf0 = setFSR0(ip0);
	for(int i = 0; i < size; i++)
	{
		if ( i < size0 || CONST_ITEM(ip0) )
			fetch(ip0, i, indf0);
		else if ( ip0->acceSign() )
		{
			if ( i == size0 )
				EXTEND_WREG;
		}
		else
			regWREG->load(0);
		ASM_CODE(_MOVWI, "--INDF1");
	}
}