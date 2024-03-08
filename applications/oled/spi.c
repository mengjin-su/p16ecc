#include <p16f688.h>
#include "define.h"
#include "spi.h"

/////////////////////////////////////////////////////////////
/*
    intitialize PIC16F688 virtual SPI interface
*/
void SPI_init ()
{
	CFG_SPI_SCK;	SPI_SCK_LO;
	CFG_SPI_MOSI;

	SD_CS_CFG_DDR;
	SD_DISABLE;		// SD card select pin = output = HI
	SD_SCK_UNHOLD;

	VS1011_ICS_CFG_DDR;
	VS1011_DISABLE_INST;

	VS1011_DCS_CFG_DDR;
	VS1011_DISABLE_DATA;
}

/////////////////////////////////////////////////////////////
void SPI_write (unsigned char data)
{
	SPI_MOSI_LO;
	_MOVF_W (PORTC);

	_BTFSC (data, 7);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (data, 6);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (data, 5);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (data, 4);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (data, 3);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (data, 2);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (data, 1);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (data, 0);	SPI_MOSI_HI;	SPI_SCK_HI;    _MOVWF (PORTC);
}

/////////////////////////////////////////////////////////////
char SPI_read (void)
{
	SPI_MOSI_HI;
	FSR = 0;
	_MOVF_W (PORTC);

	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 7);	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 6);	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 5);	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 4);	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 3);	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 2);	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 1);	SPI_SCK_HI;    _MOVWF (PORTC);
	_BTFSC (PORTC, SPI_MISO_PIN); sbi(FSR, 0);	SPI_SCK_HI;    _MOVWF (PORTC);

	return FSR;
}

/////////////////////////////////////////////////////////////
void SPI_burst (void)
{
	SPI_MOSI_HI;

	SPI_SCK_HI;		SPI_SCK_LO;
	SPI_SCK_HI;		SPI_SCK_LO;
	SPI_SCK_HI;		SPI_SCK_LO;
	SPI_SCK_HI;		SPI_SCK_LO;
	SPI_SCK_HI;		SPI_SCK_LO;
	SPI_SCK_HI;		SPI_SCK_LO;
	SPI_SCK_HI;		SPI_SCK_LO;
	SPI_SCK_HI;		SPI_SCK_LO;
}

/////////////////////////////////////////////////////////////
void SPI2_write (unsigned char data)
{
	SPI2_MOSI_LO;
    FSR = 8;
	_MOVF_W (PORTC);
    do
    {
		_BTFSC (data, 7);
      	SPI2_MOSI_HI;

        SPI_SCK_HI;
        _RLF_F (data);
        _MOVWF (PORTC);
    } while ( --FSR );
}

