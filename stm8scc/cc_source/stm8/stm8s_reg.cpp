#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../item.h"
#include "stm8s_reg.h"

/////////////////////////////////////////////////////
STM8S_reg :: STM8S_reg(const char *name, int size)
{
	memset(this, 0, sizeof(STM8S_reg));
	regName = name;
	regSize = size;
	trace  = NULL;
}

/////////////////////////////////////////////////////
STM8S_reg :: ~STM8S_reg()
{
	reset();
}

/////////////////////////////////////////////////////
void STM8S_reg :: reset(void)
{
	while ( trace )
	{
		Item *p = trace->next;
		delete trace;
		trace = p;
	}
	loaded = 0;
	numLoad= 0;
}
