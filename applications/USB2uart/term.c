#include <p16f1455.h>
#include "GenericTypeDefs.h"
#include "usb_config.h"
#include "term.h"
#include "tool.h"
#include "spi.h"
#include "mcp2515.h"

_linear_ RegIOMsg regIOMsg[IO_QUEUE_LENGTH];
BYTE ioTail, ioHead;

#define TERM_BUF_LENGTH	48

static _linear_ unsigned char termBuffer[TERM_BUF_LENGTH];
static unsigned char termLength;

const unsigned char AsciiTable[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x80, 0x81, 0x82, 0x83,	0x84, 0x85,	0x86, 0x87,	0x88, 0x89, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x8a, 0x8b, 0x8c,	0x8d, 0x8e,	0x8f, 0x47,	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f
};

extern char RS232_Out_Data[];
extern char USB_Out_Buffer[];

static void TERM_parse(void);

/////////////////////////////////////////////////////////////////////
void TERM_init(void)
{
	termLength = 0;
	ioTail = 0,
	ioHead = 0;
}

/////////////////////////////////////////////////////////////////////
void TERM_input(char *input, unsigned char length)
{
	while ( length-- )
	{
		unsigned char c = *input;
		input++;
		FSR0 = AsciiTable + c;
		c = INDF0;

		if ( termLength < TERM_BUF_LENGTH )
		{
			FSR0  = termBuffer + termLength;
			INDF0 = c;
			termLength++;
		}

		if ( c == '\n' || c == '\r' )
		{
			TERM_parse();
			termLength = 0;
		}
	}
}

#define ADDR_FOUND_FLAG		(1 << 0)
#define DATA_START_FLAG		(1 << 1)
#define ADDR_FOUND			(flags & ADDR_FOUND_FLAG)
#define DATA_STARTED		(flags & DATA_START_FLAG)

// process incoming message from host
static void TERM_parse(void)
{
	unsigned char i = 0, a = 0, cmd, cnt = 0;
	char flags = 0;
	unsigned long addr;

	while ( i < termLength )
	{
		FSR0 = termBuffer + i;
		cmd  = INDF0;
		if ( !(cmd == ' ' || cmd == '\t') ) break;
		i++;
	}

	if ( i >= termLength )
		return;

	addr = 0;
	for (i++; i < termLength; i++)
	{
		unsigned char c;
		FSR0 = termBuffer + i;
		c    = INDF0;

		switch ( c )
		{
			case '\n':
			case '\r':
				if ( a && DATA_STARTED )
				{
					a = 0;
					cnt++;
				}
				break;

			case ' ':
			case '\t':
				if ( a )
				{
					a = 0;
					if ( ADDR_FOUND )
					{
						if ( DATA_STARTED )
							cnt++;
						else
							flags |= DATA_START_FLAG;
					}
				}
				break;

			default:
				if ( c & 0x80 )
				{
					c &= ~0x80;

					if ( !DATA_STARTED )
					{
						addr <<= 4;
						addr  |= c;
						flags |= ADDR_FOUND_FLAG;
					}
					else
					{
						if ( a == 0 )
							termBuffer[cnt] = c;
						else
						{
							termBuffer[cnt] <<= 4;
							termBuffer[cnt]  |= c;
						}
					}
					a++;
					break;
				}
				else
					return;
		}
	}

	switch ( cmd )
	{
		case 'S':
		case 'X':	// transmit a message
			if ( cnt > 8 ) cnt = 8;

			a = (canTxTail + 1) & (CAN_TX_QUEUE_LENGTH - 1);
			if ( a != canTxHead && ADDR_FOUND )
			{
				CanTxMsg *mp = canTxQueue + TX_BUF_OFFSET(canTxTail);
				char *p = mp->data.tx.buff;
				mp->data.tx.id = addr;
				mp->data.tx.dlc= cnt;
				mp->type = cmd;
				for (i = 0; i < cnt; i++) *p++ = termBuffer[i];
				canTxTail = a;
			}
			break;

		case '#':	// poke a register
			if ( cnt == 0 )
				return;
		case '@':	// peek a register
		case 'L':	// '\r' attached in line
		case 'M':	// timestamp mode - v1.06+
			if ( !ADDR_FOUND )
				return;
		case '!':	// reset MCP2515
		case 'V':	// version request
		case '?':	// request tx buffer remainning
			a = (ioTail + 1) & (IO_QUEUE_LENGTH - 1);
			if ( a != ioHead )
			{
				switch ( cmd )
				{
					case '#':
						MCP2515_poke(addr, termBuffer[0]);
						cmd = '@';
						break;
					case 'L':
						AppFlags &= ~LF_FLAG;
						if ( addr )	AppFlags |= LF_FLAG;
						break;
					case 'M':
						AppFlags &= ~TSTAMP_FLAG;
						if ( addr )	AppFlags |= TSTAMP_FLAG;
						break;
				}

				RegIOMsg *mp = &regIOMsg[ioTail];
				mp->type = cmd;
				mp->addr = addr;
				ioTail = a;
			}
			break;

		case 'N':
			if ( cnt >= 2 )
				MCP2515_config(addr, termBuffer[0], termBuffer[1]);
			break;
	}
}
