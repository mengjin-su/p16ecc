#include <p16f688.h>
#include "define.h"
#include "mmcspi.h"
#include "spi.h"
#include "fat.h"

/* --- MMC response --- */
#define GOOD_R1_RESPONSE 0x00
#define MMC_MAX_RETRY   10

//static unsigned int MMC_StatusFlags = 0;

/* --- response token bits --- */
#define PAR_ERR         0x4000   // parameter error
#define ADDR_ERR        0x2000   // address error
#define ERASE_SEQ_ERR   0x1000   // erase seq error
#define COMM_CRC_ERR    0x0800   // comm CRC error
#define ILLEGAL_CMD     0x0400   // illegal command
#define ERASE_RESET     0x0200   // erase reset
#define IN_IDLE_STATE   0x0100   // in idle state
#define OUT_OF_RANGE    0x0080   // out of range
#define ERASE_PAR       0x0040   // erase parameter
#define WP_VIOLATION    0x0020   // WP violation
#define CARD_ECC_FAIL   0x0010   // card ECC failed
#define CC_ERR          0x0008   // CC error
#define GEN_ERR         0x0004   // (generic) error
#define WP_ERASE_SKIP   0x0002   // WP erase skip

//#define CSD_BUF_SIZE 16
//bank1 unsigned char MMC_cardCsdBuf[CSD_BUF_SIZE];
bank1 unsigned int blk0_checksum;

//////////////////////////////////////////////////////////////////////////////
static unsigned char MMC_ExecCmd (unsigned char cmd, unsigned long par) @ bank1
{
    unsigned char stat, ret, crc, loop;
    unsigned int  cnt;

	SD_DISABLE;
	SD_SCK_UNHOLD;
	loop = 0;
	do
	{
		ret = 0x00;
		crc = 0xff;
//		MMC_lastCmd = cmd;

		if ( cmd == GO_IDLE_STATE )	//	CMD0
		{
			SD_DISABLE;			//	SPI_CS_HI;	// SS = hi

			for (cnt = 30; cnt--;)
				SPI_burst ();

			ret = 0x01;
			crc = 0x95;
		}

		SD_ENABLE;				// SPI_CS_LO;	// SS = lo
		SPI_write (cmd | 0x40);
		SPI_write (par >> 24);
		SPI_write (par >> 16);
		SPI_write (par >> 8);
		SPI_write (par);
		SPI_write (crc);

		cnt = 1000;
		while ( SPI_read () != ret && --cnt != 0 ) {}


		if ( cnt == 0 )
		{
			stat = MMC_ERR;
		}
		else
		{
			stat = MMC_OK;

			if ( cmd == READ_SINGLE_BLOCK )
			{
				cnt = 1000;
				while ( SPI_read () != 0xfe && --cnt != 0 ) {}

				if ( cnt == 0 )
					stat = MMC_ERR;
				else
					return MMC_OK;
			}
		}

		SD_DISABLE;	// SPI_CS_HI;
		SPI_burst ();

	} while ( stat != MMC_OK && ++loop < 5 );

	return stat;
}


//////////////////////////////////////////////////////////////////////////////
MMC_STAT MMC_Init (void)
{
    if ( MMC_ExecCmd (GO_IDLE_STATE, 0) != MMC_OK || // - CMD0
         MMC_ExecCmd (SEND_OP_COND,  0) != MMC_OK )  // - CMD1
	{
        return MMC_NO_RESP;
	}

	return MMC_ExecCmd (SET_BLOCKLEN, 512);
}
#if 0
//////////////////////////////////////////////////////////////////////////////
MMC_STAT MMC_ChkStatus (void)
{
  	return MMC_ExecCmd (SEND_STATUS, 0);
}
#endif
//////////////////////////////////////////////////////////////////////////////
MMC_STAT MMC_ReadSegment (unsigned short sect, unsigned char seg)
{
	if ( seg == 0 )
	{
		if ( MMC_ExecCmd (READ_SINGLE_BLOCK, MMC_SECTOR_SIZE*sect) != MMC_OK )
			return MMC_ERR;
	}

    unsigned char cnt = SEGMENT_SIZE;    // buffer size = 32 bytes (per segment)
    char bank2   *buf = MMC_Buf.buffer;
	do
	{   *buf = SPI_read ();
		buf++;
	} while ( --cnt );

	// if it's the last segment in a sector ...
	if ( seg >= (MMC_SECTOR_SIZE/SEGMENT_SIZE - 1) )
	{
		SPI_burst ();	// CRC higher byte
		SPI_burst ();	// CRC lower byte

		SD_DISABLE;
		SPI_burst ();
	}

	return MMC_OK;
}

