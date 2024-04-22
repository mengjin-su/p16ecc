#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fwriter_hex.h"

HexWriter :: HexWriter(char *filename)
{
	memset(this, 0, sizeof(HexWriter));
	fout = fopen(filename, "w");
	if ( fout == NULL )	fout = stdout;
}

void HexWriter :: close(void)
{
	if ( fout )
	{
		outputLine();	// dump the remaining
		fprintf(fout, ":00000001FF\n");
		fclose(fout);
	}
}

void HexWriter :: outputWord(int addr, int word)
{
	if ( addr != currentAddr || length >= 32 )
	{
		outputLine();
		length = 0;
	}

	if ( length == 0 )	// beginning of a hex line
	{
		if ( (currentAddr >> 16) != (addr >> 16) )
			outputBank(addr >> 16);

		startAddr = currentAddr = addr;
	}

	buf[length++] = word & 0xff;// lower byte
	buf[length++] = word >> 8;	// higher byte
	currentAddr  += 2;
}

void HexWriter :: outputLine(void)
{
	if ( length > 0 )
	{
		unsigned char chksum = (startAddr >> 8) + startAddr + length;
		fprintf(fout, ":%02X%04X00", length, startAddr & 0xffff);

		for (int i = 0; i < length; i++)
		{
			fprintf(fout, "%02X", buf[i]);
			chksum += buf[i];
		}
		fprintf(fout, "%02X\n", (-chksum)&0xff);
	}
}

void HexWriter :: outputBank(int bank)
{
	unsigned char chksum = 2 + 4 + (bank >> 8) + bank;
	fprintf(fout, ":02000004%04X%02X\n", bank, (-chksum)&0xff);
}