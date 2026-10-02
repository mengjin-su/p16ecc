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
#include "stm8s.h"
#include "stm8s_reg.h"
#include "stm8s_inst.h"
#include "stm8s_asm.h"

void STM8S :: moveToBF(Item *ip0, Item *ip1)
{
	if ( ip1->type == CON_ITEM )
		moveToBF(ip0, ip1->val.i);
	else
	{
		int size0 = BF_SIZE(ip0->attr);
		int shift = BF_OFFSET(ip0->attr);
		uint8_t mask  = ((1 << size0) - 1) << shift;
		
		if ( IS_BF_VAR(ip1->attr) )
		{
			getBF(ip1, shift, size0);
			ASM_CODE2(_LD, 0, REG_A);
		}
		else
		{
			storeToACC(ip1, 1);
			if ( mask != 0xff )
			{
				while ( shift-- ) ASM_CODE1(_SRL, REG_A);
				ASM_CODE2(_AND, REG_A, mask, 1);
				ASM_CODE2(_LD, 0, REG_A);
			}
		}
			
		char *reg = setPtr(ip0, 0);
		ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg));
		ASM_CODE2(_AND, REG_A, ~mask, 1);
		ASM_CODE2(_OR, REG_A, 0);
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg), REG_A);
	}
}

void STM8S :: moveToBF(Item *ip0, int n)
{
	int size0 = BF_SIZE(ip0->attr);
	int shift = BF_OFFSET(ip0->attr);
	char *reg0= setPtr(ip0, 0);
	uint8_t mask = ((1 << size0) - 1) << shift;
	n <<= shift;
	n  &= mask;

	if ( size0 == 1 && reg0 == NULL )
	{
		char *inst = n? _BSET: _BRES;
		ASM_CODE2(inst, acceItem(ip0, 0, 1), shift, '#');
	}
	else if ( size0 == 8 )
	{
		if ( n == 0 )
			ASM_CODE1(_CLR, acceItem(ip0, 0, 1, reg0));
		else
		{
			ASM_CODE2(_LD, REG_A, n, '#');
			ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
		}
	}
	else
	{
		ASM_CODE2(_LD, REG_A, acceItem(ip0, 0, 1, reg0));
		if ( n == mask )
			ASM_CODE2(_OR, REG_A, n, '#');
		else
		{
			ASM_CODE2(_AND, REG_A, ~mask & 0xff, '#');
			if ( n ) ASM_CODE2(_OR, REG_A, n, '#');
		}
		ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg0), REG_A);
	}
}

void STM8S :: getBF(Item *ip, int left_shift, int req_size)
{
	if ( IS_BF_VAR(ip->attr) )	// it's from a bit-field type data
	{
		char *reg  = setPtr(ip, 0);
		int offset = BF_OFFSET(ip->attr);
		int size   = BF_SIZE(ip->attr);
		int shift  = offset - left_shift;
		uint8_t mask = (req_size < size)? ((1 << req_size) - 1) << offset:
										  ((1 <<     size) - 1) << offset;
		if ( shift > 0 ) mask >>= shift;
		else		     mask <<= abs(shift);		
		
		ASM_CODE2(_LD, REG_A, acceItem(ip, 0, 1, reg));
		if ( shift >= 4 || shift <= -4 )
		{
			ASM_CODE1(_SWAP, REG_A);
			shift = (shift > 0)? shift - 4: shift + 4;
		}
		while ( shift ) {
			if ( shift > 0 ) 
				ASM_CODE1(_SRL, REG_A), shift--;
			else
				ASM_CODE1(_SLL, REG_A), shift++;
		}
		
		if ( mask != 0xff )
			ASM_CODE2(_AND, REG_A, mask, 1);
	}	
}

void STM8S :: moveFromBF(Item *ip0, Item *ip1)
{
	getBF(ip1);
	int size0 = ip0->acceSize();
	char *reg = NULL;
	if ( usePtr(ip0) )
	{
		ASM_CODE2(_LD, 0, REG_A);
		reg = setPtr(ip0, 0);
		ASM_CODE2(_LD, REG_A, 0);
	}
	
	ASM_CODE2(_LD, acceItem(ip0, 0, 1, reg), REG_A);
	for (int i = 1; i < size0; i++)
		ASM_CODE1(_CLR, acceItem(ip0, i, 1, reg));
}