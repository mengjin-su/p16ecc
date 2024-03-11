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
		case DATA_MEMORY: maxSize = useBSR6? 8192: 4096;	break;	// max size
		case CODE_MEMORY: maxSize = 1024*32;				break;	// max size
	}

	mem = new char[maxSize];
	size= maxSize;
	init();
}

Memory :: ~Memory()
{
	delete [] mem;
}

void Memory :: init(line_t *lp)
{
}

void Memory :: resize(int _size)
{
	if ( _size <= maxSize )
	{
		size = _size;
		init();
	}
}

void Memory :: init(void)
{
	if ( type == DATA_MEMORY )
		for (int a = 0; a < size; a++)
			mem[a] = GENERAL_MEMORY(a)? 0: -1;
	else
		memset(mem, 0, size);
}

void Memory :: blank(int start, int length)
{
	if ( type == CODE_MEMORY )
	{
		for (; length-- && start < size; start++)
			mem[start] = -1;
	}
	else if ( start >= 0x2000 )	// in linear space
	{
		for (; length-- && (start & 0x1fff) < size; start++)
			mem[TO_BANKED_ADDR(start)] = -1;
	}
	else
	{
		for (; length-- && start < size; start++)
			mem[start] = -1;
	}
}

int Memory :: getSpace(int init_addr, int req_size, int flag)
{
	for (int i = 0; i < req_size;)
	{
		int addr = init_addr + i;
		
		if ( type == DATA_MEMORY && addr >= 0x2000 ) addr = TO_BANKED_ADDR(addr);
		
		if ( (addr + req_size) > size ) 	return -1;	// out of space
		
		bool next_addr = false; 
		if ( mem[addr] )
		{
			if ( flag & FIXED_ADDR_FLAG )	return -1;	// can't fit in space
			next_addr = true;
		}

		if ( type == DATA_MEMORY && i && (addr & 0x7f) == 0x20 && !(flag & LINEAR_DATA_FLAG) )
			next_addr = true;	// memory across bank

		if ( type == CODE_MEMORY && i && (addr & 2047) == 0 && (flag & PAGE_CHECK_FLAG) )
			next_addr = true;	// memory across page
		
		i++;
		if ( next_addr )
		{
			i = 0;
			init_addr++;		// try next address
		}	
	}

	fillSpace(init_addr, req_size, 1);
	return init_addr;
}

void Memory :: fillSpace(int start, int length, char val)
{
	if ( type == DATA_MEMORY && start >= 0x2000 )
		start = TO_BANKED_ADDR(start);

	for (; length && start < size; start++)
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
	for (int a = 0; a < size; a++)
	{
		if ( (a & 0x03f) == 0 ) fprintf(fout, "%04X: ", a);

		if ( mem[a] == 0 )	   fprintf(fout, ".");
		else if ( mem[a] > 0 ) fprintf(fout, "+");
		else				   fprintf(fout, "X");

		if ( (a & 0x3f) == 0x3f ) fprintf(fout, "\n");
	}
	fprintf(fout, "\n");
}

void Memory :: reset(void)
{
	for (int i = 0; i < size; i++)
		if ( mem[i] > 0 ) mem[i] = 0;
}

int Memory :: memUsed(int *unused)
{
	int n = 0;
	*unused = 0;
	for (int i = 0; i < size; i++)
	{
		if ( mem[i] > 0 ) n++;
		if ( mem[i] == 0 ) (*unused)++;
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