//////////////////////////////////////////////////////////////////////////////
MMC_STAT MMC_ReadSegOffset (unsigned short sect, unsigned int offset)
{
    if ( MMC_ExecCmd (READ_SINGLE_BLOCK, MMC_SECTOR_SIZE*sect) != MMC_OK )
        return MMC_ERR;


    unsigned int  sect_size = MMC_SECTOR_SIZE;  // 512
    unsigned char cnt = SEGMENT_SIZE;           // 32
    unsigned char bank2 *buf = MMC_Buf.buffer;

	if ( sect == 0 )
       	blk0_checksum = 0;

    do
    {
		unsigned char c = SPI_read ();

        if ( offset || cnt == 0 )
        {
            offset--;
        }
        else
        {
            *buf = c;
            buf++;
			cnt--;
        }

        if ( sect == 0 )
        	blk0_checksum += c;

    } while ( --sect_size );

    SPI_burst ();	// read sector CRC16
	SPI_burst ();

	SD_DISABLE;
	SPI_burst ();
    return MMC_OK;
}

//////////////////////////////////////////////////////////////////////////////
MMC_STAT MMC_NextSector (unsigned short sector)
{
    if ( MMC_ExecCmd (READ_SINGLE_BLOCK, MMC_SECTOR_SIZE*sector) == MMC_OK )
    {
        return MMC_OK;
	}

	SD_DISABLE;
	SPI_burst ();
    return MMC_ERR;
}

///////////////////////////////////////////////////////////////////////////////
void MMC_dumpSegment (unsigned char seg)
{
	VS1011_ENABLE_DATA;

	FSR = SEGMENT_SIZE;
	do { SPI_burst (); } while ( --FSR );

	VS1011_DISABLE_DATA;

	// if it's the last segment in a sector ...
    if ( seg >= (MMC_SECTOR_SIZE/SEGMENT_SIZE-1) )
    {
	    SPI_burst ();	// read sector CRC16
		SPI_burst ();

        SD_DISABLE;
		SPI_burst ();
    }
}

///////////////////////////////////////////////////////////////////////////////
void MMC_flushSector (unsigned char seg)
{
	if ( seg >= (MMC_SECTOR_SIZE/SEGMENT_SIZE) )
		return;

	do
	{
	    FSR = SEGMENT_SIZE;
	    do { SPI_burst (); } while ( --FSR );

	} while ( ++seg < (MMC_SECTOR_SIZE/SEGMENT_SIZE) );

    SPI_burst ();	// read sector CRC16
	SPI_burst ();

    SD_DISABLE;
	SPI_burst ();
}

///////////////////////////////////////////////////////////////////////////////
MMC_STAT MMC_SkipSegment (unsigned short sector, unsigned char seg)
{
	if ( seg < (MMC_SECTOR_SIZE/SEGMENT_SIZE) )
	{
	    if ( MMC_ExecCmd (READ_SINGLE_BLOCK, MMC_SECTOR_SIZE*sector) != MMC_OK )
	        return MMC_ERR;

		seg <<= 4;
	    do
	    {
	        SPI_read ();
	        SPI_read ();
	    } while ( --seg );
	}

    return MMC_OK;
}
