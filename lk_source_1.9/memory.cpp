#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
}
#include "memory.h"

Memory :: Memory(MEMORY_TYPE _type)
{
	switch ( type = _type )
	{
		case DATA_MEMORY: maxSize = (useBSR6? 64:32)*80+16;	break;	// max RAM size
		case CODE_MEMORY: maxSize = 1024*32;				break;	// max ROM size
	}

	memSize = maxSize;
	mem = new char[maxAddr()];
	init();
}

Memory :: ~Memory()
{
	delete [] mem;
}

void Memory :: resize(int _size)
{
	if ( _size <= maxSize )
	{
		memSize = _size;
		init();
	}
}

void Memory :: init(void)
{
	if ( type == DATA_MEMORY )
		for (int a = 0; a < maxAddr(); a++)
			mem[a] = GENERAL_MEMORY(a)? 0: -1;
	else
		memset(mem, 0, memSize);
}

void Memory :: blank(int start, int length)
{
	if ( type == CODE_MEMORY )
	{
		for (; length-- && start < memSize; start++)
			mem[start] = -1;
	}
	else if ( start >= 0x2000 )	// in linear space
	{
		for (; length-- && (start & 0x1fff) < memSize-16; start++)
			mem[TO_BANKED_ADDR(start)] = -1;
	}
	else
	{
		for (; length-- && start < maxAddr(); start++)
			mem[start] = -1;
	}
}

bool Memory :: getSpace(int init_addr, int *act_addr, int req_size, int flag)
{
	int org_addr = init_addr;

	if ( type == DATA_MEMORY )	// RAM momery
	{
		if ( init_addr >= 0x2000 ) init_addr = TO_BANKED_ADDR(init_addr);
	}
	else						// ROM momery
		init_addr &= 0x7fff;

	if ( !(type == DATA_MEMORY && (flag & FIXED_ADDR_FLAG)) )
	{
		int addr = init_addr;
		for (int reqsize = req_size; reqsize > 0;)
		{
			bool restart = false;

			if ( addr >= maxAddr() )
				return false;	// out of space

			if ( mem[addr] )
			{
				if ( flag & FIXED_ADDR_FLAG ) return false;	// can't fit in space
				restart = true;
			}
			else if ( reqsize < req_size )
			{
				if ( type == DATA_MEMORY && (addr & 0x7f) == 32 && !(flag & LINEAR_DATA_FLAG) )
					restart = true;	// memory across bank

				if ( type == CODE_MEMORY && (addr & 2047) == 0 && (flag & PAGE_CHECK_FLAG) )
					restart = true;	// memory across page
			}

			reqsize--;
			addr++;
			if ( restart )
			{
				reqsize = req_size;	// re-start over
				addr = ++init_addr;	// try next address
			}
			else if ( type == DATA_MEMORY && (addr & 0x7f) >= 0x70 )
			{
				// go to next bank general space
				addr = ((addr + 128) & ~0x7f) | 0x20;
			}
		}
	}

	fillSpace(init_addr, req_size, 1);
	if ( type == DATA_MEMORY )	// RAM memory
		*act_addr = ((flag & FIXED_ADDR_FLAG) &&
					 org_addr < 0x2000 && !GENERAL_MEMORY(org_addr) )? org_addr:
					 												   TO_LINEAR_ADDR(init_addr);
	else						// ROM memory
		*act_addr = init_addr | 0x8000;

	return true;
}

void Memory :: fillSpace(int start, int length, char val)
{
	if ( type == DATA_MEMORY && start >= 0x2000 )
		start = TO_BANKED_ADDR(start);

	for (; length && start < maxAddr(); start++)
	{
		if ( mem[start] >= 0 )
		{
			mem[start] = val;
			length--;
		}
	}
}

void Memory :: printSpace(FILE *fout)
{
	if ( fout == NULL ) fout = stdout;

	fprintf(fout, "\n- MEMORY MAP   'X'=forbidden, '+'=used, '.'=unused\n");
	for (int a = 0; a < maxAddr(); a++)
	{
		if ( (a & 0x3f) == 0x00 ) fprintf(fout, "%04X: ", a);

		if ( mem[a] == 0 )	   fprintf(fout, ".");
		else if ( mem[a] > 0 ) fprintf(fout, "+");
		else				   fprintf(fout, "X");

		if ( (a & 0x3f) == 0x3f ) fprintf(fout, "\n");
	}
	fprintf(fout, "\n");
}

void Memory :: reset(void)
{
	for (int i = 0; i < maxAddr(); i++)
		if ( mem[i] > 0 ) mem[i] = 0;
}

int Memory :: memUsed(int *total)
{
	int n = 0;
	*total = 0;
	for (int i = 0; i < maxAddr(); i++)
	{
		if ( mem[i] > 0 ) n++;
		if ( mem[i] >= 0 ) (*total)++;
	}

	return n;
}

int Memory :: maxAddr(void)
{
	int n = memSize;
	if ( type == DATA_MEMORY )
	{
		n = memSize - 16;
		n = (n%80)? (n/80)*128 + (n%80) + 32: (n/80)*128;
	}
	return n;
}

void Memory :: display(int addr)
{
	for (int i = 0; i < addr; i++)
	{
		if ( (i & 0x0f) == 0 ) printf("\n%03x: ", i);
		printf("%c ", mem[i]+'0');
	}
	printf("\n");
}
